// aArtisanQ_PID.ino
// ------------

// Written to support the Artisan roasting scope //http://code.google.com/p/artisan/

//   Heater is controlled from OT1 using a zero cross SSR (integral pulse control)
//   AC fan is controlled from OT2 using a random fire SSR (phase angle control)
//   zero cross detector (true on logic low) is connected to I/O3

// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2011, MLG Properties, LLC
// All rights reserved.
//
// Contributor:  Jim Gallt
//
// Redistribution and use in source and binary forms, with or without modification, are 
// permitted provided that the following conditions are met:
//
//   Redistributions of source code must retain the above copyright notice, this list of 
//   conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright notice, this list 
//   of conditions and the following disclaimer in the documentation and/or other materials 
//   provided with the distribution.
//
//   Neither the name of the copyright holder(s) nor the names of its contributors may be 
//   used to endorse or promote products derived from this software without specific prior 
//   written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------------------------------------------------------------------------------------------

#define BANNER_ARTISAN "aArtisanQ_PID Read1"

// Revision history:
// 20110408 Created.
// 20110409 Reversed the BT and ET values in the output stream.
//          Shortened the banner display time to avoid timing issues with Artisan
//          Echo all commands to the LCD
// 20110413 Added support for Artisan 0.4.1
// 20110414 Reduced filtering levels on BT, ET
//          Improved robustness of checkSerial() for stops/starts by Artisan
//          Revised command format to include newline character for Artisan 0.4.x
// 20110528 New command language added (major revision)
//          Use READ command to poll the device for up to 4 temperature channels
//          Change temperature scale using UNITS command
//          Map physical channels on ADC to logical channels using the CHAN command
//          Select SSR output duty cycle with OT1 and OT2 commands
//          Select PWM logic level output on I/O3 using IO3 command
//          Directly control digital pins using DPIN command (WARNING -- this might not be smart)
//          Directly control analog pins using APIN command (WARNING -- this might not be smart)
// 20110601 Major rewrite to use cmndproc.h library
//          RF2000 and RC2000 set channel mapping to 1200
// 20110602 Added ACKS_ON to control verbose output
// ----------- Version 1.10
// 20111011 Added support for type J and type T thermocouples
//          Better error checking on EEPROM reads
// ----------- aArtsianQ version 0.xx
// 20111031 Created.
// ----------- aArtisanQ beta1
// 20111101 Beta 1 release


// ----------- aArtisanQ_PID (Brad Collins)
// 20120915 Created.
//          Added PID Library
//          Added code for analogue inputs
// 20120916 Added PID command allowing PID control to be activated and deactivated from Artisan
//          Added roast clock. Can be reset with PID;TIME command
//          Removed LCD ambient temp display and added roast clock display
// 20120918 Added code to read profile from EEPROM and interpolate to calculate setpoint. Time/Temp profiles
//          Added code to end PID control when end of profile is reached
//          Added additional PID command allowing roast profile to be selected
//          Added additional PID command allowing PID tunings to be adjusted on the fly (required MAX_TOKENS 5 in cmndproc.h library)
// 20120920 Added RoR calcs
// 20120921 Updated RoR calcs to better handle first loop issue (RoR not calculated in first loop)
//          Stopped ANLG1 being read during PID control
//          Added code to convert PID Setpoint temps to correct units.  Added temperature units data to profile format
//          Serial command echo to LCD now optional
// 20120922 Added support for LCDapter buttons and LEDs (button 1 currently activates or deactivates PID control if enabled)
//          Added code to allow power to OT1 to be cut if OT2 is below OT1_CUTOFF percentage as defined in user.h.  For heater protection if required. Required modification to phase_ctrl.cpp
//          Added code to allow OT2 to range between custom min and max percentages (defined in user.h)
// 20121007 Fixed PID tuning command so it handles doubles
//          Added inital PID tuning parameters in user.h
// 20121013 Added code to allow Artisan plotting of levelOT1 and levelOT2 if PLOT_POWER is defined in user.h
//          Swapped location of T1 and T2 on LCD display and renamed to ET and BT
// 20121014 Enhanced LCD display code and added support for 4x20 LCDs. Define LCD_4x20 in user.h
// 20121021 Added optional limits for Analogue1
// 20121120 Added support for ANDROID logging
// 20121213 Added UP and DOWN parameters for OT1 and OT2 commands.  Increments or decrements power levels by 5%
// 20130116 Added user adjustable analogue input rounding (ANALOGUE_STEP) in user.h
// 20130119 aArtisanQ_PID release 3_5
//          Added code to allow for additional LCD display modes
// 20130120 Added ability to change roast profile using LCD and buttons
// 20130121 Tidied up button press code
// 20130203 Permits use of different TC types on individual channels as in aArtisan 2.10
//          Updated temperature sample filtering to match aArtisan 2.10
// 20130406 Added GO and STOP commands to use with Artisan 'Charge' and 'End' buttons
// 20140127 aArtisanQ_PID release 4_0 created
//          Added support for Roastlogger roasting software (responds to LOAD, POWER and FAN commands. Sends rorT1=, T1=, rorT2=, T2= and power levels to roastlogger)
// 20140128 Improved handling of heater and fan power limits
// 20140213 aArtisanQ_PID release 4_2
// 20140214 Added option in useh.h to define software mode (Artisan, Roastlogger or ANDROID)
//          Fixed? bug causing crashes when receiving READ commands
// 20140214 aArtisanQ_PID release 4_3

// this library included with the arduino distribution
#include <Wire.h>

// The user.h file contains user-definable and some other global compiler options
// It must be located in the same folder as aArtisan.pde
#include "user.h"

// command processor declarations -- must be in same folder as aArtisan
#include "cmndreader.h"

#ifdef MEMORY_CHK
// debugging memory problems
#include "MemoryFree.h"
#endif

// code for integral cycle control and phase angle control
#include "phase_ctrl.h"

// these "contributed" libraries must be installed in your sketchbook's arduino/libraries folder
#include <cmndproc.h> // for command interpreter
#include <thermocouple.h> // type K, type J, and type T thermocouple support
#include <cADC.h> // MCP3424
//#include <PWM16.h> // for SSR output
#ifdef LCD
#include <cLCD.h> // required only if LCD is used
#endif

// ------------------------ other compile directives
#define MIN_DELAY 300   // ms between ADC samples (tested OK at 270)
#define DP 1  // decimal places for output on serial port
#define D_MULT 0.001 // multiplier to convert temperatures from int to float
#define DELIM "; ,=" // command line parameter delimiters

#include <mcEEPROM.h>
mcEEPROM eeprom;
calBlock caldata;

float AT; // ambient temp
float T[NC];  // final output values referenced to physical channels 0-3
int32_t ftemps[NC]; // heavily filtered temps
int32_t ftimes[NC]; // filtered sample timestamps
int32_t ftemps_old[NC]; // for calculating derivative
int32_t ftimes_old[NC]; // for calculating derivative
float RoR[NC]; // final RoR values
uint8_t actv[NC];  // identifies channel status, 0 = inactive, n = physical channel + 1

#ifdef CELSIUS // only affects startup conditions
boolean Cscale = true;
#else
boolean Cscale = false;
#endif

int levelOT1, levelOT2;  // parameters to control output levels
#ifdef MEMORY_CHK
uint32_t checktime;
#endif

#ifdef ANALOGUE1
  uint8_t anlg1 = 0; // analog input pins
  int32_t old_reading_anlg1; // previous analogue reading
  boolean analogue1_changed;
#endif

#ifdef ANALOGUE2
  uint8_t anlg2 = 1; // analog input pins
  int32_t old_reading_anlg2; // previous analogue reading
  boolean analogue2_changed;
#endif

#ifdef PID_CONTROL
  #include <PID_v1.h>

  //Define PID Variables we'll be connecting to
  double Setpoint, Input, Output;

  //Specify the links and initial tuning parameters
  PID myPID(&Input, &Output, &Setpoint,2,5,1, DIRECT);

  int profile_number; // number of the profile for PID control
  int profile_ptr; // EEPROM pointer for profile data
  char profile_name[40];
  char profile_description[80];
  int profile_number_new; // used when switching between profiles
  
  int times[2], temps[2]; // time and temp values read from EEPROM for setpoint calculation
  
  char profile_CorF; // profile temps stored as Centigrade or Fahrenheit

#endif

boolean artisan_logger = false;
//boolean ANDROID = false; // set initial state for ANDROID flag
//boolean roastlogger = false; // set initial state for roastlogger flag
uint32_t counter; // second counter
uint32_t next_loop_time; // 
boolean first;

// class objects
cADC adc( A_ADC ); // MCP3424
ambSensor amb( A_AMB ); // MCP9800
filterRC fT[NC]; // filter for logged ET, BT
filterRC fRise[NC]; // heavily filtered for calculating RoR
filterRC fRoR[NC]; // post-filtering on RoR values
//PWM16 ssr;  // object for SSR output on OT1, OT2
CmndInterp ci( DELIM ); // command interpreter object

// array of thermocouple types
tcBase * tcp[4];
TC_TYPE1 tc1;
TC_TYPE2 tc2;
TC_TYPE3 tc3;
TC_TYPE4 tc4;

// ---------------------------------- LCD interface definition
#ifdef LCD
// LCD output strings
char st1[6],st2[6];
int LCD_mode = 0;
#ifdef LCDAPTER
  #include <cButton.h>
  cButtonPE16 buttons; // class object to manage button presses
  #define BACKLIGHT lcd.backlight();
  cLCD lcd; // I2C LCD interface
#else // parallel interface, standard LiquidCrystal
  #define BACKLIGHT ;
  #define RS 2
  #define ENABLE 4
  #define D4 7
  #define D5 8
  #define D6 12
  #define D7 13
  LiquidCrystal lcd( RS, ENABLE, D4, D5, D6, D7 ); // standard 4-bit parallel interface
#endif
#endif
// --------------------------------------------- end LCD interface

// T1, T2 = temperatures x 1000
// t1, t2 = time marks, milliseconds
// ---------------------------------------------------
float calcRise( int32_t T1, int32_t T2, int32_t t1, int32_t t2 ) {
  int32_t dt = t2 - t1;
  if( dt == 0 ) return 0.0;  // fixme -- throw an exception here?
  float dT = ( convertUnits( T2 ) - convertUnits( T1 ) ) * D_MULT;
  float dS = dt * 0.001; // convert from milli-seconds to seconds
  return ( dT / dS ) * 60.0; // rise per minute
}


// ------------- wrapper for the command interpreter's serial line reader
void checkSerial() {
  const char* result = ci.checkSerial();
  if( result != NULL ) { // some things we might want to do after a command is executed
    #if defined LCD && defined COMMAND_ECHO
    lcd.setCursor( 0, 0 ); // echo all commands to the LCD
    lcd.print( result );
    #endif
    #ifdef MEMORY_CHK
    Serial.print("# freeMemory()=");
    Serial.print(freeMemory());
    Serial.print("  ,  ");
    Serial.println( result );
    #endif
  }
}

// ----------------------------------
void checkStatus( uint32_t ms ) { // this is an active delay loop
  uint32_t tod = millis();
  while( millis() < tod + ms ) {
    checkSerial();
    #ifdef LCDAPTER
      checkButtons();
    #endif
  }
}

// ----------------------------------------------------
float convertUnits ( float t ) {
  if( Cscale ) return F_TO_C( t );
  else return t;
}

// ------------------------------------------------------------------
void logger() {

#ifdef ARTISAN
  // print ambient
  Serial.print( convertUnits( AT ), DP );
  // print active channels
  for( uint8_t jj = 0; jj < NC; ++jj ) {
    uint8_t k = actv[jj];
    if( k > 0 ) {
      --k;
      Serial.print(",");
      Serial.print( convertUnits( T[k] ) );

    }
  }
    
  #ifdef PLOT_POWER
  Serial.print(",");
  if( levelOT2 < OT1_CUTOFF ) { // send 0 if OT1 has been cut off
    Serial.print( 0 );
  }
  else {  
    Serial.print( levelOT1 );
  }
  Serial.print(",");
  Serial.print( levelOT2 );
  #endif  
    
  Serial.println();

#endif

#ifdef ROASTLOGGER
  for( uint8_t jj = 0; jj < NC; ++jj ) {
    uint8_t k = actv[jj];
    if( k > 0 ) {
      --k;
      Serial.print("rorT");
      Serial.print(k+1);
      Serial.print("=");
      Serial.println( RoR[k], DP );
      Serial.print("T");
      Serial.print(k+1);
      Serial.print("=");
      Serial.println( convertUnits( T[k] ) );
    }
  }
  Serial.print("Power%=");
  if( levelOT2 < OT1_CUTOFF ) { // send 0 if OT1 has been cut off
    Serial.println( 0 );
  }
  else {  
    Serial.println( levelOT1 );
  }
  Serial.print("Fan=");
  Serial.println( levelOT2 );
#endif

#ifdef ANDROID

  // print counter
  Serial.print( counter );
  Serial.print( "," );

  // print ambient
  Serial.print( convertUnits( AT ), DP );
  // print active channels
  for( uint8_t jj = 0; jj < NC; ++jj ) {
    uint8_t k = actv[jj];
    if( k > 0 ) {
      --k;
      Serial.print(",");
      Serial.print( convertUnits( T[k] ) );
      Serial.print(",");
      Serial.print( RoR[k], DP );
    }
  }
    
  #ifdef PLOT_POWER
  Serial.print(",");
  if( levelOT2 < OT1_CUTOFF ) { // send 0 if OT1 has been cut off
    Serial.print( 0 );
  }
  else {  
    Serial.print( levelOT1 );
  }
  Serial.print(",");
  Serial.print( levelOT2 );
  #endif  
    
  Serial.println();

#endif

}

// --------------------------------------------------------------------------
void get_samples() // this function talks to the amb sensor and ADC via I2C
{
  int32_t v;
  tcBase * tc;
  float tempF;
  int32_t itemp;
  float rx;

  uint16_t dly = amb.getConvTime(); // use delay based on slowest conversion
  uint16_t dADC = adc.getConvTime();
  dly = dly > dADC ? dly : dADC;
  
  for( uint8_t jj = 0; jj < NC; jj++ ) { // one-shot conversions on both chips
    uint8_t k = actv[jj]; // map logical channels to physical ADC channels
    if( k > 0 ) {
      --k;
      tc = tcp[k]; // each channel may have its own TC type
      adc.nextConversion( k ); // start ADC conversion on physical channel k
      amb.nextConversion(); // start ambient sensor conversion
      checkStatus( dly ); // give the chips time to perform the conversions

      if( !first ) { // on first loop dont save zero values
        ftemps_old[k] = ftemps[k]; // save old filtered temps for RoR calcs
        ftimes_old[k] = ftimes[k]; // save old timestamps for filtered temps for RoR calcs
      }
      
      ftimes[k] = millis(); // record timestamp for RoR calculations
      
      amb.readSensor(); // retrieve value from ambient temp register
      v = adc.readuV(); // retrieve microvolt sample from MCP3424
      tempF = tc->Temp_F( 0.001 * v, amb.getAmbF() ); // convert uV to Celsius

      // filter on direct ADC readings, not computed temperatures
      v = fT[k].doFilter( v << 10 );  // multiply by 1024 to create some resolution for filter
      v >>= 10; 
      AT = amb.getAmbF();
      T[k] = tc->Temp_F( 0.001 * v, AT ); // convert uV to Fahrenheit;

      ftemps[k] =fRise[k].doFilter( tempF * 1000 ); // heavier filtering for RoR

      if ( !first ) { // on first loop dont calc RoR
        rx = calcRise( ftemps_old[k], ftemps[k], ftimes_old[k], ftimes[k] );
        RoR[k] = fRoR[k].doFilter( rx / D_MULT ) * D_MULT; // perform post-filtering on RoR values
      }
    }
  }
  first = false;
};

#ifdef LCD
// --------------------------------------------
void updateLCD() {
  
  if( LCD_mode == 0 ) { // Display normal LCD screen
  
    lcd.setCursor(0,0);  
    if(counter/60 < 10) lcd.print("0"); lcd.print(counter/60); // Prob can do this better. Check aBourbon.
    lcd.print(":"); // make this blink?? :)
    if(counter - (counter/60)*60 < 10) lcd.print("0"); lcd.print(counter - (counter/60)*60);
    
  #ifdef LCD_4x20
  
  #ifdef COMMAND_ECHO
    lcd.print(" "); // overwrite artisan commands
  #endif
  
    // display the first 2 active channels encountered, normally BT and ET
    int it01;
    uint8_t jj,j;
    uint8_t k;
    for( jj = 0, j = 0; jj < NC && j < 2; ++jj ) {
      k = actv[jj];
      if( k != 0 ) {
        ++j;
        it01 = round( convertUnits( T[k-1] ) );
        if( it01 > 999 ) 
          it01 = 999;
        else
          if( it01 < -999 ) it01 = -999;
        sprintf( st1, "%4d", it01 );
        if( j == 1 ) {
          lcd.setCursor( 13, 0 );
          lcd.print("ET:");
        }
        else {
          lcd.setCursor( 13, 1 );
          lcd.print( "BT:" );
        }
        lcd.print(st1);  
      }
    }
    
  // AT
    it01 = round( convertUnits( AT ) );
    if( it01 > 999 ) 
      it01 = 999;
    else
      if( it01 < -999 ) it01 = -999;
    sprintf( st1, "%3d", it01 );
    lcd.setCursor( 6, 0 );
    lcd.print("AT:");
    lcd.print(st1);
    
  #ifdef PID_CONTROL
    if( myPID.GetMode() != MANUAL ) { // if PID is on then display PID: nnn% instead of OT1:
      lcd.setCursor( 0, 2 );
      lcd.print( "PID:" );
      if( levelOT2 < OT1_CUTOFF ) { // display 0% if OT1 has been cut off
      sprintf( st1, "%4d", (int)0 );     
      }
      else {
        sprintf( st1, "%4d", (int)levelOT1 );
      }
      lcd.print( st1 ); lcd.print("%");
      
      lcd.setCursor( 13, 2 ); // display setpoint if PID is on
      lcd.print( "SP:" );
      sprintf( st1, "%4d", (int)Setpoint );
      lcd.print( st1 );
    }
    else {
  #ifdef ANALOGUE1
      lcd.setCursor( 13, 2 );
      lcd.print("       "); // blank out SP: nnn if PID is off
  #else
      lcd.setCursor( 0, 2 );
      lcd.print("                    "); // blank out PID: nnn% and SP: nnn if PID is off and ANALOGUE1 isn't defined
  #endif // end ifdef ANALOGUE1
    }
  #endif // end ifdef PID_CONTROL
  
    // RoR
    lcd.setCursor( 0, 1 );
    lcd.print( "RoR:");
    sprintf( st1, "%4d", (int)RoR[ROR_CHAN] );
    lcd.print( st1 );
  
  
  #ifdef ANALOGUE1
  #ifdef PID_CONTROL
    if( myPID.GetMode() == MANUAL ) { // only display OT2: nnn% if PID is off so PID display isn't overwriten
      lcd.setCursor( 0, 2 );
      lcd.print("OT1:");
      if( levelOT2 < OT1_CUTOFF ) { // display 0% if OT1 has been cut off
      sprintf( st1, "%4d", (int)0 );     
      }
      else {
        sprintf( st1, "%4d", (int)levelOT1 );
      }
      lcd.print( st1 ); lcd.print("%");
    }
      
  #else // if PID_CONTROL isn't defined then always display OT1: nnn%
      lcd.setCursor( 0, 2 );
      lcd.print("OT1:");
      if( levelOT2 < OT1_CUTOFF ) { // display 0% if OT1 has been cut off
      sprintf( st1, "%4d", (int)0 );     
      }
      else {
        sprintf( st1, "%4d", (int)levelOT1 );
      }
      lcd.print( st1 ); lcd.print("%");
  #endif // end ifdef PID_CONTROL
  #endif // end ifdef ANALOGUE1
  
  #ifdef ANALOGUE2
    lcd.setCursor( 0, 3 );
    lcd.print("OT2:");
    sprintf( st1, "%4d", (int)levelOT2 );
    lcd.print( st1 ); lcd.print("%");
  #endif
  
  #else // if not def LCD_4x20 ie if using a standard 2x16 LCD
  
  #ifdef COMMAND_ECHO
    lcd.print("    "); // overwrite artisan commands
  #endif
  
    // display the first 2 active channels encountered, normally BT and ET
    int it01;
    uint8_t jj,j;
    uint8_t k;
    for( jj = 0, j = 0; jj < NC && j < 2; ++jj ) {
      k = actv[jj];
      if( k != 0 ) {
        ++j;
        it01 = round( convertUnits( T[k-1] ) );
        if( it01 > 999 ) 
          it01 = 999;
        else
          if( it01 < -999 ) it01 = -999;
        sprintf( st1, "%4d", it01 );
        if( j == 1 ) {
          lcd.setCursor( 9, 0 );
          lcd.print("ET:");
        }
        else {
          lcd.setCursor( 9, 1 );
          lcd.print( "BT:" );
        }
        lcd.print(st1);  
      }
    }
  
  #ifdef PID_CONTROL
    if( myPID.GetMode() != MANUAL ) {
      lcd.setCursor( 0, 1 );
      if( levelOT2 < OT1_CUTOFF ) { // display 0% if OT1 has been cut off
        lcd.print( "  0" );
      }
      else {
        sprintf( st1, "%3d", (int)levelOT1 );
        lcd.print( st1 );
      }
      lcd.print("%");
      sprintf( st1, "%4d", (int)Setpoint );
      lcd.print(st1);  
    }
    else {
      lcd.setCursor( 0, 1 );
      lcd.print( "RoR:");
      sprintf( st1, "%4d", (int)RoR[ROR_CHAN] );
      lcd.print( st1 );
    }
  #else
      lcd.setCursor( 0, 1 );
      lcd.print( "RoR:");
      sprintf( st1, "%4d", (int)RoR[ROR_CHAN] );
      lcd.print( st1 );
  #endif // end ifdef PID_CONTROL
  
  #ifdef ANALOGUE1
    if( analogue1_changed == true ) { // overwrite RoR or PID values
      lcd.setCursor( 0, 1 );
      lcd.print("OT1:     ");
      lcd.setCursor( 4, 1 );
      if( levelOT2 < OT1_CUTOFF ) { // display 0% if OT1 has been cut off
        sprintf( st1, "%3d", (int)0 );     
      }
      else {
        sprintf( st1, "%3d", (int)levelOT1 );
      }
      lcd.print( st1 ); lcd.print("%");
    }
  #endif //ifdef ANALOGUE1
  #ifdef ANALOGUE2
    if( analogue2_changed == true ) { // overwrite RoR or PID values
      lcd.setCursor( 0, 1 );
      lcd.print("OT2:     ");
      lcd.setCursor( 4, 1 );
      sprintf( st1, "%3d", (int)levelOT2 );
      lcd.print( st1 ); lcd.print("%");
    }
  #endif // end ifdef ANALOGUE2
  
  #endif // end of ifdef LCD_4x20

  }

  else if( LCD_mode == 1 ) { // Display alternative 1 LCD display

  #ifdef PID_CONTROL
  #ifdef LCD_4x20
    lcd.setCursor( 0, 0 );
    for( int i = 0; i < 20; i++ ) {
      if( profile_name[i] != 0 ) lcd.print( profile_name[i] );
    }
    lcd.setCursor( 0, 1 );
    for( int i = 0; i < 20; i++ ) {
      if( profile_description[i] != 0 ) lcd.print( profile_description[i] );
    }
    lcd.setCursor( 0, 2 );
    for( int i = 20; i < 40; i++ ) {
      if( profile_description[i] != 0 ) lcd.print( profile_description[i] );
    }
    lcd.setCursor( 0, 3 );
    lcd.print("PID: "); lcd.print( myPID.GetKp() ); lcd.print(","); lcd.print( myPID.GetKi() ); lcd.print(","); lcd.print( myPID.GetKd() ); 

  #else // if not def LCD_4x20 ie if using a standard 2x16 LCD
    lcd.setCursor( 0, 0 );
    for( int i = 0; i < 20; i++ ) {
      if( profile_name[i] != 0 ) lcd.print( profile_name[i] );
    }
    lcd.setCursor( 0, 1 );
    lcd.print("P:"); lcd.print( myPID.GetKp() ); lcd.print(","); lcd.print( myPID.GetKi() ); lcd.print(","); lcd.print( myPID.GetKd() ); 
  #endif // end ifdef LCD_4x20
  #endif // end ifdef PID_CONTROL
  }

} // end of updateLCD()
#endif // end ifdef LCD

#if defined ANALOGUE1 || defined ANALOGUE2
// -------------------------------- reads analog value and maps it to 0 to 100
// -------------------------------- rounded to the nearest 5
int32_t getAnalogValue( uint8_t port ) {
  int32_t mod, trial;
  float aval;
  aval = analogRead( port );
  #ifdef ANALOGUE1
    if( port == anlg1 ) {
      aval = MIN_OT1 * 10.24 + ( aval / 1024 ) * 10.24 * ( MAX_OT1 - MIN_OT1 ) ; // scale analogue value to new range
      if ( aval == ( MIN_OT1 * 10.24 ) ) aval = 0; // still allow OT1 to be switched off at minimum value. NOT SURE IF THIS FEATURE IS GOOD???????
      mod = MIN_OT1;
    }
  #endif
  #ifdef ANALOGUE2
    if( port == anlg2 ) {
      aval = MIN_OT2 * 10.24 + ( aval / 1024 ) * 10.24 * ( MAX_OT2 - MIN_OT2 ) ; // scale analogue value to new range
      if ( aval == ( MIN_OT2 * 10.24 ) ) aval = 0; // still allow OT2 to be switched off at minimum value. NOT SURE IF THIS FEATURE IS GOOD???????
      mod = MIN_OT2;
    }
  #endif
  trial = ( aval + 0.001 ) * 100; // to fix weird rounding error from previous calcs?????
  trial /= 1023;
  trial = ( trial / ANALOGUE_STEP ) * ANALOGUE_STEP; // truncate to multiple of ANALOGUE_STEP
  if( trial < mod ) trial = 0;
//  mod = trial % ANALOGUE_STEP;
//  trial = ( trial / ANALOGUE_STEP ) * ANALOGUE_STEP; // truncate to multiple of ANALOGUE_STEP
//  if( mod >= ANALOGUE_STEP / 2 )
//    trial += ANALOGUE_STEP;
  return trial;
}
#endif // end if defined ANALOGUE1 || defined ANALOGUE2

#ifdef ANALOGUE1
// ---------------------------------
void readAnlg1() { // read analog port 1 and adjust OT1 output
  char pstr[5];
  int32_t reading;
  reading = getAnalogValue( anlg1 );
  if( reading <= 100 && reading != old_reading_anlg1 ) { // did it change?
    analogue1_changed = true;
    levelOT1 = reading;
    old_reading_anlg1 = reading; // save reading for next time
    output_level_icc( levelOT1 );  // integral cycle control and zero cross SSR on OT1
  }
  else {
    analogue1_changed = false;
  }
}
#endif // end ifdef ANALOGUE1

#ifdef ANALOGUE2
// ---------------------------------
void readAnlg2() { // read analog port 2 and adjust OT2 output
  char pstr[5];
  int32_t reading;
  reading = getAnalogValue( anlg2 );
  if( reading <= 100 && reading != old_reading_anlg2 ) { // did it change?
    analogue2_changed = true;
    levelOT2 = reading;
    old_reading_anlg2 = reading; // save reading for next time
    output_level_pac( levelOT2 );
  }
  else {
    analogue2_changed = false;
  }
}
#endif // end ifdef ANALOGUE2


#ifdef PID_CONTROL
// ---------------------------------
void updateSetpoint() { //read profile data from EEPROM and calculate new setpoint

  while( counter < times[0] || counter >= times[1] ) { // if current time outside currently loaded interval then adjust profile pointer before reading new interval data from EEPROM
    if( counter < times[0] ) {
      profile_ptr = profile_ptr - 2; // two bytes per int
    }
    else {
      profile_ptr = profile_ptr + 2; // two bytes per int
    }

    eeprom.read( profile_ptr, (uint8_t*)&times, sizeof(times) ); // read two profile times
    eeprom.read( profile_ptr + 100, (uint8_t*)&temps, sizeof(temps) ); // read two profile temps.  100 = size of time data
    
    if( times[1] == 0 ) {
      Setpoint = 0;
      myPID.SetMode(MANUAL); // deactivate PID control
      Output = 0; // set PID output to 0
      break;
    }
  }
  
  float x = (float)( counter - times[0] ) / (float)( times[1] - times[0] ); // can probably be tidied up?? Calcs proportion of time through current profile interval
  Setpoint = temps[0] + x * ( temps[1] - temps[0] );  // then applies the proportion to the temps
  if( profile_CorF == 'F' && Cscale ) { // make setpoint units match current units
    Setpoint = convertUnits( Setpoint ); // convert F to C
  }
  else if( profile_CorF == 'C' & !Cscale) { // make setpoint units match current units
    Setpoint = Setpoint * 9 / 5 + 32; // convert C to F
  }
}


void setProfile() { // set profile pointer and read initial profile data

  profile_ptr = 1024 + ( 400 * ( profile_number - 1 ) ) + 4; // 1024 = start of profile storage in EEPROM. 400 = size of each profile. 4 = location of profile C or F data
  eeprom.read( profile_ptr, (uint8_t*)&profile_CorF, sizeof(profile_CorF) ); // read profile temp type
  
  getProfileDescription(profile_number); // read profile name and description data from eeprom for active profile number
  
  profile_ptr = 1024 + ( 400 * ( profile_number - 1 ) ) + 125; // 1024 = start of profile storage in EEPROM. 400 = size of each profile. 125 = size of profile header data
  eeprom.read( profile_ptr, (uint8_t*)&times, sizeof(times) ); // read 1st two profile times
  eeprom.read( profile_ptr + 100, (uint8_t*)&temps, sizeof(temps) ); // read 1st two profile temps.  100 = size of time data
  
  // profile_ptr is left set for profile temp/time reads
  
}

void getProfileDescription(int pn) { // read profile name and description data from eeprom
  
  int pp = 1024 + ( 400 * ( pn - 1 ) ) + 5; // 1024 = start of profile storage in EEPROM. 400 = size of each profile. 5 = location of profile name
  eeprom.read( pp, (uint8_t*)&profile_name, sizeof(profile_name) ); // read profile name  

  pp = 1024 + ( 400 * ( pn - 1 ) ) + 45; // 1024 = start of profile storage in EEPROM. 400 = size of each profile. 45 = location of profile description
  eeprom.read( pp, (uint8_t*)&profile_description, sizeof(profile_description) ); // read profile name  
  
}
#endif // end ifdef PID_CONTROL

#ifdef LCDAPTER
// ----------------------------------
void checkButtons() { // take action if a button is pressed
  if( buttons.readButtons() ) {
    
    switch (LCD_mode) {
    
      case 0: // Main LCD display
      
        if( buttons.keyPressed( 0 ) && buttons.keyChanged( 0 ) ) { // button 1 - PID on/off - PREVIOUS PROFILE
          #ifdef PID_CONTROL
          if( myPID.GetMode() == MANUAL ) {
            myPID.SetMode( AUTOMATIC );
          }
          else {
            myPID.SetMode( MANUAL );
          }
          #endif
        }
        else if( buttons.keyPressed( 1 ) && buttons.keyChanged( 1 ) ) { // button 2 - RESET TIMER - NEXT PROFILE
          counter = 0;
        }   
        else if( buttons.keyPressed( 2 ) && buttons.keyChanged( 2 ) ) { // button 3 - ENTER BUTTON
          // do something
        }
        else if( buttons.keyPressed( 3 ) && buttons.keyChanged( 3 ) ) { // button 4 - CHANGE LCD MODE
          lcd.clear();
          LCD_mode++; // change mode
          #ifndef PID_CONTROL
          if( LCD_mode == 1 ) LCD_mode++; // deactivate LCD mode 1 if PID control is disabled
          #endif
          if( LCD_mode > 1 ) LCD_mode = 0; // loop at limit of modes
          delay(5);
        }
        break;
        
      case 1: // Profile Selection and PID parameter LCD display
           
        if( buttons.keyPressed( 0 ) && buttons.keyChanged( 0 ) ) { // button 1 - PID on/off - PREVIOUS PROFILE
          #ifdef PID_CONTROL
          profile_number_new--;
          if( profile_number_new == 0 ) profile_number_new = NUM_PROFILES; // loop profile_number to end
          getProfileDescription(profile_number_new);
          #endif
        }
        else if( buttons.keyPressed( 1 ) && buttons.keyChanged( 1 ) ) { // button 2 - RESET TIMER - NEXT PROFILE
          #ifdef PID_CONTROL
          profile_number_new++;
          if( profile_number_new > NUM_PROFILES ) profile_number_new = 1; // loop profile_number to start
          getProfileDescription(profile_number_new);
          #endif
        }   
        else if( buttons.keyPressed( 2 ) && buttons.keyChanged( 2 ) ) { // button 3 - ENTER BUTTON
          #ifdef PID_CONTROL
          profile_number = profile_number_new; // change profile_number to new selection
          setProfile(); // call setProfile to load the profile selected
          lcd.clear();
          LCD_mode = 0; // jump back to main LCD display mode
          #endif
        }
        else if( buttons.keyPressed( 3 ) && buttons.keyChanged( 3 ) ) { // button 4 - CHANGE LCD MODE
          lcd.clear();
          #ifdef PID_CONTROL
          profile_number_new = profile_number; // reset profile_number_new if profile wasn't changed
          setProfile(); // or getProfileDescription()?????????
          #endif
          LCD_mode++; // change mode
          if( LCD_mode > 1 ) LCD_mode = 0; // loop at limit of modes
          delay(5);
        }
        break;
    } //end of switch
  } // end of if( buttons.readButtons() )
} // end of void checkButtons()
#endif // end ifdef LCDAPTER


// ------------------------------------------------------------------------
// MAIN
//
void setup()
{
  delay(100);
  Wire.begin(); 
  Serial.begin(BAUD);
  amb.init( AMB_FILTER );  // initialize ambient temp filtering

#ifdef LCD
#ifdef LCD_4x20
  lcd.begin(20, 4);
#else
  lcd.begin(16, 2);
#endif // LCD_4x20
  BACKLIGHT;
  lcd.setCursor( 0, 0 );
  lcd.print( BANNER_ARTISAN ); // display version banner
#endif // LCD

#ifdef LCDAPTER
  buttons.begin( 4 );
  buttons.readButtons();
  buttons.ledAllOff();
#endif

#ifdef MEMORY_CHK
  Serial.print("# freeMemory()=");
  Serial.println(freeMemory());
#endif

  adc.setCal( CAL_GAIN, UV_OFFSET );
  amb.setOffset( AMB_OFFSET );

  // read calibration and identification data from eeprom
  if( readCalBlock( eeprom, caldata ) ) {
    adc.setCal( caldata.cal_gain, caldata.cal_offset );
    amb.setOffset( caldata.K_offset );
  }
  else { // if there was a problem with EEPROM read, then use default values
    adc.setCal( CAL_GAIN, UV_OFFSET );
    amb.setOffset( AMB_OFFSET );
  }   

  // initialize filters on all channels
  fT[0].init( ET_FILTER ); // digital filtering on ET
  fT[1].init( BT_FILTER ); // digital filtering on BT
  fT[2].init( ET_FILTER);
  fT[3].init( ET_FILTER);
  fRise[0].init( RISE_FILTER ); // digital filtering for RoR calculation
  fRise[1].init( RISE_FILTER ); // digital filtering for RoR calculation
  fRoR[0].init( ROR_FILTER ); // post-filtering on RoR values
  fRoR[1].init( ROR_FILTER ); // post-filtering on RoR values
  
  // set up output on OT1 and OT2
//  ssr.Setup( TIME_BASE );
  levelOT1 = levelOT2 = 0;
  init_control();

  #ifdef ANALOGUE1
  old_reading_anlg1 = getAnalogValue( anlg1 ); // initialize old_reading with initial analogue value
  #endif
  #ifdef ANALOGUE2
  old_reading_anlg2 = getAnalogValue( anlg2 ); // initialize old_reading with initial analogue value
  #endif  
  
  // initialize the active channels to default values
  actv[0] = 2; // ET on TC1
  actv[1] = 1; // BT on TC2
  actv[2] = 0; // default inactive
  actv[3] = 0; // default inactive
  
  // assign thermocouple types
  tcp[0] = &tc1;
  tcp[1] = &tc2;
  tcp[2] = &tc3;
  tcp[3] = &tc4;

// add active commands to the linked list in the command interpreter object
  ci.addCommand( &dwriter );
  ci.addCommand( &awriter );
  ci.addCommand( &units );
  ci.addCommand( &chan );
  ci.addCommand( &io3 );
  ci.addCommand( &ot2 );
  ci.addCommand( &ot1 );
  ci.addCommand( &rf2000 );
  ci.addCommand( &rc2000 );
  ci.addCommand( &reader );
  ci.addCommand( &pid );
  ci.addCommand( &reset );
  ci.addCommand( &load );
  ci.addCommand( &power );
  ci.addCommand( &fan );
  
  pinMode( LED_PIN, OUTPUT );

#ifdef LCD
  delay( 500 );
  lcd.clear();
#endif
#ifdef MEMORY_CHK
  checktime = millis();
#endif

#ifdef PID_CONTROL
  myPID.SetSampleTime(1000); // set sample time to 1 second
  myPID.SetOutputLimits(MIN_OT1, MAX_OT1); // set output limits to user defined limits
  myPID.SetControllerDirection(DIRECT); // set PID to be direct acting mode. Increase in output leads to increase in input
  myPID.SetTunings(PRO, INT, DER); // set initial PID tuning values
  myPID.SetMode(MANUAL); // start with PID control off
  profile_number = 1; // set default profile
  profile_number_new = profile_number; 
  setProfile(); // read profile description initial time/temp data from eeprom and set profile_pointer
#endif

first = true;
counter = 3; // start counter at 3 to match with Artisan. Probably a better way to sync with Artisan???
next_loop_time = millis() + 1000; // needed?? 

}


// -----------------------------------------------------------------
void loop()
{
  if( ACdetect() ) {
    digitalWrite( LED_PIN, HIGH );
  }  
  else {
    digitalWrite( LED_PIN, LOW );
  }
  #ifdef MEMORY_CHK  
    uint32_t now = millis();
    if( now - checktime > 1000 ) {
      Serial.print("# freeMemory()=");
      Serial.println(freeMemory());
      checktime = now;
    }
  #endif
  checkSerial();  // Has a command been received?
  get_samples();
  #ifdef ANALOGUE1
    #ifdef PID_CONTROL
      if( myPID.GetMode() == MANUAL ) readAnlg1(); // if PID is off allow ANLG1 read
    #else
      readAnlg1(); // if PID_CONTROL is defined always allow ANLG1 read
    #endif // PID_CONTROL
  #endif // ANALOGUE1
  #ifdef ANALOGUE2
    readAnlg2();
  #endif
  #ifdef PID_CONTROL
    if( myPID.GetMode() != MANUAL ) { // If PID in AUTOMATIC mode calc new output and assign to OT1
      Input = convertUnits( T[PID_CHAN] ); // using temp from this TC4 channel as PID input. use actv[?] instead of 0??
      updateSetpoint(); // read profile data from EEPROM and calculate new setpoint
      myPID.Compute();  // do PID calcs
      levelOT1 = Output; // update OT1 based on PID optput
      output_level_icc( levelOT1 );  // integral cycle control and zero cross SSR on OT1
    }
  #endif
  #ifdef LCD
    updateLCD();
  #endif
  #if defined ROASTLOGGER
    logger(); // send data every second to ANDROID
  #endif
  #if defined ARTISAN || defined ANDROID
    if( artisan_logger == true ) {
      artisan_logger = false;
      logger();
    }
  #endif
//  Serial.println( next_loop_time - millis() ); // how much time spare in loop. approx 350ms
  while( millis() < next_loop_time ) {
  #ifdef LCDAPTER
    checkButtons();
  #endif
  }
  next_loop_time = next_loop_time + 1000; // add 1 second until next loop
  counter++; if( counter > 3599 ) counter = 3599;
}

