/*
 * 24-FEB-2025
 * Power Logger
 * V, mA, mW, mWh, mAh, h
 */

#include <SPI.h>
#include<SD.h>
#include <Wire.h>
#include <U8x8lib.h>
#include <Adafruit_INA219.h>

#define oled_i2c_addr 0x3c
#define ina219_i2c_addr 0x40
#define sd_cs 10
#define INTERVAL 100
#define isr_pin 2

Adafruit_INA219 ina219;

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

unsigned long Time = 0;

bool save_pwr = false;

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float power_mW = 0;
float load_v = 0;
float Time_min = 0;
float power_mwh = 0;    

char v_tmp[12];
char ma_tmp[13];
char mwh_tmp[14];
char time_m_tmp[14]; 

void setup() { 

  pinMode(isr_pin, INPUT);

  detachInterrupt(digitalPinToInterrupt(isr_pin));

  if(!u8x8.begin()){
    pinMode(13, OUTPUT);
    while(!digitalRead(isr_pin)){
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }    
    pinMode(13, DEFAULT);
  }else{
    u8x8.setPowerSave(0);
    u8x8.clear();
    u8x8.setFont(u8x8_font_8x13_1x2_r);
    u8x8.drawString(2,10,"Data Logger");
  }
  
  if(!SD.begin(sd_cs)){
    u8x8.clear();
    u8x8.setFont(u8x8_font_8x13_1x2_r);
    u8x8.drawString(3,0,"SD FAILED!");   
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    u8x8.drawString(0,3,"Press the button");
    u8x8.drawString(2,4,"to skip this");
    u8x8.drawString(5,5,"step.");
    pinMode(13, OUTPUT);
    while(!digitalRead(isr_pin)){
      digitalWrite(13, HIGH);
      delay(200);
      digitalWrite(13, LOW);
      delay(200);
    }   
    pinMode(13, DEFAULT);
    u8x8.clear();

    u8x8.setFont(u8x8_font_8x13_1x2_r);
    u8x8.drawString(1,10,"Wait..");
  }
  
  if(!ina219.begin()){
    u8x8.clear();
    pinMode(13, OUTPUT); 
    u8x8.setFont(u8x8_font_8x13_1x2_r);
    u8x8.drawString(1,2,"INA219 ERROR!");
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    u8x8.drawString(1,4,"Please reset!");
    while(1){
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);   
    }
  }

  delay(500);
  
  attachInterrupt(digitalPinToInterrupt(isr_pin), isr1, RISING); 
  delay(70); 
  save_pwr = false;

  u8x8.clear();
}

void loop() {

  if((millis()-Time) >= INTERVAL){
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    load_v = busvoltage + (shuntvoltage / 1000);
    Time_min = ((float)Time/1000)/60;
    power_mwh = power_mW*(Time_min/60);    

    dtostrf(load_v, 6, 3, v_tmp);    
    dtostrf(current_mA, 6, 2, ma_tmp);        
    dtostrf(power_mwh, 6, 2, mwh_tmp);       
    dtostrf(Time_min, 6, 2, time_m_tmp);

    File voltage = SD.open("voltage.txt", FILE_WRITE);
    if(voltage){
      voltage.println(v_tmp);
      voltage.close();
    }
    
    File current = SD.open("current.txt", FILE_WRITE);    
    if(current){
      current.println(ma_tmp);
      current.close();
    }
    
    File minutes = SD.open("minutes.txt", FILE_WRITE);
    if(minutes){
      minutes.println(time_m_tmp);
      minutes.close();
    }  

    sprintf(v_tmp, "%s V", v_tmp);//11    
    sprintf(ma_tmp, "%s mA", ma_tmp);//11        
    sprintf(mwh_tmp, "%s mWh", mwh_tmp);//12        
    sprintf(time_m_tmp, "%s min", time_m_tmp);//12

    if(!save_pwr){
      u8x8.setPowerSave(0);
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      u8x8.drawString(1,0,v_tmp); 
      u8x8.drawString(1,2,ma_tmp);           
      u8x8.drawString(1,4,mwh_tmp);     
      u8x8.drawString(1,6,time_m_tmp);
    }else{
      u8x8.clearDisplay();  
      u8x8.setPowerSave(1);         
    }
    Time = millis();
  }
}

void isr1(void){
  save_pwr = !save_pwr; 
  delayMicroseconds(500); 
}
