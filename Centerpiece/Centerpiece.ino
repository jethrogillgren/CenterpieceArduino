/* @file HelloKeypad.pde
  || @version 1.0
  || @author Alexander Brevig
  || @contact alexanderbrevig@gmail.com
  ||
  || @description
  || | Demonstrates the simplest use of the matrix Keypad library.
  || #
*/
#include <Keypad.h>
#include <Adafruit_NeoPixel.h>

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
byte rowPins[ROWS] = {17, 19, 2, 15}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {18, 14, 16}; //connect to the column pinouts of the keypad


Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

const int keyDelay = 200;

//The brightness for a half-active LED Button, and the pre-game LED value
const int halfLitBrightness = 100;

//Button LEDs - These (PWM) pins should connect to a MOSFET which controlls the Button LED, as it is 12V.
int woodLED = 3;
int metalLED = 5;
int fireLED = 6;
int waterLED = 9;
int earthLED = 10;

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

int fadeAmount = 1; //Speed of fade
int fadeDelay = 20; //speed of fade

//Strip LEDs - These are WS2811 LEDs powered by 12V - used for the Activation lines between KoalaPos

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
int creatorNumLEDs = 10;
Adafruit_NeoPixel lhsCreatorPixels = Adafruit_NeoPixel(creatorNumLEDs, 4, NEO_GRB + NEO_KHZ800);//Ring around the LHS Creator KoalaPos
Adafruit_NeoPixel rhsCreatorPixels = Adafruit_NeoPixel(creatorNumLEDs, 7, NEO_GRB + NEO_KHZ800);//Ring around the RHS Creator KoalaPos

int creatorToCombinorNumLEDs = 10;
Adafruit_NeoPixel lhsCreatorToCombinorPixels = Adafruit_NeoPixel(creatorToCombinorNumLEDs, 8, NEO_GRB + NEO_KHZ800);//Creator->Combinor lhs
Adafruit_NeoPixel rhsCreatorToCombinorPixels = Adafruit_NeoPixel(creatorToCombinorNumLEDs, 12, NEO_GRB + NEO_KHZ800);//Creator->Combinor rhs




  //SOUND
#include <Tone.h>
int buzzerPin = 13;

Tone notePlayer;
int notes[] = { NOTE_A3,
                NOTE_B3,
                NOTE_C4,
                NOTE_D4,
                NOTE_E4,
                NOTE_F4,
                NOTE_G4,
                NOTE_A4,
                NOTE_B4,
                NOTE_C5
              };




void setup() {
  Serial.begin(9600);
  notePlayer.begin(buzzerPin);

  pinMode(woodLED, OUTPUT);
  pinMode(metalLED, OUTPUT);
  pinMode(fireLED, OUTPUT);
  pinMode(waterLED, OUTPUT);
  pinMode(earthLED, OUTPUT);

  //Initialize the NeoPixel library.
  lhsCreatorPixels.begin();
  rhsCreatorPixels.begin();
  lhsCreatorToCombinorPixels.begin();
  rhsCreatorToCombinorPixels.begin();

  // WHen resetting the room, we want to see that the lights are working.
  //They will go unlit when the Game starts and send a Rest Command.
  SetAllLEDs(halfLitBrightness); 
}

void loop() {

  //// SOUND
  notePlayer.stop();


  //// NETWORK COMMANDS
  if ( Serial.available() )
  {
    // If new input is available on serial port
    parseSerialInput(Serial.read()); // parse it
  }


  //// LEDs
  if( woodBrightness < woodTarget )
  {
    woodBrightness+=fadeAmount;
    analogWrite(woodLED, woodBrightness);
    delay( fadeDelay);
  }
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

    Serial.print(key); //This goes to the Game Server

    //SOUND
    switch (key)
    {
      case '1':
        notePlayer.play(notes[0]);
        delay(keyDelay);
        woodBrightness=1;
        break;
      case '2':
        notePlayer.play(notes[1]);
        delay(keyDelay);
        metalBrightness=1;
        break;
      case '3':
        notePlayer.play(notes[2]);
        delay(keyDelay);
        fireBrightness=1;
        break;
      case '4':
        notePlayer.play(notes[3]);
        delay(keyDelay);
        waterBrightness =1;
        break;
      case '5':
        notePlayer.play(notes[4]);
        delay(keyDelay);
        earthBrightness=1;
        break;
      case '6':
        notePlayer.play(notes[5]);
        delay(keyDelay);
        break;
      case '7':
        notePlayer.play(notes[6]);
        delay(keyDelay);
        break;
      case '8':
        notePlayer.play(notes[7]);
        delay(keyDelay);
        break;
      case '9':
        notePlayer.play(notes[8]);
        delay(keyDelay);
        break;
      case '0':
        notePlayer.play(notes[9]);
        delay(keyDelay);
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


//Blocking
void PlayBadSound()
{
  notePlayer.stop();
  notePlayer.play(NOTE_C2);
  delay(10);
  notePlayer.play(NOTE_B2);
  delay(10);
  notePlayer.play(NOTE_A2);

  delay(100);
  notePlayer.stop();
}
//Blocking
void PlayGoodSound()
{
  notePlayer.stop();
  notePlayer.play(NOTE_A5);
  delay(50);
  notePlayer.play(NOTE_B5);
  delay(50);
  notePlayer.play(NOTE_C5);

  delay(50);
  notePlayer.stop();
}
//Blocking
void PlayGoodSound2()
{
  notePlayer.stop();
  notePlayer.play(NOTE_A5);
  delay(50);
  notePlayer.play(NOTE_C5);
  delay(50);
  notePlayer.play(NOTE_E5);

  delay(50);
  notePlayer.stop();
}



void ResetAllLEDs()
{
    analogWrite(woodLED, 0);
    analogWrite(fireLED, 0);
    analogWrite(waterLED, 0);
    analogWrite(metalLED, 0);
    analogWrite(earthLED, 0);

    SetNeopixelColourNone(lhsCreatorPixels, creatorNumLEDs);
    SetNeopixelColourNone(rhsCreatorPixels, creatorNumLEDs);
    SetNeopixelColourNone(lhsCreatorToCombinorPixels, creatorToCombinorNumLEDs);
    SetNeopixelColourNone(rhsCreatorToCombinorPixels, creatorToCombinorNumLEDs);
}

//Set LEDs to white at the amount specified (0-255)
void SetAllLEDs( int amount)
{
    woodTarget = amount;
    fireTarget = amount;
    metalTarget = amount;
    waterTarget = amount;
    earthTarget = amount;

    SetNeopixelColour(lhsCreatorPixels, creatorNumLEDs, amount, amount, amount);
    SetNeopixelColour(rhsCreatorPixels, creatorNumLEDs, amount, amount, amount);
    SetNeopixelColour(lhsCreatorToCombinorPixels, creatorToCombinorNumLEDs, amount, amount, amount);
    SetNeopixelColour(rhsCreatorToCombinorPixels, creatorToCombinorNumLEDs, amount, amount, amount);
}



////Neopixel LED FUNCTIONS 
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
  for(int i=0;i<numLEDs;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    neopixel.setPixelColor(i, neopixel.Color(red, green, blue));
    neopixel.show(); // This sends the updated pixel color to the hardware.
    delay(10);
  }
}



// Parse serial input, take action if it's a valid character
void parseSerialInput(char c)
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
      SetNeopixelColourGreen( lhsCreatorPixels, creatorNumLEDs );
      SetNeopixelColourGreen( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'G': // Light the Combinor Green LEDs
      woodTarget = 255; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourGreen( lhsCreatorToCombinorPixels, creatorNumLEDs );
      SetNeopixelColourGreen( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'r': // Light the Fire Creator LEDs
      fireTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourRed( lhsCreatorPixels, creatorNumLEDs );
      SetNeopixelColourRed( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'R': // Light the Combinor Red LEDs
      fireTarget = 255; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourRed( lhsCreatorToCombinorPixels, creatorNumLEDs );
      SetNeopixelColourRed( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'b': // Light the Water Creator LEDs
      waterTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourBlue( lhsCreatorPixels, creatorNumLEDs );
      SetNeopixelColourBlue( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'B': // Light the Combinor Blue LEDs
      waterTarget = 255; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourBlue( lhsCreatorToCombinorPixels, creatorNumLEDs );
      SetNeopixelColourBlue( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'w': // Light the Metal Creator LEDs
      metalTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourWhite( lhsCreatorPixels, creatorNumLEDs );
      SetNeopixelColourWhite( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'W': // Light the Combinor White LEDs
      metalTarget = 255; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourWhite( lhsCreatorToCombinorPixels, creatorNumLEDs );
      SetNeopixelColourWhite( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
      PlayGoodSound();
      break;

    case 'y': // Light the Earth Creator LEDs
      earthTarget = halfLitBrightness; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourYellow( lhsCreatorPixels, creatorNumLEDs );
      SetNeopixelColourYellow( rhsCreatorPixels, creatorNumLEDs );

      PlayGoodSound();
      break;
    case 'Y': // Light the Combinor Yellow LEDs
      earthTarget = 255; //Start lighting the Button - the main loop will fade it up
      SetNeopixelColourYellow( lhsCreatorToCombinorPixels, creatorNumLEDs );
      SetNeopixelColourYellow( rhsCreatorToCombinorPixels, creatorNumLEDs );
      
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
