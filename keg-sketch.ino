/**********************************************************
This is example code for using the Adafruit liquid flow meters. 
Tested and works great with the Adafruit plastic and brass meters
    ------> http://www.adafruit.com/products/828
    ------> http://www.adafruit.com/products/833
Connect the red wire to +5V, 
the black wire to common ground 
and the yellow sensor wire to pin #7
Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!
Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above must be included in any redistribution
**********************************************************/
#include "LiquidCrystal.h"
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// which pin to use for reading the sensor? can use any pin!
#define FLOWSENSORPIN 7
#define KEGSIZE 1.5//29.33

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

//thermometer vars
int sensorPin = 0;
int tempC, tempF;



void setup() {
   Serial.begin(9600);
   Serial.print("Flow sensor test!");
   lcd.begin(16, 2);
   
   pinMode(FLOWSENSORPIN, INPUT);
   digitalWrite(FLOWSENSORPIN, HIGH);
   lastflowpinstate = digitalRead(FLOWSENSORPIN);
   useInterrupt(true);

}

void loop()                     // run over and over again
{ 
//get temperature
  tempC = get_temperature(sensorPin);
  tempF = celsius_to_fahrenheit(tempC);
  lcd.setCursor(0,0);
  lcd.print("Spotted Cow");
  Serial.print("Freq: "); Serial.println(flowrate);
  Serial.print("Pulses: "); Serial.println(pulses, DEC);
  
  // if a plastic sensor use the following calculation
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;

  Serial.print(liters); Serial.println(" Liters");
  String bummer = ". BUMMER.";
  lcd.setCursor(0, 1);
  double beerLeft = ((KEGSIZE - liters) / KEGSIZE * 100);
  if (beerLeft < 0) {
    beerLeft = 0;

  }
  int beerLeftPercentage = (int) beerLeft;
  lcd.print(beerLeftPercentage); lcd.print("% LEFT"); 
  if (beerLeftPercentage < 1) {
    lcd.print(bummer);
  }
  lcd.print("   "); lcd.print(tempF); lcd.print((char)223); lcd.print("F    ");
  delay(500);
}



int get_temperature(int pin) {
// We need to tell the function which pin the sensor is hooked up to. We're using
// the variable pin for that above
// Read the value on that pin
  int temperature = analogRead(pin);
  // Calculate the temperature based on the reading and send that value back
  float voltage = temperature * 5.0;
  voltage = voltage / 1024.0;
  return ((voltage - 0.5) * 100);
}

int celsius_to_fahrenheit(int temp) {
  return (temp * 9 / 5) + 32;
}

