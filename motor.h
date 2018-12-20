#ifndef MOTOR_0098877_H
#define MOTOR_0098877_H
#define MOTOR_CMD_STOP         0
#define MOTOR_CMD_START_LEFT   1
#define MOTOR_CMD_START_RIGHT  2
#define MOTOR_CMD_BRAKE        3
// 维持当前状态
#define MOTOR_KEPT             -1
struct MOTOR_CONTROL{
  void (*action)(struct MOTOR_CONTROL *);
  int cmd_code;
  uint16_t pwm_duty;
};
//void motor(struct MOTOR_CONTROL *);
#ifndef PA4_SELECT
#error "include comp.h before motor.h"
#endif
#define FA PA4_SELECT
#define FB PA5_SELECT
#define FC PA0_SELECT

#endif
