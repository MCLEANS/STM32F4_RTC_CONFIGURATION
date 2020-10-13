#include "RTC.h"

namespace custom_libraries{

RTC_::RTC_(){
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

uint8_t RTC_::obtain_unit(uint16_t value){
    return(value%10);
}

uint8_t RTC_::obtain_tens(uint16_t value){
  return((value/10)%10);  
}

uint8_t RTC_::BCD_to_decimal(uint8_t val){
	return((val /16 * 10) + (val %16));
}

uint8_t RTC_::decimal_to_BCD(uint8_t val){
	return((val /10 * 16) + (val %10));
}

void RTC_::set_time(RTC_values &rtc_values){
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

RTC_values RTC_::get_time(void){
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


}