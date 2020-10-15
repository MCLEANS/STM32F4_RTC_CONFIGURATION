#include "stm32f4xx.h"
#include "clockconfig.h"
#include "NOKIA_5110.h"
#include "RTC.h"
#include <stdlib.h>
#include <string>
#include <string.h>

#define RST_PORT GPIOD
#define RST_PIN 0
#define CS_PORT GPIOD
#define CS_PIN 1
#define DC_PORT GPIOD
#define DC_PIN 2

custom_libraries::clock_config system_clock;
custom_libraries::NOKIA_5110 NOKIA(SPI1,
                                    GPIOA,
                                    5,
                                     7,
                                    6,
                                    64,
                                    false,
                                    false,
                                    false,
                                    CS_PORT,
                                    CS_PIN,
                                    RST_PORT,
                                    RST_PIN,
                                    DC_PORT,
                                    DC_PIN);
custom_libraries::RTC_ rtc;
custom_libraries::RTC_values rtc_values;

char year[5];
char month[3];
char date[3];
std::string day_of_week[] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
char day[10];
char hour[3];
char minute[3];
char second[3];
char date_seperator[] = "/";
char time_seperator[] = ":";

int main(void) {
  system_clock.initialize();
  NOKIA.inverted_mode();

  /*
  rtc_values.second = 0;
  rtc_values.minute = 33;
  rtc_values.hour = 10;
  rtc_values.day_of_week = 4;
  rtc_values.date = 15;
  rtc_values.month = 10;
  rtc_values.year = 20;

  rtc.set_time(rtc_values);
  */

  while(1){
    rtc_values = rtc.get_time();
    itoa((rtc_values.year),year,10);
    itoa(rtc_values.month,month,10);
    itoa(rtc_values.date,date,10);
    itoa(rtc_values.hour,hour,10);
    itoa(rtc_values.minute,minute,10);
    itoa(rtc_values.second,second,10);
    strcpy(day,(day_of_week[rtc_values.day_of_week-1]).c_str());

    //Date
    NOKIA.print(day,15,0);
    NOKIA.print(date,5,2);
    NOKIA.print(date_seperator,20,2);
    NOKIA.print(month,30,2);
    NOKIA.print(date_seperator,45,2);
    NOKIA.print(year,55,2);
    //Time
    NOKIA.print(hour,10,4);
    NOKIA.print(time_seperator,25,4);
    NOKIA.print(minute,35,4);
    NOKIA.print(time_seperator,50,4);
    NOKIA.print(second,60,4);
    //Clear the LCD every minute
    if(rtc_values.second  == 59){
      NOKIA.clear();
    }

  }
}
 