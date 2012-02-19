#include <DHT22.h>
#include <Encoder.h>
#include <NewSoftSerial.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"




// Data structure

struct config_t
{
    float deshum;
    float destemp;
    
} configuration;


int LCDPin = 6;

NewSoftSerial LCD(0,LCDPin); //LCD Serial is attached to pin 10

// Data wire is plugged into port 7 on the Arduino
// Connect a 4.7K resistor between VCC and the data pin (strong pullup)
#define DHT22_PIN 4

// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);
int relay1 = 8;
int relay2 = 9;

// setup our temp variables
float tempc, tempf, relh;
unsigned long tempmili = 0;
unsigned long tempmili2 = 0;

//rotary encoder variables
boolean cflag = false;

Encoder knobLeft(2, 3);

float positionLeft  = -999;


int screen = 1;
//Buttons

int menubut = 10;
//temp values
 float tempval;
void setup(void)
{
  
  configuration.destemp = 70;
  // start serial port
  Serial.begin(9600);
  LCD.begin(9600);
  //savestartup();
  clearLCD();


  pinMode (relay1,OUTPUT);
  pinMode (relay2,OUTPUT);

  EEPROM_readAnything(0, configuration);
  pinMode (menubut,INPUT);
  
  
 delay(2000);

}

void loop(void)
{ 
  // Poll sensors on delay free timer  
  if(millis() > (2000)+tempmili)
  {
    pollsensors();
    clearLCD();
  }


  long newLeft ;
  newLeft = knobLeft.read();

  if (newLeft != positionLeft) {
    Serial.print("Left = ");
    Serial.print(newLeft);

    Serial.println();
    positionLeft = newLeft;
  }



  if (Serial.available()){
    char string[32] = ""; 
    int availableBytes = Serial.available();
    for(int i=0; i<availableBytes; i++)
    {
      string[i] = Serial.read();      
    } 

    String command = string;
    int digi = atoi(string);
    if(command != "") Serial.println(command);


    if(command == "on") backlightOn();
    if(command == "off") backlightOff();
    if(command == "init") initeeprom();
    if(command == "dump") dumpeeprom();
    
  }





 if (digitalRead(menubut) == HIGH) {
   
   //screens 2 & 3 accept user input - record changes if present and increment menu
   switch(screen){
     case 2:
       if(configuration.destemp != tempval) //changes were made we should update the data structure and save.
          {
          configuration.destemp = tempval;
          save();
          }
   
     break; 
     case 3:
       if(configuration.deshum != tempval) //changes were made we should update the data structure and save.
          {
          configuration.deshum = tempval;
          save();
          }
   

     break;
     
     
     
   }
   
   
   
   screen++;
   
   //Clear rotary encoder value - to prevent changes to other screens
   knobLeft.write(0);
   delay(1000);
 }


if(tempf>configuration.destemp) {
 digitalWrite(relay1, HIGH);
}else{
   digitalWrite(relay1, LOW);
  
  
}

if(relh<configuration.deshum) {
 digitalWrite(relay2, HIGH);
}else{
   digitalWrite(relay2, LOW);
  
  
}


  updateLCD(screen);
  //Serial.print(millis());
  //delay(100);
}


void updateLCD(int screenvar)
{
  switch(screenvar){
    case 1:
        selectLineOne(); 
        LCD.print(tempf);
        LCD.print("F ");
        LCD.print(relh);
        LCD.print("%"); 
        selectLineTwo();
      
       // float outputvalue;
     //   outputvalue = 70+ positionLeft * .01;  
      
        LCD.print(configuration.destemp);
        LCD.print("F");
        
        LCD.print(" ");
        LCD.print(configuration.deshum);
        LCD.print("%");
       
       
       /* if (tempf > configuration.destemp){
          LCD.print(" Above");
          digitalWrite(relay1, HIGH);
        }
        else{
          LCD.print(" Below");
          digitalWrite(relay1, LOW);
        }
        
        */
        break;
  case 2:
    selectLineOne(); 
    LCD.print("Please set Temp");
    selectLineTwo();
    
   
    tempval = configuration.destemp + positionLeft * .01;
    LCD.print( tempval);
    
    break;
  case 3:
    selectLineOne(); 
    LCD.print("Please set Hum%");
    selectLineTwo();
    
   
    tempval = configuration.deshum + positionLeft * .01;
    LCD.print( tempval);
    LCD.print("%");
    break;
  default:
  screen = 1;
  
  
  }

}


void pollsensors()
{

  tempmili = millis();
  DHT22_ERROR_t errorCode;
  //Serial.print("Requesting data...");
  errorCode = myDHT22.readData();
  switch(errorCode)
  {
  case DHT_ERROR_NONE:
    //Serial.print("Got Data ");

    tempc = myDHT22.getTemperatureC();
    tempf = Farenhot(tempc);
    relh = myDHT22.getHumidity();
    //
    // Serial.print(tempc);
    //Serial.print("C ");

    break;
  case DHT_ERROR_CHECKSUM:
    Serial.print("check sum error ");
    Serial.print(myDHT22.getTemperatureC());
    Serial.print("C ");
    Serial.print(myDHT22.getHumidity());
    Serial.println("%");
    break;
  case DHT_BUS_HUNG:
    Serial.println("BUS Hung ");
    break;
  case DHT_ERROR_NOT_PRESENT:
    Serial.println("Not Present ");
    break;
  case DHT_ERROR_ACK_TOO_LONG:
    Serial.println("ACK time out ");
    tempc = 0;
    break;
  case DHT_ERROR_SYNC_TIMEOUT:
    Serial.println("Sync Timeout ");
    break;
  case DHT_ERROR_DATA_TIMEOUT:
    Serial.println("Data Timeout ");
    break;
  case DHT_ERROR_TOOQUICK:
    Serial.println("Polled to quick ");
    break;
  }



}



float Farenhot(float degc){
  return ((9 * degc)/5) + 32;

}


void selectLineOne(){  //puts the cursor at line 0 char 0.
  LCD.write(byte(0xFE));   //command flag
  LCD.print(128, BYTE);    //position
  delay(10);
}
void selectLineTwo(){  //puts the cursor at line 0 char 0.
  LCD.write(byte(0xFE));   //command flag
  LCD.print(192, BYTE);    //position
  delay(10);
}
void clearLCD(){
  LCD.print(0xFE, BYTE);   //command flag
  LCD.print(0x01, BYTE);   //clear command.
  delay(10);
}

void backlightOn(){  //turns on the backlight
  LCD.print(0x7C, BYTE);   //command flag for backlight stuff
  LCD.print(157, BYTE);    //light level.
  delay(10);
}
void backlightOff(){  //turns off the backlight
  LCD.print(0x7C, BYTE);   //command flag for backlight stuff
  LCD.print(128, BYTE);     //light level for off.
  delay(10);
}

void savestartup(){  
  selectLineOne();
  LCD.print("Zboard V1.0");
  selectLineTwo();
  LCD.print("By LDC");
  LCD.print(0x7C, BYTE);   

  LCD.print(0x0A, BYTE);     //light level for off.
  delay(10);
}
void serCommand(){   //a general function to call the command flag for issuing all other commands   
  LCD.print(0xFE, BYTE);
}

void backlightfull(){
  LCD.print(0x7C, BYTE); 
  LCD.print(0x80 | (29));
}

void resetLCD() { 
  LCD.print(0x07, BYTE); 
  LCD.print(18, BYTE); 
} 

/////
void aChange (){

  cflag = true;
}
void initeeprom(){
  dumpeeprom();
  //set to default values
  configuration.destemp = 70;
  configuration.deshum = 50;
  //Commit Changes
  EEPROM_writeAnything(0, configuration);
}

void dumpeeprom()
{
  Serial.print("Destemp=");
  Serial.println(configuration.destemp);
  
  Serial.print("DesHum=");
  Serial.println(configuration.deshum);
  
  
}

void save()
{
  EEPROM_writeAnything(0, configuration);
}

