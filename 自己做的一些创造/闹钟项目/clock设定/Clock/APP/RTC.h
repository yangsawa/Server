#ifndef __RTC_H
#define __RTC_H

extern uint16_t MyRTC_Time[];

void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);
void MyRTC_Adjust(uint16_t year, uint16_t mon, uint16_t day,
                  uint16_t hour, uint16_t min, uint16_t sec);

#endif
