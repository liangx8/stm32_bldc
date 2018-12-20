#include <stdint.h>
#include <stm32f0xx.h>
#include "config.h"
#include "timer.h"
#include "comp.h"
#include "motor.h"
#include "pins.h"
#include "zerocross.h"


// SysTick 在 48Mhz, 这个值相当于 10666us
// 48Mhz 相当于 48cycle每微秒(us)
// 0xbb00/48~=16000us
#define STARTUP_COMM 0xbb000
#define MIN_COMM 0xbc00

struct RUN_DATA{
  volatile uint32_t bflag;                // 运行标记 0
  uint8_t  state;                         // 当前相 4
  uint8_t  _reversed[3];                  // 5
  uint32_t _reversed1[2];
};
// 定义bflag
// 当前转的方向
#define RUN_DIR_POS            0
#define RUN_DIR_MSK            (1 << RUN_DIR_POS)
// 上次转的方向
#define LAST_RUN_DIR_POS       1
#define LAST_RUN_DIR_MSK      (1 << LAST_RUN_DIR_POS)
//一次相位完成，
#define COMM_DONE_POS          2
#define COMM_DONE_MSK          (1 << COMM_DONE_POS)
// 当速度比较快的时候就不用过滤PWM打开
#define ZC_SKIP_PWM_ON_POS     3
#define ZC_SKIP_PWM_ON_MSK    (1 << ZC_SKIP_PWM_ON_POS)
// 电机启动阶段
#define STARTUP_STAGE_POS      4
#define STARTUP_STAGE_MSK      (1 << STARTUP_STAGE_POS)
// 出错了。
#define HW_ERROR_POS           31
#define HW_ERROR_MSK           (1 << HW_ERROR_POS)

struct RUN_DATA rd;

void idle_start(struct MOTOR_CONTROL *);
/*
void init_data(void){
  rd.state=0;
  rd.bflag &= LAST_RUN_DIR_MSK;

}
*/
void instant_start(uint32_t); // implements in motor_asm.S
void instant_stop(void); // implements in motor_asm.S
//void zc_sync(void){
//}
// 原来的顺序是FC,FB,FA,FC,FB,FA,但是只是设置而没有真正切换相位，因此，比较器必须提前一轮

void parse_rcp(struct MOTOR_CONTROL *mctrl)
{
  uint32_t rcp=get_rcp_value();

  if(rcp==0) {
	mctrl->cmd_code=MOTOR_KEPT;
	return;
  }

  if(rcp < 2049) {
	mctrl->cmd_code=MOTOR_CMD_STOP;
	return;
  }
  // convert rcp to pwm
  if(rcp < 2113){
	mctrl->pwm_duty=5;
  } else if(rcp > 3904) {
	mctrl->pwm_duty=254;
  } else {
	mctrl->pwm_duty=(rcp - 2000) >> 3;
  }
  if(SWDEBUG){
    if(mctrl->pwm_duty > 50)mctrl->pwm_duty=50; // 开发阶段，最大限制在 50/250 功率
  }
  mctrl->cmd_code=MOTOR_CMD_START_LEFT;
  return;
}

const uint32_t feedback[6]={FA,FC,FB,FA,FC,FB};
void motor_run(struct MOTOR_CONTROL *mctrl)
{
  parse_rcp(mctrl);
  uint32_t next_comm;
  if(mctrl->cmd_code== MOTOR_CMD_STOP) {
    instant_stop();
    TIM1->EGR=TIM_EGR_COMG;
    // 做一写停止马达后的工作，例如：恢复串口通信
    mctrl->action=idle_start;
    //rd.bflag &= ~HW_ERROR_MSK;
    return;
  }
  if(rd.bflag & HW_ERROR_MSK){
    SysTick->CTRL=0;
    set_step(7);
    TIM1->EGR=TIM_EGR_COMG;
    return;
  }

  if(SysTick->LOAD <= 48 * 128){
    rd.bflag |= HW_ERROR_MSK;
    return;
  }
  if(mctrl->cmd_code != MOTOR_KEPT){
    uint16_t duty=mctrl->pwm_duty;
    if((rd.bflag & STARTUP_STAGE_MSK) && duty > 50){
      duty=50;
    }
    timer1_commit_pwm(duty);
  }
  zc_init();
  if(TIM1->CCR1 > 128){
    rd.bflag &= ~ZC_SKIP_PWM_ON_MSK;
  }

  next_comm=zc_noise();
  if(next_comm){
    SysTick->LOAD=next_comm;
  } else {
    next_comm=zc_scan();
    SysTick->LOAD = next_comm;
  }

  while((rd.bflag & COMM_DONE_MSK) ==0);
  // commutation end
  rd.bflag &= ~COMM_DONE_MSK;
  // prepare next state

  if(rd.state <5)
	rd.state ++;
  else
	rd.state=0;

  // 理论上，这动作应该放在中断中,在中断中修改，就必须把feedback中的位置移动一个位置
  comp1_change_input(feedback[rd.state]);
  set_step(rd.state);
}



void idle(struct MOTOR_CONTROL *mctrl)
{
  parse_rcp(mctrl);


  switch(mctrl->cmd_code){
  case MOTOR_KEPT:
  case MOTOR_CMD_STOP:return;
  case MOTOR_CMD_START_LEFT:
	rd.bflag |= RUN_DIR_MSK;
	break;
  case MOTOR_CMD_START_RIGHT:
	rd.bflag &= ~RUN_DIR_MSK;
	break;
  }

  // 马达开始转
  LED_L;
  BEEP_OFF;
  instant_start(STARTUP_COMM);
  mctrl->action=motor_run;
  TIM1->EGR=TIM_EGR_COMG;

  //  rd.state ++;
  //  set_step(rd.state);
}
void idle_start(struct MOTOR_CONTROL *mctrl)
{
  comp1_change_input(FC);
  mctrl->action=idle;
  LED_H;
  BEEP_ON;
}
/*
void motor(struct MOTOR_CONTROL *mctrl)
{
  init_data();
  while(1){
	rd.action(mctrl);
  }
}
*/
void SysTick_handler(void)
{
  uint32_t flag=rd.bflag | COMM_DONE_MSK;
  rd.bflag = flag;
  TIM1->EGR=TIM_EGR_COMG;
  if(flag & STARTUP_STAGE_MSK){
    return;
  }
  if(rd.state & 1){
    B14ON;
  }else{
    B14OFF;
  }
}
/*
void calc_next_comm(void){
  uint32_t zc_length=rd.real_comm-SysTick->VAL;
  uint32_t comm=(zc_length * 2 + rd.real_comm) >> 1;
  comm = (comm + rd.real_comm) >> 1;
  rd.real_comm=rd.next_comm;
  SysTick->LOAD=rd.real_comm;
  rd.next_comm=comm;
}

*/
