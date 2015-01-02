#include <Adafruit_NeoPixel.h>

// Initialize the 5x8 NeoPixel Arduino shield on pin 6.  The control
// parameters are:
//   NEO_KHZ800  800 KHz bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
Adafruit_NeoPixel strip = Adafruit_NeoPixel(40, 6, NEO_GRB + NEO_KHZ800);

// Global state variables
int refPixelLevel, refPixelLevelOld;
unsigned long refLastChanged;

// Setup the serial port and NeoPixels, and create the initial monitoring
// state.
void setup() {
  // Set for a baud rate of 9,600
  Serial.begin(9600);
  
  // Set up the NeoPixels so that all are off
  strip.begin();
  strip.show();
  
  // Power on NeoPixel test sequence - red, green, blue, and off
  colorWipe(strip.Color(25,  0,  0), 50); // Red
  colorWipe(strip.Color( 0, 25,  0), 50); // Green
  colorWipe(strip.Color( 0,  0, 25), 50); // Blue
  colorWipe(strip.Color( 0,  0,  0), 50); // Off
  
  // Power light
  strip.setPixelColor(7, strip.Color(0, 10, 6));
  
  // Set the initial state
  refPixelLevel = -1;
  refPixelLevelOld = -2;
  refLastChanged = 0;
}

void loop() {
  // Loop control
  uint16_t i, j, k;
  
  // Read the current voltages for the reference and the sensor
  float refVoltage, sensorVoltage;
  refVoltage = 5.0*analogRead(A0)/1023.0;
  sensorVoltage = 5.0*analogRead(A1)/1023.0;
  
  // Convert the voltages to distances
  float refD, sensorD;
  refD = voltage2distance(refVoltage);
  sensorD = voltage2distance(sensorVoltage);
  
  // Convert the refence distance to a pixel count and clamp it to the 1 
  // to 8 range
  refPixelLevel = (int) round( 7 * (80 - refD) / (80 - 10) ) + 1;
  if( refPixelLevel < 1 ) {
    refPixelLevel = 1;
  }
  if( refPixelLevel > 8 ) {
    refPixelLevel = 8;
  }
  
  // Part 1 - Has the reference distance changed since the last polling?
  if( refPixelLevel != refPixelLevelOld || millis() - refLastChanged < 4000 ) {
    /* Lower level markers */
//    for(i=0; i<40; i+=8) {
//      strip.setPixelColor(i, strip.Color(0, 0, 25));
//    }
    
    /* Center level markers */
//    for(i=12; i<32; i+=8) {
//      strip.setPixelColor(i,   strip.Color(0, 25, 0));
//      strip.setPixelColor(i-1, strip.Color(0, 25, 0));
//    }
    
    /* Upper level markers */
//    for(i=7; i<40; i+=8) {
//      strip.setPixelColor(i, strip.Color(25, 0, 0));
//    }
    /* Reference mode indicator */
    setPixelColor(39, strip.Color(6, 0, 10));
    
    /* Level scales */
    for(i=8; i<16; i++) {
      setPixelColor(i,    strip.Color(3*(i-8)+1, 0, 0));
      setPixelColor(i+16, strip.Color(3*(i-8)+1, 0, 0));
    }
    
    /* The reference level indicator */
    for(i=16; i<24; i++){
      if( i-16 < refPixelLevel ) {
        setPixelColor(i, strip.Color(12, 12, 0));
      } else {
        setPixelColor(i, strip.Color(0, 0, 0));
      }
    }
    strip.show();
    delay(50);
    
    /* Update the state */
    if( refPixelLevel != refPixelLevelOld ) {
      refLastChanged = millis();
    }
    refPixelLevelOld = refPixelLevel;
  } else {
    // Clear the pixels to get ready for the next iteration
    for(i=0; i<strip.numPixels(); i++){
      setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
    delay(50);
  }
  
  // Part 2 - Should a notification be displayed?
  if( sensorD <= refD ) {
    for(k=0; k<5; k++) {
      /* Fade from off to white */
      for(j=0; j<30; j++) {
        for(i=0; i<strip.numPixels(); i++){
          strip.setPixelColor(i, strip.Color(j, j, j));
        }
        strip.show();
        delay(50);
      }
      
      /* All yellow */
      for(i=0; i<strip.numPixels(); i++){
        strip.setPixelColor(i, strip.Color(30, 19, 0));
      }
      strip.show();
      delay(100);
      
      /* All white */
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(30, 30, 30));
      }
      strip.show();
      delay(100);
    }
    
    /* All off plus the power light */
    for(i=0; i<strip.numPixels(); i++){
      strip.setPixelColor(i, strip.Color(30, 19, 0));
    }
    strip.setPixelColor(7, strip.Color(0, 10, 6));
    strip.show();
    delay(50);
  }
  
}

// Given a voltage between 0 and 3 V, convert it to a distance in cm 
// assuming a Sharp GP2Y0A21YK0F sensor.  This function uses a quadratic
// fit to the voltage vs. inverse distance figure shown in the sensor 
// data sheet.
float voltage2distance(float voltage) {
  float inverseD;
  
  // Clamp the voltage to the 0 to 3 V range
  if( voltage < 0.0 ) {
    voltage = 0.0;
  }
  if( voltage > 3.0 ) {
    voltage = 3.0;
  }
  
  // Compute the inverse distance and clamp the low end at 0.01 1/cm
  inverseD = 0.005313*voltage*voltage + 0.031101*voltage - 0.000200;
  if( inverseD < 0.01 ) {
    inverseD = 0.01;
  }
  
  // Invert and return
  return 1.0/inverseD;
}

void setPixelColor(uint16_t n, uint32_t c) {
  if( n != 7 ) {
    strip.setPixelColor(n, c);
  }
}

// Fill the dots one after the other with a color - from the AdaFruit 
// NeoPixel Library
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels()/2; i++) {
      strip.setPixelColor(i, c);
      strip.setPixelColor(strip.numPixels()-1-i, c);
      strip.show();
      delay(wait);
  }
}

// Theatre-style crawling lights - from the AdaFruit NeoPixel Library
void theaterChase(uint32_t c, uint8_t wait) {
  for(uint16_t j=0; j<10; j++) {  //do 10 cycles of chasing
    for(uint16_t k=0; k<3; k++) {
      for(uint16_t i=0; i<strip.numPixels(); i+=3) {
        strip.setPixelColor(i+k, c);    //turn every third pixel on
      }
      strip.show();
      delay(wait);
     
      for(uint16_t i=0; i<strip.numPixels(); i+=3) {
        strip.setPixelColor(i+k, 0);        //turn every third pixel off
      }
    }
  }
}
