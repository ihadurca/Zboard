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

NewSoftSerial LCD(0,LCDPin); 
#define DHT22_PIN 4
// Connect a 4.7K resistor between VCC and the data pin (strong pullup)

// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);




// setup our temp variables
float tempc, tempf, relh;
unsigned long tempmili = 0;
unsigned long tempmili2 = 0;

//rotary encoder variables
boolean cflag = false;

Encoder knobLeft(2, 3);

float positionLeft  = -999;

//Set screen to 1
int screen = 1;


//Button Pins
int menubut = 10;

//Define Relay Pins
int relay1 = 8;
int relay2 = 9;

  
//temp values
 float tempvalflt;
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
       if(configuration.destemp != tempvalflt) //changes were made we should update the data structure and save.
          {
          configuration.destemp = tempvalflt;
          save();
          }
   
     break; 
     case 3:
       if(configuration.deshum != tempvalflt) //changes were made we should update the data structure and save.
          {
          configuration.deshum = tempvalflt;
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
      
        LCD.print(configuration.destemp);
        LCD.print("F");
        
        LCD.print(" ");
        LCD.print(configuration.deshum);
        LCD.print("%");
       
        break;
  case 2:
    selectLineOne(); 
    LCD.print("Please set Temp");
    selectLineTwo();
    
   
    tempvalflt = configuration.destemp + positionLeft * .01;
    LCD.print( tempvalflt);
    
    break;
  case 3:
    selectLineOne(); 
    LCD.print("Please set Hum%");
    selectLineTwo();
    
   
    tempvalflt = configuration.deshum + positionLeft * .01;
    LCD.print( tempvalflt);
    LCD.print("%");
    break;
  default:
  // No other cases match set to 1
  screen = 1;
  
  
  }

}


void pollsensors()
{

  //We output all error messages to the hardware serial line.
  
  
  tempmili = millis();
  DHT22_ERROR_t errorCode;
  errorCode = myDHT22.readData();
  switch(errorCode)
  {
  case DHT_ERROR_NONE:
    tempc = myDHT22.getTemperatureC();
    tempf = convtof(tempc);
    relh = myDHT22.getHumidity();
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



float convtof(float degc){
  return ((9 * degc)/5) + 32;
}

//Assuming Sparkfun Serial LCD for LCD - modify as needed for any other type.

void selectLineOne(){  //puts the cursor at line 0 char 0.
  LCD.print(0xFE, BYTE);   //command flag
  LCD.print(128, BYTE);    //position
  delay(10);
}
void selectLineTwo(){  //puts the cursor at line 0 char 0.
  LCD.print(0xFE, BYTE);   //command flag
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


// Note!! - for data structure to work correctly the structure must be initialized in eeprom!!! - if this is not done all dependent values will be zero!
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

