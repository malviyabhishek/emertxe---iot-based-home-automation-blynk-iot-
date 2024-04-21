#include "ldr.h"
#include "Arduino.h"
#include "main.h"

void init_ldr(void)
{
   pinMode(GARDEN_LIGHT, OUTPUT);
   
}
unsigned int inputVal = 0;

void brightness_control(void)
{
  //read the values from LDR sensor

   inputVal = analogRead(LDR_SENSOR);

  //scale it down from (0 to 1023)to (255 to 0)

  inputVal =  (1023 - inputVal)/4;

  // write values to the led

   analogWrite (GARDEN_LIGHT, inputVal);

   delay (100);
}
