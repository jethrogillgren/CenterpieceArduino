/*
 * --------------------------------------------------------------------------------------------------------------------
 * XBee comms control 5 RGB LEDs and a keypad/buzzer.
 * --------------------------------------------------------------------------------------------------------------------
 *
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
byte rowPins[ROWS] = {A3, A1, A0, A5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A2, 7, A4}; //connect to the column pinouts of the keypad
//byte rowPins[ROWS] = {A3, A5, 7, A1}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {A4, A0, A2}; //connect to the column pinouts of the keypad


Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


//The brightness for a half-active LED Button, and the pre-game LED value
const int halfLitBrightness = 100;

//Button LEDs - These (PWM) pins should connect to a MOSFET which controlls the Button LED, as it is 12V.
int woodLED = 10;
int metalLED = 9;
int fireLED = 6;
int waterLED = 5;
int earthLED = 3;

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
SoftwareSerial xbeeSerial(2, 4); // RX, TX

//Works with Series1 and 2
XBeeWithCallbacks xbee;

#define MSG_WOOD_PRESSED 'w'
#define MSG_METAL_PRESSED 'm'
#define MSG_FIRE_PRESSED 'f'
#define MSG_WATER_PRESSED 'W'
#define MSG_EARTH_PRESSED 'e'

#define MSG_WOOD_CREATOR 'g'
#define MSG_METAL_CREATOR 'w'
#define MSG_FIRE_CREATOR 'r'
#define MSG_WATER_CREATOR 'b'
#define MSG_EARTH_CREATOR 'y'
#define MSG_WOOD_COMBINOR 'G'
#define MSG_METAL_COMBINOR 'W'
#define MSG_FIRE_COMBINOR 'R'
#define MSG_WATER_COMBINOR 'B'
#define MSG_EARTH_COMBINOR 'Y'

#define MSG_RESET   'n'

// Build a reuseable message packet to send to the Co-Ordinator
XBeeAddress64 coordinatorAddr = XBeeAddress64(0x00000000, 0x00000000);

uint8_t pressMessagePayload[1] = {0};
ZBTxRequest pressMessage = ZBTxRequest(coordinatorAddr, pressMessagePayload, sizeof(pressMessagePayload));




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

  //// NETWORK COMMANDS
  // Continuously let xbee read packets and call callbacks.
  xbee.loop();

  ////LED Buttons
  /*Serial.print( "Wood: ");
  Serial.print(analogRead(woodPress) );
  Serial.print( "    Metal: ");
  Serial.println(analogRead(metalPress) );
*/

//return;

  if ( analogRead(woodPress) > 200 )
  {
    Serial.println("Wood Pressed");
    SendPressedMessage(MSG_WOOD_PRESSED, false);
  }
  if ( analogRead(metalPress) > 200 )
  {
    Serial.println("Metal Pressed");
    SendPressedMessage(MSG_METAL_PRESSED, false);
  }
  if ( digitalRead(firePress) == HIGH )
  {
    Serial.println("Fire Pressed");
    SendPressedMessage(MSG_FIRE_PRESSED, false);
  }
  if ( digitalRead(waterPress) == HIGH )
  {
    Serial.println("Water Pressed");
    SendPressedMessage(MSG_WATER_PRESSED, false);
  }
  if ( digitalRead(earthPress) == HIGH )
  {
    Serial.println("Earth Pressed");
    SendPressedMessage(MSG_EARTH_PRESSED, false);
  }

  //// LEDs

  if ( woodBrightness < woodTarget )
  {
    woodBrightness += fadeAmount;
    analogWrite(woodLED, woodBrightness);
    delay( fadeDelay);
  }
  if ( metalBrightness < metalTarget )
  {
    metalBrightness += fadeAmount;
    analogWrite(metalLED, metalBrightness);
    delay( fadeDelay);
  }
  if ( fireBrightness < fireTarget )
  {
    fireBrightness += fadeAmount;
    analogWrite(fireLED, fireBrightness);
    delay( fadeDelay);
  }
  if ( waterBrightness < waterTarget)
  {
    waterBrightness += fadeAmount;
    analogWrite(waterLED, waterBrightness);
    delay( fadeDelay);
  }
  if ( earthBrightness < earthTarget )
  {
    earthBrightness += fadeAmount;
    analogWrite(earthLED, earthBrightness);
    delay( fadeDelay);
  }


  //KEYPAD PRESSES
  char key = keypad.getKey();
  if (key) {

    Serial.println(key + " pressed");
    SendPressedMessage( key, false ); //This goes to the Game Server

    //SOUND
    switch (key)
    {
      case '1':
        TimerFreeTone(TONE_PIN, notes[1], keyDelay);
        break;
      case '2':
        TimerFreeTone(TONE_PIN, notes[2], keyDelay);
        break;
      case '3':
        TimerFreeTone(TONE_PIN, notes[3], keyDelay);
        break;
      case '4':
        TimerFreeTone(TONE_PIN, notes[4], keyDelay);
        break;
      case '5':
        TimerFreeTone(TONE_PIN, notes[5], keyDelay);
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
        //PlayGoodSound();
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




int badMelody[] =      {196, 196, 196};
int badMelodyDelay[] = {125, 125, 125};
int goodMelody[] =      {523.25, 659.26, 1046.5};
int goodMelodyDelay[] = {125 , 125,  250};
int tuneMelody[] =      {262, 196, 196, 220, 196, 0,   247, 262};
int tuneMelodyDelay[] = {250, 125, 125, 250, 250, 250, 250, 250};

//Blocking
void PlayBadSound()
{
  for ( int thisNote = 0; thisNote < 3; thisNote++)
  {
    TimerFreeTone(TONE_PIN, badMelody[thisNote], badMelodyDelay[thisNote]);
  }
}
//Blocking
void PlayGoodSound()
{
  for ( int thisNote = 0; thisNote < 5; thisNote++)
  {
    TimerFreeTone(TONE_PIN, goodMelody[thisNote], goodMelodyDelay[thisNote]);
  }
}
//Blocking
void PlayTune()
{
  for ( int thisNote = 0; thisNote < 8; thisNote++)
  {
    TimerFreeTone(TONE_PIN, tuneMelody[thisNote], tuneMelodyDelay[thisNote]);
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


  //No fading needed, but we want to update the fade values anyway.
  FadeAllLEDs(amount);
  
  woodBrightness = amount;
  fireBrightness = amount;
  metalBrightness = amount;
  waterBrightness = amount;
  earthBrightness = amount;
  
  Serial.print("Set all LEDs: ");
  Serial.println(amount);
}

//Only fades up.
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
  Serial.print(F("Handling command  HEX: 0x" ));
  Serial.print(c, HEX);
  Serial.print(F(" ASCII:" ));
  Serial.print(c);
  Serial.println();
  
  switch (c)
  {
    case 's': // Successful Code
      PlayGoodSound();
      break;

    case 'f': // Incorrect code
      PlayBadSound();
      break;

    case MSG_WOOD_CREATOR: // Light the Wood Creator LEDs
      woodTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      /*SetNeopixelColourGreen( lhsCreatorPixels, creatorNumLEDs );
        SetNeopixelColourGreen( rhsCreatorPixels, creatorNumLEDs );*/

      //PlayGoodSound();
      break;
    case MSG_WOOD_COMBINOR: // Light the Combinor Green LEDs
      woodTarget = 255;
      //SetNeopixelColourGreen( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourGreen( rhsCreatorToCombinorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;

    case MSG_FIRE_CREATOR: // Light the Fire Creator LEDs
      fireTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourRed( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourRed( rhsCreatorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;
    case MSG_FIRE_COMBINOR: // Light the Combinor Red LEDs
      fireTarget = 255;
      //SetNeopixelColourRed( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourRed( rhsCreatorToCombinorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;

    case MSG_WATER_CREATOR: // Light the Water Creator LEDs
      waterTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourBlue( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourBlue( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case MSG_WATER_COMBINOR: // Light the Combinor Blue LEDs
      waterTarget = 255;
      //SetNeopixelColourBlue( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourBlue( rhsCreatorToCombinorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;

    case MSG_METAL_CREATOR: // Light the Metal Creator LEDs
      metalTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourWhite( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourWhite( rhsCreatorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;
    case MSG_METAL_COMBINOR: // Light the Combinor White LEDs
      metalTarget = 255;
      //SetNeopixelColourWhite( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourWhite( rhsCreatorToCombinorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;

    case MSG_EARTH_CREATOR: // Light the Earth Creator LEDs
      earthTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      //SetNeopixelColourYellow( lhsCreatorPixels, creatorNumLEDs );
      //SetNeopixelColourYellow( rhsCreatorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;
    case MSG_EARTH_COMBINOR: // Light the Combinor Yellow LEDs
      earthTarget = 255;
      //SetNeopixelColourYellow( lhsCreatorToCombinorPixels, creatorNumLEDs );
      //SetNeopixelColourYellow( rhsCreatorToCombinorPixels, creatorNumLEDs );

      //PlayGoodSound();
      break;

    case MSG_RESET: // Turn off all RGB LEDs
      SetAllLEDs(0);
      break;

    default: // If an invalid character, do nothing
      break;
  }
}

void SendPressedMessage( char msg, bool async )
{
  pressMessagePayload[0] = msg;
  pressMessage.setFrameId(xbee.getNextFrameId());

  Serial.print(F("SENDING Message to Co-ordinator: " ));
  Serial.print(pressMessagePayload[0], HEX);
  Serial.println();

if(async)
{

  // Send the command and wait up to N ms for a response.  xbee loop continues during this time.
  uint8_t status = xbee.sendAndWait(pressMessage, 5000);
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
else
{
  xbee.send(pressMessage);
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
