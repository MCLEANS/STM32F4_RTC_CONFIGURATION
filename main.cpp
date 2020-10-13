#include "stm32f4xx.h"
#include "clockconfig.h"
#include "NOKIA_5110.h"
#include "RTC.h"
#include <stdlib.h>

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
uint8_t x_value = 70;
uint8_t y_value = 1;

uint8_t line_counter = 1;
uint8_t page = 1;

bool clear_screen = false;

struct RTC_values{
  uint16_t year;
  uint8_t month;
  uint8_t date;
  uint8_t day_of_week;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};


uint8_t obtain_unit(uint16_t value){
  return(value%10);
}

uint8_t obtain_tens(uint16_t value){
  return((value/10)%10);  
}

uint8_t BCD_to_decimal(uint8_t val){
	return((val /16 * 10) + (val %16));
}

uint8_t decimal_to_BCD(uint8_t val){
	return((val /10 * 16) + (val %10));
}


void init(void){
  //Configuration of the RTC
  //Enable power interface clock
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  //Enable access to backup domain registers
  PWR->CR |= PWR_CR_DBP;
  //Reset the backup domain
  RCC->BDCR |= RCC_BDCR_BDRST;
  RCC->BDCR &= ~RCC_BDCR_BDRST;
  //Enable the LSI clock
  RCC->CSR |= RCC_CSR_LSION;
  //Wait for LSI to be ready
  while(!(RCC->CSR & RCC_CSR_LSIRDY)){}
  //Set RTC clock source to LSI
  RCC->BDCR |= RCC_BDCR_RTCSEL_1;
  //Enable the RTC
  RCC->BDCR |= RCC_BDCR_RTCEN;
}

void set_time(RTC_values &rtc_values){
  //Disable RTC registers write protection
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
  //Enter initialization mode
  RTC->ISR |= RTC_ISR_INIT;
  //Wait for initialization
  while(!(RTC->ISR & RTC_ISR_INITF)){}
  //set the prescaler
  RTC->PRER |= 250 << 0;
  RTC->PRER |= 128 << 16;
  /**********************************************************************
   * NB - The peripheral registers have to be accessed by words (32 bits).
   * *******************************************************************/
  //Load date and time values in the shadow registers
  uint32_t date_value = 0;
  //Set year
  date_value |= decimal_to_BCD(obtain_unit(rtc_values.year)) << 16;
  date_value |= decimal_to_BCD(obtain_tens(rtc_values.year)) << 20;
  //Set Month
  date_value |= decimal_to_BCD(obtain_unit(rtc_values.month)) << 8;
  date_value |= decimal_to_BCD(obtain_tens(rtc_values.month)) << 12;
  //Set date
  date_value |= decimal_to_BCD(obtain_unit(rtc_values.date)) << 0;
  date_value |= decimal_to_BCD(obtain_tens(rtc_values.date)) << 4;
  //Set week day (1 = Monday , 7 = Sunday)
  date_value |= decimal_to_BCD(rtc_values.day_of_week) << 13;
  //Write to data register
  RTC->DR = date_value;
  //Set Notation to 24 hr clock
  uint32_t time_value  = 0;
  time_value &= ~RTC_TR_PM;
  //Set Hour
  time_value |= decimal_to_BCD(obtain_unit(rtc_values.hour)) << 16;
  time_value |= decimal_to_BCD(obtain_tens(rtc_values.hour)) << 20;
   //Set Minutes
  time_value |= decimal_to_BCD(obtain_unit(rtc_values.minute)) << 8;
  time_value |= decimal_to_BCD(obtain_tens(rtc_values.minute)) << 12;
  //Set seconds
  time_value |= decimal_to_BCD(obtain_unit(rtc_values.second)) << 0;
  time_value |= decimal_to_BCD(obtain_tens(rtc_values.second)) << 4;
  //write to time register
  RTC->TR = time_value;
  //Set 24hr or 12hr
  RTC->CR |= RTC_CR_FMT; //24 HR SYS
  //Exit initialization mode
  RTC->ISR &= ~RTC_ISR_INIT;
  //Enable RTC registers write protection
  RTC->WPR = 0xFF;

}

RTC_values get_time(void){
  RTC_values rtc_values;
  //Read seconds value
  uint8_t second_units = BCD_to_decimal(RTC->TR & RTC_TR_SU);
  uint8_t second_tens = BCD_to_decimal((RTC->TR & RTC_TR_ST) >> 4); 
  rtc_values.second = (second_tens*10)+second_units;
  //Read Minute value
  uint8_t minute_units = BCD_to_decimal((RTC->TR & RTC_TR_MNU) >> 8);
  uint8_t minute_tens = BCD_to_decimal((RTC->TR & RTC_TR_MNT) >> 12);
  rtc_values.minute = (minute_tens*10)+minute_units;
  //Read hour value
  uint8_t hour_units = BCD_to_decimal((RTC->TR & RTC_TR_HU) >> 16);
  uint8_t hour_tens = BCD_to_decimal((RTC->TR & RTC_TR_HT) >> 20);
  rtc_values.hour = (hour_tens*10)+hour_units;
  //Read Day of Week
  rtc_values.day_of_week = BCD_to_decimal((RTC->DR & RTC_DR_WDU) >> 13);
  //Read date
  uint8_t date_units = BCD_to_decimal((RTC->DR & RTC_DR_DU) >> 0);
  uint8_t date_tens = BCD_to_decimal((RTC->DR & RTC_DR_DT) >> 4);
  rtc_values.date = (date_tens*10)+date_units;
  //Read Month
  uint8_t month_units = BCD_to_decimal((RTC->DR & RTC_DR_MU) >> 12);
  uint8_t month_tens = BCD_to_decimal((RTC->DR & RTC_DR_MT) >> 12);
  rtc_values.month = (month_tens*10)+month_units;
  //Read year
  uint8_t year_units = BCD_to_decimal((RTC->DR & RTC_DR_YU) >> 16);
  uint8_t year_tens = BCD_to_decimal((RTC->DR & RTC_DR_YT) >> 20);
  rtc_values.year = 2000+((year_tens*10)+year_units);
  
  return rtc_values;
}

void first_menu_page(void){
  NOKIA.print("1. FILES",0,1);
  NOKIA.print("2. time_value",0,2);
  NOKIA.print("3. ARCHIVE",0,3);
  NOKIA.print("4. PRINT",0,4);
  NOKIA.print("5. DELETE",0,5);
}

void second_menu_page(void){
  NOKIA.print("6. FILES",0,1);
  NOKIA.print("7. time_value",0,2);
  NOKIA.print("8. ARCHIVE",0,3);
  NOKIA.print("9. PRINT",0,4);
  NOKIA.print("10. DELETE",0,5);  
}


int main(void) {
  
  system_clock.initialize();

  //Configuration of the RTC
  //Enable power interface clock
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  //Enable access to backup domain registers
  PWR->CR |= PWR_CR_DBP;
  //Reset the backup domain
  RCC->BDCR |= RCC_BDCR_BDRST;
  RCC->BDCR &= ~RCC_BDCR_BDRST;
  //Enable the LSI clock
  RCC->CSR |= RCC_CSR_LSION;
  //Wait for LSI to be ready
  while(!(RCC->CSR & RCC_CSR_LSIRDY)){}
  //Set RTC clock source to LSI
  RCC->BDCR |= RCC_BDCR_RTCSEL_1;
  //Enable the RTC
  RCC->BDCR |= RCC_BDCR_RTCEN;
  //Disable RTC registers write protection
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
  //Enter initialization mode
  RTC->ISR |= RTC_ISR_INIT;
  //Wait for initialization
  while(!(RTC->ISR & RTC_ISR_INITF)){}
  //set the prescaler
  RTC->PRER |= 250 << 0;
  RTC->PRER |= 128 << 16;

  /**********************************************************************
   * NB - The peripheral registers have to be accessed by words (32 bits).
   * *******************************************************************/
  //Load date and time values in the shadow registers
  uint32_t date_value = 0;
  //Set year
  date_value |= decimal_to_BCD(obtain_unit(2020)) << 16;
  date_value |= decimal_to_BCD(obtain_tens(2020)) << 20;
  //Set Month
  date_value |= decimal_to_BCD(obtain_unit(3)) << 8;
  date_value |= decimal_to_BCD(obtain_tens(3)) << 12;
  //Set date
  date_value |= decimal_to_BCD(obtain_unit(12)) << 0;
  date_value |= decimal_to_BCD(obtain_tens(12)) << 4;
  //Set week day (1 = Monday , 7 = Sunday)
  date_value |= decimal_to_BCD(5) << 13;
  //Write to data register
  RTC->DR = date_value;
  //Set Notation to 24 hr clock
  uint32_t time_value  = 0;
  time_value &= ~RTC_TR_PM;
  //Set Hour
  time_value |= decimal_to_BCD(obtain_unit(19)) << 16;
  time_value |= decimal_to_BCD(obtain_tens(19)) << 20;
   //Set Minutes
  time_value |= decimal_to_BCD(obtain_unit(54)) << 8;
  time_value |= decimal_to_BCD(obtain_tens(54)) << 12;
  //Set seconds
  time_value |= decimal_to_BCD(obtain_unit(10)) << 0;
  time_value |= decimal_to_BCD(obtain_tens(10)) << 4;
  //write to time register
  RTC->TR = time_value;

  //Set 24hr or 12hr
  RTC->CR |= RTC_CR_FMT; //24 HR SYS
  //Exit initialization mode
  RTC->ISR &= ~RTC_ISR_INIT;
  //Enable RTC registers write protection
  RTC->WPR = 0xFF;

      char sec[3];
      char min[5];
      char hr[3];
        

  while(1){

    uint8_t second_units = BCD_to_decimal(RTC->TR & RTC_TR_SU);
    uint8_t second_tens = BCD_to_decimal((RTC->TR & RTC_TR_ST) >> 4); 
    uint8_t seconds = (second_tens*10)+second_units;

    uint8_t minute_units = BCD_to_decimal((RTC->TR & RTC_TR_MNU) >> 8);
    uint8_t minute_tens = BCD_to_decimal((RTC->TR & RTC_TR_MNT) >> 12);
    uint8_t minutes = (minute_tens*10)+minute_units;


    uint8_t hour_units = BCD_to_decimal((RTC->TR & RTC_TR_HU)>>16);
    uint8_t hour_tens = BCD_to_decimal((RTC->TR & RTC_TR_HT)>>20);
    uint8_t hours = (hour_tens*10)+hour_units;

    itoa(seconds,sec,10);
    itoa(minutes,min,10);
    itoa(hours,hr,10);

   NOKIA.print(hr,10,3);
    NOKIA.print(":",30,3);
    NOKIA.print(min,40,3);
    NOKIA.print(":",60,3);
    NOKIA.print(sec,70,3);

    
      

    /*
    NOKIA.print("MENU",20,0);
    if(line_counter > 10) line_counter = 10;
    if(line_counter < 1) line_counter = 1;
    if(line_counter <= 5) page = 1;
    if(line_counter > 5 and line_counter <= 10) page = 2;
    if(page == 1) first_menu_page();
    if(page == 2) second_menu_page();
    NOKIA.print("<<",x_value,y_value);

    if(clear_screen){
      NOKIA.clear();
      clear_screen = false;
    }
  */
  }
}
