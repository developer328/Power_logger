/*
 * 24-FEB-2025
 * Power Logger
 * V, mA, mWh, h
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
unsigned long Time_offset = 0;

bool save_pwr = false;

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float power_mW = 0;
float load_v = 0;
float Time_min = 0;
float full_t = 0;
float power_mwh = 0;    

char v_tmp[12];
char ma_tmp[13];
char mwh_tmp[14];
char time_m_tmp[14]; 

void setup() { 

  pinMode(isr_pin, INPUT); 

  detachInterrupt(digitalPinToInterrupt(isr_pin));

  if(!u8x8.begin()){
    pinMode(LED_BUILTIN, OUTPUT);
    while(!digitalRead(isr_pin)){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }    
    pinMode(LED_BUILTIN, INPUT);
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
    cli();
//    pinMode(4, OUTPUT);     
    while(!digitalRead(isr_pin)){
      ;
//      digitalWrite(4, HIGH);
//      delay(200);
//      digitalWrite(4, LOW);
//      delay(200);
    }  
    sei(); 
    pinMode(4, INPUT);
    u8x8.clear();

    u8x8.setFont(u8x8_font_8x13_1x2_r);
    u8x8.drawString(1,10,"Wait..");
  }
  
  if(!ina219.begin()){
    u8x8.clear();
//    pinMode(4, OUTPUT); 
    u8x8.setFont(u8x8_font_8x13_1x2_r);
    u8x8.drawString(1,2,"INA219 ERROR!");
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    u8x8.drawString(1,4,"Please reset!");
    cli();
    while(1){
      ;
//      digitalWrite(4, HIGH);
//      delay(100);
//      digitalWrite(4, LOW);
//      delay(100);   
    }
  }

  char tmp_buf[] = "########### New ###########";

  sd_print(tmp_buf, tmp_buf, tmp_buf);

  delay(500);

  u8x8.clear();
  u8x8.setFont(u8x8_font_8x13_1x2_r);
  u8x8.drawString(1,10,"Ready?");
  cli();
  while(!digitalRead(isr_pin)){
    ;
  }
  sei();

  delay(70);
  attachInterrupt(digitalPinToInterrupt(isr_pin), isr1, RISING); 
  delay(70); 
  save_pwr = false;

  u8x8.clear();
  Time_offset = millis();
}

void loop() {

  if(((millis()-Time_offset)-Time) >= INTERVAL){
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    load_v = busvoltage + (shuntvoltage / 1000.0);
    Time_min = ((float)(millis()-Time)/1000.0)/60.0;
    power_mwh += load_v * current_mA / 3600;;    
    full_t += Time_min;

    dtostrf(load_v, 6, 3, v_tmp);    
    dtostrf(current_mA, 6, 2, ma_tmp);        
    dtostrf(power_mwh, 6, 2, mwh_tmp);       
    dtostrf(full_t, 6, 2, time_m_tmp);

    sd_print(v_tmp, ma_tmp, time_m_tmp);

    sprintf(v_tmp, "%s V", v_tmp);//11 char   
    sprintf(ma_tmp, "%s mA", ma_tmp);//11 char  
    sprintf(mwh_tmp, "%s mWh", mwh_tmp);//12 char    
    sprintf(time_m_tmp, "%s min", time_m_tmp);//12 char

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
  Time_offset = 0;
}

void isr1(void){
  save_pwr = !save_pwr; 
  delayMicroseconds(500); 
}

void sd_print(char* v, char* ma, char* mins){
    File voltage = SD.open("voltage.txt", FILE_WRITE);
    if(voltage){
      voltage.println(v);
      voltage.close();
    }
    
    File current = SD.open("current.txt", FILE_WRITE);    
    if(current){
      current.println(ma);
      current.close();
    }
    
    File minutes = SD.open("minutes.txt", FILE_WRITE);
    if(minutes){
      minutes.println(mins);
      minutes.close();
    }  
}
