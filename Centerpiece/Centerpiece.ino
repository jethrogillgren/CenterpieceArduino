
/* @file HelloKeypad.pde
  || @version 1.0
  || @author Alexander Brevig
  || @contact alexanderbrevig@gmail.com
  ||
  || @description
  || | Demonstrates the simplest use of the matrix Keypad library.
  || #
*/
#include <Printers.h>
#include <Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include <TimerFreeTone.h>

//KEYPAD
//Connect Keypad pins 1-7 to arduino pins 2-8
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// First Version
//byte rowPins[ROWS] = {5, 7, 8, 3}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {6, 2, 4}; //connect to the column pinouts of the keypad

//Combined with PWM LED version
//byte rowPins[ROWS] = {17, 19, 2, 15}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {18, 14, 16}; //connect to the column pinouts of the keypad
byte rowPins[ROWS] = {A3, A5, 4, A1}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A4, A0, A2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


//The brightness for a half-active LED Button, and the pre-game LED value
const int halfLitBrightness = 100;

//Button LEDs - These (PWM) pins should connect to a MOSFET which controlls the Button LED, as it is 12V.
int woodLED = 3;
int metalLED = 5;
int fireLED = 6;
int waterLED = 9;
int earthLED = 10;

int woodPress = A6;//Analog
int metalPress = A7;//Analog
int firePress = 8;//Digital
int waterPress = 11;//Digital
int earthPress = 12;//Digital

//Active brightness - will get tweened up to 255, once set to >0
int woodBrightness = 0;
int metalBrightness = 0;
int fireBrightness = 0;
int waterBrightness = 0;
int earthBrightness = 0;

//The brightness to aniamte the Buttons towards
int woodTarget = 0;
int metalTarget = 0;
int fireTarget = 0;
int waterTarget = 0;
int earthTarget = 0;

int fadeAmount = 5; //Speed of fade
int fadeDelay = 20; //speed of fade

/*//Strip LEDs - These are WS2811 LEDs powered by 12V - used for the Activation lines between KoalaPos

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
int creatorNumLEDs = 10;
Adafruit_NeoPixel lhsCreatorPixels = Adafruit_NeoPixel(creatorNumLEDs, 4,
NEO_GRB + NEO_KHZ800);//Ring around the LHS Creator KoalaPos
Adafruit_NeoPixel rhsCreatorPixels = Adafruit_NeoPixel(creatorNumLEDs, 7,
NEO_GRB + NEO_KHZ800);//Ring around the RHS Creator KoalaPos

int creatorToCombinorNumLEDs = 10;
Adafruit_NeoPixel lhsCreatorToCombinorPixels
= Adafruit_NeoPixel(creatorToCombinorNumLEDs, 8, NEO_GRB + NEO_KHZ800);//Creator->Combinor lhs
Adafruit_NeoPixel rhsCreatorToCombinorPixels
= Adafruit_NeoPixel(creatorToCombinorNumLEDs, 12, NEO_GRB + NEO_KHZ800);//Creator->Combinor rhs
*/

//// XBEE & COMMUNICATIONS
SoftwareSerial xbeeSerial(6, 7); // RX, TX

//Works with Series1 and 2
XBeeWithCallbacks xbee;

#define MSG_WOOD_PRESSED 'w'
#define MSG_METAL_PRESSED 'm'
#define MSG_FIRE_PRESSED 'f'
#define MSG_WATER_PRESSED 'W'
#define MSG_EARTH_PRESSED 'e'

#define MSG_RESET   'r'

// Build a reuseable message packet to send to the Co-Ordinator
XBeeAddress64 coordinatorAddr = XBeeAddress64(0x00000000, 0x00000000);

//uint8_t pressMessagePayload[4] = {0};
//ZBTxRequest pressMessage = ZBTxRequest(coordinatorAddr, pressMessagePayload, sizeof(pressMessagePayload));

uint8_t placeMessagePayload[4] = {0};
ZBTxRequest placeMessage = ZBTxRequest(coordinatorAddr, placeMessagePayload, sizeof(placeMessagePayload));


//// SOUND
 
//#include <Tone.h> <-- This collided with using PWM Timer on Pins 3 and 11
//Tone notePlayer;

#define TONE_PIN 13
const int keyDelay = 200;

//Major C chord starting from A4
int notes[] = { 440,
                493,
                523,
                587,
                659,
                698,
                783,
                880,
                987,
                1046
              };



void setup() {
  Serial.begin(9600);
  
  //notePlayer.begin(buzzerPin);

  //pinMode(woodPress, INPUT); <-- Analogue pins do not use pinMode to initialize
  //digitalWrite(woodPress, HIGH); 
  //pinMode(metalPress, INPUT);
  //digitalWrite(metalPress, HIGH);
  pinMode(firePress, INPUT);// initialize the pushbutton pin as an input:
  digitalWrite(firePress, HIGH);// connect internal pull-up so it is not floating
  pinMode(waterPress, INPUT);
  digitalWrite(waterPress, HIGH);
  pinMode(earthPress, INPUT);
  digitalWrite(earthPress, HIGH);
  
  pinMode(woodLED, OUTPUT);
  pinMode(metalLED, OUTPUT);
  pinMode(fireLED, OUTPUT);
  pinMode(waterLED, OUTPUT);
  pinMode(earthLED, OUTPUT);

  /*//Initialize the NeoPixel library.
  lhsCreatorPixels.begin();
  rhsCreatorPixels.begin();
  lhsCreatorToCombinorPixels.begin();
  rhsCreatorToCombinorPixels.begin();*/

  // WHen resetting the room, we want to see that the lights are working.
  //They will go unlit when the Game starts and send a Rest Command.
  //SetAllLEDs(255);
  SetAllLEDs(255);
  PlayGoodSound();
  SetAllLEDs(0);

  
  // XBEE
  xbeeSerial.begin(9600);
  xbee.setSerial(xbeeSerial);

  // Make sure that any errors are logged to Serial. The address of
  // Serial is first cast to Print*, since that's what the callback
  // expects, and then to uintptr_t to fit it inside the data parameter.
  xbee.onPacketError(printErrorCb, (uintptr_t)(Print*)&Serial);
  xbee.onTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&Serial);
  xbee.onZBTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&Serial);

  // These are called when an actual packet received
  xbee.onZBRxResponse(zbReceive, (uintptr_t)(Print*)&Serial);

  // Print any unhandled response with proper formatting
  xbee.onOtherResponse(printResponseCb, (uintptr_t)(Print*)&Serial);

  // Enable this to print the raw bytes for _all_ responses before they
  // are handled
  xbee.onResponse(printRawResponseCb, (uintptr_t)(Print*)&Serial);


  Serial.println("SETUP OK");
  
}

void loop() {


  //// SOUND
  //notePlayer.stop();


  //// NETWORK COMMANDS
  // Continuously let xbee read packets and call callbacks.
  xbee.loop();


  SendPressedMessage('a');
  delay(100);

  ////LED Buttons
  //Serial.println( "Wood: " + digitalRead(woodPress) );
  /*if ( analogRead(woodPress) > 200 )
  {
    Serial.println("Wood Pressed");
    SendPressedMessage(MSG_WOOD_PRESSED);
  }
  if ( analogRead(metalPress) > 200 )
  {
    Serial.println("Metal Pressed");
    SendPressedMessage(MSG_METAL_PRESSED);
  }
  if ( digitalRead(firePress) == HIGH )
  {
    Serial.println("Fire Pressed");
    SendPressedMessage(MSG_FIRE_PRESSED);
  }
  if ( digitalRead(waterPress) == HIGH )
  {
    Serial.println("Water Pressed");
    SendPressedMessage(MSG_WATER_PRESSED);
  }
  if ( digitalRead(earthPress) == HIGH )
  {
    Serial.println("Earth Pressed");
    SendPressedMessage(MSG_EARTH_PRESSED);
  }*/

  //// LEDs
  if( woodBrightness < woodTarget )
  {
    woodBrightness+=fadeAmount;
    analogWrite(woodLED, woodBrightness);
    delay( fadeDelay);
  }
  
  /* <-- Debug Fade code
  analogWrite(woodLED, woodBrightness);
  woodBrightness = woodBrightness + fadeAmount;
  if (woodBrightness <= 0 || woodBrightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  delay(30);
  */
  
  if( metalBrightness < metalTarget )
  {
    metalBrightness+=fadeAmount;
    analogWrite(metalLED, metalBrightness);
    delay( fadeDelay);
  }
  if( fireBrightness < fireTarget )
  {
    fireBrightness+=fadeAmount;
    analogWrite(fireLED, fireBrightness);
    delay( fadeDelay);
  }
  if( waterBrightness < waterTarget)
  {
    waterBrightness+=fadeAmount;
    analogWrite(waterLED, waterBrightness);
    delay( fadeDelay);
  }
  if( earthBrightness < earthTarget )
  {
    earthBrightness+=fadeAmount;
    analogWrite(earthLED, earthBrightness);
    delay( fadeDelay);
  }


  //KEYPAD PRESSES
  char key = keypad.getKey();
  if (key) {

    Serial.println(key + " pressed");
    SendPressedMessage( key ); //This goes to the Game Server
    
    //SOUND
    switch (key)
    {
      case '1':
        //notePlayer.play(notes[0]);
        //delay(keyDelay);
        TimerFreeTone(TONE_PIN, notes[1], keyDelay);
        woodBrightness=1;
        break;
      case '2':
        TimerFreeTone(TONE_PIN, notes[2], keyDelay);
        metalBrightness=1;
        break;
      case '3':
        TimerFreeTone(TONE_PIN, notes[3], keyDelay);
        fireBrightness=1;
        break;
      case '4':
        TimerFreeTone(TONE_PIN, notes[4], keyDelay);
        waterBrightness =1;
        break;
      case '5':
        TimerFreeTone(TONE_PIN, notes[5], keyDelay);
        earthBrightness=1;
        break;
      case '6':
        TimerFreeTone(TONE_PIN, notes[6], keyDelay);
        break;
      case '7':
        TimerFreeTone(TONE_PIN, notes[7], keyDelay);
        break;
      case '8':
        TimerFreeTone(TONE_PIN, notes[8], keyDelay);
        break;
      case '9':
        TimerFreeTone(TONE_PIN, notes[9], keyDelay);
        break;
      case '0':
        TimerFreeTone(TONE_PIN, notes[0], keyDelay);
        break;

      case '*':
        PlayBadSound();
        break;

      case '#':
        PlayGoodSound();
        break;

      default:
        break;
    }

  }

  //TODO
  // Read any keypresses on the 5 RGB LEDs
  // and send to server using same CommandCodes Serial.print("G"); //This goes to the Game Server

} //End loop




//// XBEE / COMMUNICATION FUNCTIONS
//FrameType:  0x90  recieved.
void zbReceive(ZBRxResponse& rx, uintptr_t data) {

  if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED
      || rx.getOption() == ZB_BROADCAST_PACKET ) {

      //Debug it out - copied from the lib
      Print *p = (Print*)data;
      if (!p) {
        Serial.println("ERROR 2");
        //flashSingleLed(LED_BUILTIN, 2, 500);
        return;
      }
      p->println(F("Recieved:"));
        p->print("  Payload: ");
        printHex(*p, rx.getFrameData() + rx.getDataOffset(), rx.getDataLength(), F(" "), F("\r\n    "), 8);
      p->println();
        p->print("  From: ");
        printHex(*p, rx.getRemoteAddress64() );
      p->println();

      //This Project only ever takes a 1 char command
      //printHex(rx.getData()[0], 2);
      parseCommand( (char) rx.getData()[0] );
      
      //flashSingleLed(LED_BUILTIN, 5, 50);
      
  } else {
      // we got it (obviously) but sender didn't get an ACK
      Serial.println("ERROR 1");
      //flashSingleLed(LED_BUILTIN, 1, 500);
  }
}




int badMelody[] =      {262, 196, 196, 220, 196, 0,   247, 262};
int badMelodyDelay[] = {250, 125, 125, 250, 250, 250, 250, 250};

//Blocking
void PlayBadSound()
{
  for( int thisNote = 0; thisNote < 8; thisNote++)
  {
    TimerFreeTone(TONE_PIN, badMelody[thisNote], badMelodyDelay[thisNote]);
  }
}
int goodMelody[] =      {262, 196, 196, 220, 196, 0,   247, 262};
int goodMelodyDelay[] = {250, 125, 125, 250, 250, 250, 250, 250};

//Blocking
void PlayGoodSound()
{
  for( int thisNote = 0; thisNote < 8; thisNote++)
  {
    TimerFreeTone(TONE_PIN, goodMelody[thisNote], goodMelodyDelay[thisNote]);
  }
}



void ResetAllLEDs()
{
    SetAllLEDs(0);

    /*SetNeopixelColourNone(lhsCreatorPixels, creatorNumLEDs);
    SetNeopixelColourNone(rhsCreatorPixels, creatorNumLEDs);
    SetNeopixelColourNone(lhsCreatorToCombinorPixels, creatorToCombinorNumLEDs);
    SetNeopixelColourNone(rhsCreatorToCombinorPixels, creatorToCombinorNumLEDs);
    */
}

//Set LEDs to white at the amount specified (0-255)
void SetAllLEDs( int amount)
{
    analogWrite(woodLED, amount);
    analogWrite(fireLED, amount);
    analogWrite(waterLED, amount);
    analogWrite(metalLED, amount);
    analogWrite(earthLED, amount);

}

 void FadeAllLEDs(int amount)
 {
    woodTarget = amount;
    fireTarget = amount;
    metalTarget = amount;
    waterTarget = amount;
    earthTarget = amount;

    /*SetNeopixelColour(lhsCreatorPixels, creatorNumLEDs, amount, amount, amount);
    SetNeopixelColour(rhsCreatorPixels, creatorNumLEDs, amount, amount, amount);
    SetNeopixelColour(lhsCreatorToCombinorPixels, creatorToCombinorNumLEDs, amount, amount, amount);
    SetNeopixelColour(rhsCreatorToCombinorPixels, creatorToCombinorNumLEDs, amount, amount, amount);
    */
}



/*////Neopixel LED FUNCTIONS 
void SetNeopixelColourRed( Adafruit_NeoPixel neopixel, int numLEDs ) {
  SetNeopixelColour(neopixel, numLEDs, 255,0,0);
}
void SetNeopixelColourGreen( Adafruit_NeoPixel neopixel, int numLEDs ) {
  SetNeopixelColour(neopixel, numLEDs, 0,255,0);
}
void SetNeopixelColourYellow( Adafruit_NeoPixel neopixel, int numLEDs ) {
  SetNeopixelColour(neopixel, numLEDs, 255,255,0);
}
void SetNeopixelColourWhite( Adafruit_NeoPixel neopixel, int numLEDs ) {
  SetNeopixelColour(neopixel, numLEDs, 255,255,255);
}
void SetNeopixelColourBlue( Adafruit_NeoPixel neopixel, int numLEDs ) {
  SetNeopixelColour(neopixel, numLEDs, 0,0,255);
}
void SetNeopixelColourNone( Adafruit_NeoPixel neopixel, int numLEDs ) {
  SetNeopixelColour(neopixel, numLEDs, 0,0,0);
}
void SetNeopixelColour(Adafruit_NeoPixel neopixel, int numLEDs, int red, int green, int blue)
{
  /*for(int i=0;i<numLEDs;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    neopixel.setPixelColor(i, neopixel.Color(red, green, blue));
    neopixel.show(); // This sends the updated pixel color to the hardware.
    delay(10);
  }
}*/



// Parse serial input, take action if it's a valid character
void parseCommand(char c)
{
  switch (c)
  {
    case 's': // Successful Code
      PlayGoodSound();
      break;

    case 'f': // Incorrect code
      PlayBadSound();
      break;

    case 'g': // Light the Wood Creator LEDs
      woodTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      /*SetNeopixelColourGreen( lhsCreatorPixels, creatorNumLEDs );
      SetNeopixelColourGreen( rhsCreatorPixels, creatorNumLEDs );*/

      PlayGoodSound();
      break;
    case 'G': // Light the Combinor Green LEDs
      woodTarget = 255; 
      //SetNeopixelColourGreen( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourGreen( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'r': // Light the Fire Creator LEDs
      fireTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourRed( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourRed( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'R': // Light the Combinor Red LEDs
      fireTarget = 255;
      //SetNeopixelColourRed( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourRed( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'b': // Light the Water Creator LEDs
      waterTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourBlue( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourBlue( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'B': // Light the Combinor Blue LEDs
      waterTarget = 255;
      //SetNeopixelColourBlue( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourBlue( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'w': // Light the Metal Creator LEDs
      metalTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourWhite( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourWhite( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'W': // Light the Combinor White LEDs
      metalTarget = 255; 
      //SetNeopixelColourWhite( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourWhite( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'y': // Light the Earth Creator LEDs
      earthTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourYellow( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourYellow( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'Y': // Light the Combinor Yellow LEDs
      earthTarget = 255;
      //SetNeopixelColourYellow( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourYellow( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'n': // RESET - Turn off all RGB LEDs
    case 'N': // RESET - Turn off all RGB LEDs
      ResetAllLEDs();
      break;

    default: // If an invalid character, do nothing
      break;
  }
}

void SendPressedMessage( char msg )
{
    /*uint8_t test = 46;
    pressMessagePayload[0] = test;
    pressMessage.setFrameId(xbee.getNextFrameId());
    
    Serial.print(F("SENDING 'Pressed' Message to Co-ordinator: " ));
    Serial.print(pressMessagePayload[0], HEX);
    Serial.print(pressMessagePayload[1], HEX);
    Serial.print(pressMessagePayload[2], HEX);
    Serial.print(pressMessagePayload[3], HEX);

    Serial.println();
    //xbee.send(pressMessage);*/

    Serial.println(F("SENDING UID:"));
    for ( uint8_t i = 0; i < 4; i++) {  //
      placeMessagePayload[i] = 0x0;
      Serial.print(placeMessagePayload[i], HEX);
    }
    Serial.println();
  
    placeMessage.setFrameId(xbee.getNextFrameId());
    
    Serial.println("SENDING 'Placed' Message to Co-ordinator");
    //xbee.send(placeMessage);
    
    // Send the command and wait up to N ms for a response.  xbee loop continues during this time.
    uint8_t status = xbee.sendAndWait(placeMessage, 5000);
    if (status == 0)
    {
      Serial.println(F("SEND ACKNOWLEDGED"));
    }
    else
    { 
      Serial.print(F("SEND FAILED: "));
      printHex(status, 2);
      Serial.println();
    }
}

// UTIL FUNCTIONS
void printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "0x%%.%dX", precision);

     sprintf(tmp, format, num);
     Serial.print(tmp);
}
