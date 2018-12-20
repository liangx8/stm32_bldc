/*
 * 测量ZC时要注意测量的延时，根据经验。ZC的时间取点，必须是电压第一次交叉的时间为准。否则就会
 * 导致偏差的积累而使的电机卡转
 * 设计一个起步阶段。
 * 起步阶段ZC没有被扫描到，就会变化相的长度。
 */


#include <stdint.h>
#include <stm32f0xx.h>
#include "pins.h"


struct ZC_DATA{
  volatile uint32_t bflag;                // 运行标记 0
  uint8_t  state;                         // 当前相 4
  uint8_t  startup_cnt;                   // 5 这个值在 instant_start中被初始化
  uint8_t  _reversed[2];                  // 6
  uint32_t win_size;
  uint32_t timestamp;
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


extern struct ZC_DATA rd;

/*
void zc_exam(void)
{
  struct ZC_DATA *zc=&rd;
  uint32_t cp_out;
  if((zc->flag & ZC_SKIP_PWM_ON_MSK ) && (TIM1->CNT < TIM1->CCR1))
    return;
  cp_out=COMP->CSR << (31-COMP_CSR_COMP1OUT_Pos);
  if((cp_out ^ zc->flag) & ZC_DIR_MSK){
    if(zc->window < zc->win_size)zc.window ++;
    B5ON;
  } else {
    if(zc->window)zc.window --;
    B5OFF;
  }
}
*/
//#define ZC_FAIL_POS 0
//#define ZC_FAIL_MSK (1<<ZC_FAIL_POS)
/* 非阻塞扫描ZC,如果找到ZC，
 * 1 保存当前时间到timestamp,
 * 2 计算出下一相并返回其值
 * 最新进展：把ZC的偏移补偿放到和comm平均的计算中，电机转动的效果更加好。但是观察示波器的信息，
 * 还是会出现ZC不出现的现象。应该从换相的计算入手去尝试改进
 */
uint32_t zc_scan(void)
{
  uint32_t timestamp;
  uint32_t window=0;
  uint32_t checkpoint=0;
  uint32_t comm=SysTick->LOAD;
  while(1){
    uint32_t cp_out;
    timestamp=SysTick->VAL;
    if(timestamp<1025)break;
    if((rd.bflag & ZC_SKIP_PWM_ON_MSK ) && (TIM1->CNT < TIM1->CCR1))continue;
    cp_out = COMP->CSR >> COMP_CSR_COMP1OUT_Pos;
    if( (cp_out ^ rd.state) & 1){
      if(checkpoint==0)checkpoint=timestamp;
      window ++;
      if(window > rd.win_size){
	// calculate next comm;
	uint32_t tmp;
	uint32_t center=comm >> 1;
	int shift=center - checkpoint;
	shift= shift /4;
	if(rd.bflag & STARTUP_STAGE_MSK){
	  if(rd.startup_cnt > 10){
	    rd.bflag &= ~STARTUP_STAGE_MSK;
	  } else {
	    rd.startup_cnt ++;
	  }
	}
	tmp=comm+rd.timestamp-checkpoint;
	tmp += shift;
	rd.timestamp=checkpoint;
	tmp=(tmp+comm)>>1;
	tmp=(tmp+comm)>>1;
	tmp=(tmp+comm)>>1;
	B9TAGGLE;
	return tmp;
      }
    } else {
      checkpoint=0;
      if(window)window--;
    }
  }
  // 扫描ZC失败，
  // 下一相时间根据是否是起步阶段来决定如何计算
  rd.startup_cnt = 0;
  rd.timestamp=0;
  return comm + (comm >> 4);
}
/*
 * 如果nosie过滤不了。返回下一相的长度,否则0
 * 换相以后，如果马上检测到反馈的电平已经交叉，即认为是干扰。在半相一内如果
 * 没有等到没有交叉的信号,即认为是错误。本次不捕捉反馈。直接在下一相缩短换相时间
 */
uint32_t zc_noise(void)
{
  uint32_t comm=SysTick->LOAD;
  uint32_t half_comm=comm >> 1;
  uint32_t window=0;
  // 不要斋等,积极检查开始的干扰是否存在，如果没有干扰就可以马上退出
  while(SysTick->VAL > half_comm){
    if((rd.bflag & ZC_SKIP_PWM_ON_MSK ) && (TIM1->CNT < TIM1->CCR1))continue;
    uint32_t cp_out = COMP->CSR >> COMP_CSR_COMP1OUT_Pos;
    if((cp_out ^ rd.state) & 1){
      if(window) window --;
    } else {
      window ++;
    }
    if (window > (rd.win_size >> 1)){
      // 干扰开始消失或者没有发生，退出。让下一个步骤开始(zc_scan() 开始运行)
      return 0;
    }
  }
  B5TAGGLE;
  //要根据是否是起步阶段来决定
  if(rd.bflag & STARTUP_STAGE_MSK){
    rd.startup_cnt = 0;
    // 干扰到了半相，还没有消失，后面无法扫描ZC,直接把下一相缩短，设ZC为相开始
    rd.timestamp=comm;
  } else {
    rd.timestamp = comm - (comm >> 2);
  }
  return comm - (comm >> 4);
}
/*
 * initialized zc
 */
void zc_init(void)
{
  uint32_t comm=SysTick->LOAD;
  uint32_t tmp=comm >> 11;
  if(tmp <= 10){
    rd.win_size=10;
    rd.bflag &= ~ZC_SKIP_PWM_ON_MSK;
  }else{
    rd.win_size=tmp;
    rd.bflag |= ZC_SKIP_PWM_ON_MSK;
  }
}
