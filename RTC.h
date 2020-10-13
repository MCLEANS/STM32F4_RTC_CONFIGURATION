#ifndef _RTC_H
#define _RTC_H

#include "stm32f4xx.h"

namespace custom_libraries{

//struct to hold the RTC time values
struct RTC_values{
  uint16_t year;
  uint8_t month;
  uint8_t date;
  uint8_t day_of_week;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

class RTC_{
    private:
    private:
      uint8_t obtain_unit(uint16_t value);
      uint8_t obtain_tens(uint16_t value);
      uint8_t BCD_to_decimal(uint8_t val);
      uint8_t decimal_to_BCD(uint8_t val);
    public:
    public:
      RTC_();
      void set_time(RTC_values &rtc_values);
      RTC_values get_time(void);    
      ~RTC_();

};

}

#endif //_RTC_H