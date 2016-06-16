/*
 * This script triggers microfluidic valve controllers based on
 * inputs from the Serial port (virtual serial port via USB).
 * 
 * To maximise utility, the ouput to the valve drivers can be set
 * to ACTIVE HIGH or ACTIVE LOW depending on the circuitry involved.
 * The script will be programmed for all 8 valves, but not all need
 * to be used (also some of the 'valves' can be used as triggers
 * to control other activties such as function generators or lasers).
 * 
 * There are also serial command to alter delays and auto-off times.
 * These delays are global: TODO make them indepndent for each valve.
 * 
 * The IO connections are as follows:
 * Pin# -- function --- comments
 *  0   -- valve 1 --- controls the valve numbered '1'
 *  1   -- valve 2 --- controls the valve numbered '2'
 *  2   -- valve 3 --- controls the valve numbered '3'
 *  3   -- valve 4 --- controls the valve numbered '4'
 *  4   -- valve 5 --- controls the valve numbered '5'
 *  5   -- valve 6 --- controls the valve numbered '6'
 *  6   -- valve 7 --- controls the valve numbered '7'
 *  7   -- valve 8 --- controls the valve numbered '8'
 *  
 */
// ----------- includes --------------------------
//none

// ------------ definitions --------------
#define ACTIVE_LOW 0 //change active high/low function (1 = low, 0 = high)
#define _valve1pin 0 //change the pin as the hardware requires
#define _valve2pin 1 //change the pin as the hardware requires
#define _valve3pin 2 //change the pin as the hardware requires
#define _valve4pin 3 //change the pin as the hardware requires
#define _valve5pin 4 //change the pin as the hardware requires
#define _valve6pin 5 //change the pin as the hardware requires
#define _valve7pin 6 //change the pin as the hardware requires
#define _valve8pin 7 //change the pin as the hardware requires

// ------------ macros --------------------------
#if (ACTIVE_LOW == 1) //if active low is selected
  //write macros that set = LOW and clear = HIGH
  #define _setValve1 (digitalWriteFast(_valve1pin,LOW)) //valve 1 set
  #define _clearValve1 (digitalWriteFast(_valve1pin,HIGH)) //valve 1 clear
  #define _setValve2 (digitalWriteFast(_valve2pin,LOW)) //valve 2 set
  #define _clearValve2 (digitalWriteFast(_valve2pin,HIGH)) //valve 2 clear   
  #define _setValve3 (digitalWriteFast(_valve3pin,LOW)) //valve 3 set
  #define _clearValve3 (digitalWriteFast(_valve3pin,HIGH)) //valve 3 clear
  #define _setValve4 (digitalWriteFast(_valve4pin,LOW)) //valve 4 set
  #define _clearValve4 (digitalWriteFast(_valve4pin,HIGH)) //valve 4 clear
  #define _setValve5 (digitalWriteFast(_valve5pin,LOW)) //valve 5 set
  #define _clearValve5 (digitalWriteFast(_valve5pin,HIGH)) //valve 5 clear
  #define _setValve6 (digitalWriteFast(_valve6pin,LOW)) //valve 6 set
  #define _clearValve6 (digitalWriteFast(_valve6pin,HIGH)) //valve 6 clear
  #define _setValve7 (digitalWriteFast(_valve7pin,LOW)) //valve 7 set
  #define _clearValve7 (digitalWriteFast(_valve7pin,HIGH)) //valve 7 clear
  #define _setValve8 (digitalWriteFast(_valve8pin,LOW)) //valve 8 set
  #define _clearValve8 (digitalWriteFast(_valve8pin,HIGH)) //valve 8 clear
#else
  //write macros that set = HIGH and clear = LOW
  #define _setValve1 (digitalWriteFast(_valve1pin,HIGH)) //valve 1 set
  #define _clearValve1 (digitalWriteFast(_valve1pin,LOW)) //valve 1 clear
  #define _setValve2 (digitalWriteFast(_valve2pin,HIGH)) //valve 2 set
  #define _clearValve2 (digitalWriteFast(_valve2pin,LOW)) //valve 2 clear   
  #define _setValve3 (digitalWriteFast(_valve3pin,HIGH)) //valve 3 set
  #define _clearValve3 (digitalWriteFast(_valve3pin,LOW)) //valve 3 clear
  #define _setValve4 (digitalWriteFast(_valve4pin,HIGH)) //valve 4 set
  #define _clearValve4 (digitalWriteFast(_valve4pin,LOW)) //valve 4 clear
  #define _setValve5 (digitalWriteFast(_valve5pin,HIGH)) //valve 5 set
  #define _clearValve5 (digitalWriteFast(_valve5pin,LOW)) //valve 5 clear
  #define _setValve6 (digitalWriteFast(_valve6pin,HIGH)) //valve 6 set
  #define _clearValve6 (digitalWriteFast(_valve6pin,LOW)) //valve 6 clear
  #define _setValve7 (digitalWriteFast(_valve7pin,HIGH)) //valve 7 set
  #define _clearValve7 (digitalWriteFast(_valve7pin,LOW)) //valve 7 clear
  #define _setValve8 (digitalWriteFast(_valve8pin,HIGH)) //valve 8 set
  #define _clearValve8 (digitalWriteFast(_valve8pin,LOW)) //valve 8 clear
#endif

//--------- variable definitions ----------
IntervalTimer _valveOnDelayTimer; //timer object an optional valve delay
IntervalTimer _valveAutoOffTimer; //timer object for automatically clearing the valve state
int _delayTime = 0; // delay in microseconds
int _offTime = 0; //auto off time in microseconds. 0 = disabled.
byte _delayValves = 0; //if the valves have a delay, store it here

//-------- Setup functions --------------
void setup() {
  //set the valve pin modes to output
  pinMode(_valve1pin,OUTPUT); //valve 1 pin as an output
  pinMode(_valve2pin,OUTPUT); //valve 2 pin as an output
  pinMode(_valve3pin,OUTPUT); //valve 3 pin as an output
  pinMode(_valve4pin,OUTPUT); //valve 4 pin as an output
  pinMode(_valve5pin,OUTPUT); //valve 5 pin as an output
  pinMode(_valve6pin,OUTPUT); //valve 6 pin as an output
  pinMode(_valve7pin,OUTPUT); //valve 7 pin as an output
  pinMode(_valve8pin,OUTPUT); //valve 8 pin as an output

  // setup the serial port
  Serial.begin(9600); //serial port used for sending debug messages only
  Serial.println("Initalising..."); //send a generic startup message to the serial port

}

//------- main loop functions ----------------
void loop() {
  serviceSerial(); //checks the serial port
}

/* 
 *  USB Serial port inputs are serviced in this routine
 *  
 *  The serial expects commands in the form of:
 *                  245a
 *  where the letter at the end represents the command character
 *  
 *  Addition: can now send up to 5 values to the teensy using CSV data. EG:
 *              24443,556,3d
 *  This command sent to the PC will save values 24443, 556 and 3 to a numberic array 'val'
 *  previously, val was just a single static int, now it is an int array.
 *  
 *  There are three main command characters:
 *  a : actiaves the valves using the entered number (number converted to binary)
 *  d : sets the valve delay time
 *  o : sets the valve auto off time
 */
void serviceSerial(void)
{
  static long val[5] = {0}; //variable to store the input character
  static byte valNum = 0; //default is val ZERO
  if ( Serial.available()){ //if there is data avaliable
    char ch = Serial.read(); //read the character
    if(ch >= '0' && ch <= '9') // is it a number?
    {
      val[valNum] = val[valNum] * 10 + ch - '0'; // yes, accumulate the value
    }
    else if(ch == ','){ //if it is a comma, increment the valNum variable
      //just check first that the valNum will not be an illegal value!
      if (valNum < 4){
        valNum ++; //increment it
      }
    }
    else if(ch == 'a'){ //activate the valves using the given number
      if (val[0] > 255){ //check to see if the input number is for a valid number of valves (8 valves = 8 bit = 255)
        Serial.println("Invalid valve combination"); //if the number is invalid, alert the user
        val[5] = {0}; // reset the serial number accumulator
        valNum = 0;
      }
      else{ //the value is okay, activate the valves
        //check if a delay is needed
        if (_delayTime == 0){ //no delay needed
          activateValves((byte)val[0]); //activate the valves
        }
        else{
          //start the valves using a delay (global delay for now)
          _delayValves = (byte)val[0]; //store the valve states in a buffer
          _valveOnDelayTimer.begin(_delayActivateValves, _delayTime); //actiave the valves delay timer
        }
        //now determine if the valves will be turned off automatically
        if (_offTime != 0){
          //valves will be turned off automatically
          _valveAutoOffTimer.begin(_allValvesOff, _offTime); //start the valve off timer
        }
        val[5] = {0}; // reset the serial number accumulator
        valNum = 0;
      }
    }
    else if(ch == 'd'){ //delay time adjustment
      if (val[0] < 10000000){ //arbitrary upper limit to the delay is 10 seconds
        _delayTime = (int)val[0];
        Serial.print("Set a delay time of ");
        Serial.print(_delayTime);
        Serial.println(" microseconds");
      }
      else{
        Serial.println("Delay time too long");
      }
      val[5] = {0}; // reset the serial number accumulator
      valNum = 0;
    }
    else if(ch == 'o'){ //change the auto off time
      if (val[0] < 10000000){ //arbitrary upper limit to the delay is 10 seconds
        _offTime = (int)val[0];
        Serial.print("Set an auto OFF time of ");
        Serial.print(_offTime);
        Serial.println(" microseconds");
      }
      else{
        Serial.println("Auto OFF time too long");
      }
      val[5] = {0}; // reset the serial number accumulator
      valNum = 0;
    }
    else if(ch == 'f'){
      //two value example
      unsigned long FrameTime = (long)val[0];
      byte ValveState = (int)val[1];
      val[5] = {0}; // reset the serial number accumulator
      Serial.print("Frame time: ");
      Serial.println(FrameTime);
      Serial.print("Valve state:");
      Serial.println(ValveState);
    }
    else{
      Serial.println("Unknown command...");
      Serial.println("Resetting the accumulator.");
      val[5] = {0}; // reset the serial number accumulator
      valNum = 0;
    }
  }
}

/* 
 *  Function to activate the valves based on the give number.
 *  The number must be a byte (ie 8 -bits) where each bit represents a valve.
 *  The valve number VS bit number is as follows:
 *  VALVE # -- BIT #
 *    1     --  0  (LSB)
 *    2     --  1  
 *    3     --  2  
 *    4     --  3  
 *    5     --  4   
 *    6     --  5  
 *    7     --  6  
 *    8     --  7  (MSB)
 *  
 *  For example, running "activateValves(37)" will give the bit pattern 10100100 (LSB -> MSB).
 *  Valve 1 = on
 *  Valve 2 = off
 *  Valve 3 = on
 *  Valve 4 = off
 *  Valve 5 = off
 *  Valve 6 = on
 *  Valve 7 = off
 *  Valve 8 = off
 */
void activateValves(byte _valves){
  // valve 1 = bit 0
  if ((_valves & (1 << 0)) != 0){ //if this is true the valve should be activated
    _setValve1; //activate the valve
  }
  else{
    _clearValve1; //deactivate the valve
  }
  // valve 2 = bit 1
  if ((_valves & (1 << 1)) != 0){ //if this is true the valve should be activated
    _setValve2; //activate the valve
  }
  else{
    _clearValve2; //deactivate the valve
  }
  // valve 3 = bit 2
  if ((_valves & (1 << 2)) != 0){ //if this is true the valve should be activated
    _setValve3; //activate the valve
  }
  else{
    _clearValve3; //deactivate the valve
  }
  // valve 4 = bit 3
  if ((_valves & (1 << 3)) != 0){ //if this is true the valve should be activated
    _setValve4; //activate the valve
  }
  else{
    _clearValve4; //deactivate the valve
  }
  // valve 5 = bit 4
  if ((_valves & (1 << 4)) != 0){ //if this is true the valve should be activated
    _setValve5; //activate the valve
  }
  else{
    _clearValve5; //deactivate the valve
  }
  // valve 6 = bit 5
  if ((_valves & (1 << 5)) != 0){ //if this is true the valve should be activated
    _setValve6; //activate the valve
  }
  else{
    _clearValve6; //deactivate the valve
  }
  // valve 7 = bit 6
  if ((_valves & (1 << 6)) != 0){ //if this is true the valve should be activated
    _setValve7; //activate the valve
  }
  else{
    _clearValve7; //deactivate the valve
  }
  // valve 8 = bit 7
  if ((_valves & (1 << 7)) != 0){ //if this is true the valve should be activated
    _setValve8; //activate the valve
  }
  else{
    _clearValve8; //deactivate the valve
  }
}

// function to automatically switch the vavles ON after a delay time
void _delayActivateValves(void){
  _valveOnDelayTimer.end(); //stop the timer (single shot timer)
  activateValves(_delayValves); //actiave the valves using the saved pattern
}

//function to automatically switch the valves OFF
void _allValvesOff(void){
  _valveAutoOffTimer.end(); //stop the timer (single shot timer)
  activateValves(0); //turn off all valves
}

