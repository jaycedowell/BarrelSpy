#include <Adafruit_NeoPixel.h>

// Initialize the 5x8 NeoPixel Arduino shield on pin 6.  The control
// parameters are:
//   NEO_KHZ800  800 KHz bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
Adafruit_NeoPixel strip = Adafruit_NeoPixel(40, 6, NEO_GRB + NEO_KHZ800);

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

// Given a voltage between 0 and 3 V, convert it to a distance in cm 
// assuming a Sharp GP2Y0A21YK0F sensor.  This function uses a quadratic
// fit to the voltage vs. inverse distance figure shown in the sensor 
// data sheet.
float voltage_to_distance(float voltage) {
  float inverseD;
  
  // Clamp the voltage to the 0 to 2.9 V range
  if( voltage < 0.0 ) {
    voltage = 0.0;
  }
  if( voltage > 2.9 ) {
    voltage = 2.9;
  }
  
  // Compute the inverse distance and clamp the low end at 0.0125 1/cm
  inverseD = 0.005313*voltage*voltage + 0.031101*voltage - 0.000200;
  if( inverseD < 0.0125 ) {
    inverseD = 0.0125;
  }
  
  // Invert and return
  return 1.0/inverseD;
}

void setup() {
  // Set up the NeoPixels so that all are off
  strip.begin();
  strip.show();
  
  // Power on NeoPixel test sequence - red, green, blue, and off
  colorWipe(strip.Color(25,  0,  0), 50); // Red
  colorWipe(strip.Color( 0, 25,  0), 50); // Green
  colorWipe(strip.Color( 0,  0, 25), 50); // Blue
  colorWipe(strip.Color( 0,  0,  0), 50); // Off

  // Dim the display (for now)
  strip.setBrightness(32);
}

// Kalman filtering setup
int sensor, ref;
float Q = 3e-4;
float R = 5*5;
float x = 150, xm1, P = 150, Pm1;
float value, K;

// "Bin full" state variable
boolean full = false;

// Other variables
int i = 0, k = 0;

void loop() {
  // ADC reads + 5 ms delay since we only get one value every 5 ms
  ref = analogRead(A0);
  sensor = analogRead(A1);
  delay(5);

  // Sensor: ADC counts -> voltage -> distance
  value = voltage_to_distance(sensor * 5.0 / 1023.0);

  // Time update
  xm1 = x;
  Pm1 = P + Q;

  // Measurement update
  K = Pm1 / (Pm1 + R);
  x = xm1 + K*(value - xm1);
  P = (1 - K)*Pm1;

  // Reference: ADC counts -> voltage -> distance
  value = voltage_to_distance(ref * 5.0 / 1023.0);
  
  // Bar graph + reference marker
  for(i=0; i<8; i++) {
    float level = 80 - 10*i;
    if( x <= level ) {
      strip.setPixelColor(i, 0, 255, 0);
    } else {
      strip.setPixelColor(i, 0, 0, 0);
    }

    if( (value <= level) && (value > level - 10) ) {
      strip.setPixelColor(i+8, 255, 255, 0);
    } else {
      strip.setPixelColor(i+8, 0, 0, 0);
    }
  }

  // Bin full condition + alert
  // NOTE:  The full condition can only be cleared by a reset of the device
  if( x <= value ) {
    full = true;
  }
  if( full ) {
    long j = millis() % 300;
    for(i=0; i<8; i++) {
      if( (j < 150) || (i % 2== k) ) {
        strip.setPixelColor(i+32, 255, 255, 255);
        strip.setPixelColor(i+24, 0, 0, 0);
        strip.setPixelColor(i+16, 255, 255, 255);
      } else {
        strip.setPixelColor(i+32, 0, 0, 0);
        strip.setPixelColor(i+24, 255, 255, 255);
        strip.setPixelColor(i+16, 0, 0, 0);
      }
    }
    if( j < 10 ) {
      k++;
      if( k == 2 ) {
        k = 0;
      }
    }
  }
  
  strip.show();
}
