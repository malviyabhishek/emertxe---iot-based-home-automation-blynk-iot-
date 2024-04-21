/*************************************************************
Title         :   Home automation using blynk
Description   :   To control light's brigntness with brightness,monitor temperature , monitor water level in the tank through blynk app
Pheripherals  :   Arduino UNO , Temperature system, LED, LDR module, Serial Tank, Blynk cloud, Blynk App.
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "**************"
#define BLYNK_TEMPLATE_NAME "home AUTOMATION "
#define BLYNK_AUTH_TOKEN "********************"


// Comment this out to disable prints 
//#define BLYNK_PRINT Serial

#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

#include "main.h"
#include "temperature_system.h"
#include "ldr.h"
#include "serial_tank.h"

char auth[] = BLYNK_AUTH_TOKEN;
bool heater_sw;
bool inlet_sw, outlet_sw;
unsigned int tank_volume;

BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// This function is called every time the Virtual Pin 0 state changes
//To turn ON and OFF cooler based virtual PIN value
BLYNK_WRITE(COOLER_V_PIN)
{
  int value = param.asInt();
  
  //if cooler button is ON in Blynk APP, turn ON cooler
  if (value)
  {
    cooler_control(ON);
    lcd.setCursor(7,0);
    lcd.print("CL_R ON ");
  }
  //if cooler button is OFF in Blynk APP, turn OFF cooler
  else
  {
    cooler_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("CL_R OFF");
  }
  
  
}
//To turn ON and OFF heater based virtual PIN value
BLYNK_WRITE(HEATER_V_PIN )
{
    heater_sw = param.asInt();
  
  //if heater button is ON in Blynk APP, turn ON heater
  if (heater_sw)
  {
    heater_control(ON);
    lcd.setCursor(7,0);
    lcd.print("HT_R ON ");
  }
  //if heater button is OFF in Blynk APP, turn OFF heater
  else
  {
    heater_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("HT_R OFF");
  }
  
  
}
//To turn ON and OFF inlet vale based virtual PIN value
BLYNK_WRITE(INLET_V_PIN)
{
  inlet_sw = param.asInt();
  
  // if inlet valve button at logic high turn ON inlet valve else OFF
  if(inlet_sw)
  {
    enable_inlet();
// to print the status of valve on CLCD
    lcd.setCursor(7,1);
    lcd.print("IN_FL ON ");
  }
  else
  {
    disable_inlet();
// to print the status of valve on CLCD
    lcd.setCursor(7,1);
    lcd.print("IN_FL OFF");
  }

}
//To turn ON and OFF outlet value based virtual switch value
BLYNK_WRITE(OUTLET_V_PIN)
{
  outlet_sw = param.asInt();
  
  // if outlet valve button at logic high turn ON outlet valve else OFF
  if(outlet_sw)
  {
    enable_outlet();
// to print the status of valve on CLCD
    lcd.setCursor(7,1);
    lcd.print("OUT_FL ON ");
  }
  else
  {
    disable_outlet();
// to print the status of valve on CLCD
    lcd.setCursor(7,1);
    lcd.print("OUT_FL OFF");
  }

}
    
// To display temperature and water volume as gauge on the Blynk App
void update_temperature_reading()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(TEMPERATURE_GAUGE, read_temperature());
  Blynk.virtualWrite(WATER_VOL_GAUGE, volume());
  
}

//To turn off the heater if the temperature raises above 35 deg C
void handle_temp(void)
{
  // compare temp with 35 deg and check if heater is ON
  if((read_temperature() > float(35)) && heater_sw)
  {
    // to turn OFF heater
    heater_sw = 0;
    heater_control(OFF);
    
    // display notification on CLCD
    lcd.setCursor(7,0);
    lcd.print("HT_R OFF");

    // display notification on Blynk
     Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Temperature is above 35 Deg Celsius\n");
     Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Turning OFF HEATER\n");

     // to reflect OFF on the heater button
     Blynk.virtualWrite(HEATER_V_PIN, OFF);
  }
}

//To control water volume above 2000ltrs
void handle_tank(void)
{
  // read the volume of water with 2000 litres and check the status of the inlet valve
  if ((tank_volume < 2000) && (inlet_sw == OFF))
  {
     enable_inlet();
// to print the status of valve on CLCD
    lcd.setCursor(7,1);
    lcd.print("IN_FL ON ");
    inlet_sw = ON;

    // update the inlet button status of the blynk app
    Blynk.virtualWrite(INLET_V_PIN, ON);

    // print notification on blynk app 
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Water volume is less than 2000 L \n");
     Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Turning ON inlet valve\n");
  }

  // check if tank is full then turn OFF inlet valve 
  if ((tank_volume == 3000) && (inlet_sw == ON))
  {
     disable_inlet();
// to print the status of valve on CLCD
    lcd.setCursor(7,1);
    lcd.print("IN_FL OFF");
    inlet_sw = OFF;

    // update the inlet button status of the blynk app
    Blynk.virtualWrite(INLET_V_PIN, OFF);

    // print notification on blynk app 
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Tank is Full \n");
     Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Turning OFF inlet valve\n");
  }

}


void setup(void)
{
    // connecting arduino to blynk server
    Blynk.begin(auth);
    /*initialize the lcd*/
    lcd.init();  
    /*turn the backlight */                   
    lcd.backlight();
    /*clear the clcd*/
    lcd.clear();
    /*cursor to the home */
    lcd.home();
    // initializing Garden light/LED as output pin
    init_ldr(); 
//intializing temp sensor
    init_temperature_system();
    
    lcd.setCursor(0,0);
    lcd.print("T=");

    // set cursor to second line to display volume of water
    lcd.setCursor(0,1);
    lcd.print("V=");

    // intialize the serial tank
    init_serial_tank();

    // update temp on Blnk app for every 0.5 sec
    timer.setInterval(500L,update_temperature_reading);
}

void loop(void) 
{
    // control the brightness of Garden lightusing LDR sensor
     brightness_control();

     String temperature;
     temperature = String(read_temperature(), 2);
     // display the temp on CLCD
     lcd.setCursor(2,0); 
     lcd.print(temperature);

     // display volume of tank on CLCD
     tank_volume = volume();
     lcd.setCursor(2,1); 
     lcd.print(tank_volume);


    // to check the thresold temperature and control heater
     handle_temp();

     // to check the thresold volume and control valve
     handle_tank();

     // To run blnkk App
     Blynk.run();

     timer.run();
}
