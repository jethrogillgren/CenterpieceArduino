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

//KEYPAD
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 7, 8, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 2, 4}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

const int keyDelay = 200;

//SOUND
#include <Tone.h>

int notes[] = { NOTE_A3,
                NOTE_B3,
                NOTE_C4,
                NOTE_D4,
                NOTE_E4,
                NOTE_F4,
                NOTE_G4,
                NOTE_A5,
                NOTE_B5,
                NOTE_C5};
int noteStar = NOTE_C2;
int noteHash = NOTE_D2;


// You can declare the tones as an array
Tone notePlayer;



void setup(){
  Serial.begin(9600);
  notePlayer.begin(11);
}
  
void loop(){

  notePlayer.stop();

  char key = keypad.getKey();
  if (key){
  
    //SOUND
    switch(key)
    {
      case '1':
        notePlayer.play(notes[0]);
        delay(keyDelay);
        break;
      case '2':
        notePlayer.play(notes[1]);
        delay(keyDelay);
        break;
      case '3':
        notePlayer.play(notes[2]);
        delay(keyDelay);
        break;
      case '4':
        notePlayer.play(notes[3]);
        delay(keyDelay);
        break;
      case '5':
        notePlayer.play(notes[4]);
        delay(keyDelay);
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
        notePlayer.play(noteStar);
        delay(5);
        notePlayer.play(noteStar);
        delay(5);
        notePlayer.play(noteStar);
        delay(100);
        break;

      case '#':
        notePlayer.play(noteHash);
        delay(5);
        notePlayer.play(noteHash);
        delay(5);
        notePlayer.play(noteHash);
        
        delay(100);
        break;
  
      default:
        break;
    }

    //Actions
    

  } //else we idle loop
}
