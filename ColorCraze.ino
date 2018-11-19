// This is a game loosely based off the classic board game "Mastermind," in which the player
// guesses the color and position of a number of a hidden set.
//
// In this game, "Color Craze," the player guesses the order of colored cards. They are given 3 colored cards, 
// red, green, and blue, which they they hold up to a color sensor to make their guesses.
// Their guesses are displayed on the matrix as they make them, then the wrong guesses are removed
// and the right guesses remain for a couple of seconds, and then a message tells them the number
// of correct guesses. Given this feedback, the player guesses again. If they guess correctly
// a win screen is displayed on the matrix. If they don't after a set number of tries,
// a lose screen is displayed.
//
// The game has 3 levels. Each level has more members in the set to guess and more 
// guesses to get the correct answer. The player chooses the level from the start 
// screen, which instructs them to choose a level by holding up a specific color of card.
//
// Two cards may not be played in a row. This is due to the fact that the card color 
// is read by a change in the color sensor value. A good addition to the program would
// be to read the color without requiring a color change, thus allowing multiple
// cards in a row of the same color.

#include <RGBmatrixPanel.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// These values come from the matrix library samples available on Adafruit.
#define CLK  8   // USE THIS ON ARDUINO UNO, ADAFRUIT METRO M0, etc.
//#define CLK A4 // USE THIS ON METRO M4 (not M0)
//#define CLK 11 // USE THIS ON ARDUINO MEGA
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3

// Initialize the color sensor and matrix display
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// enum used to store value of color guesses
enum Color {none, red, green, blue};

// test function for color sensor
void testColorSensor() {
  if (tcs.begin()) {
    Serial.println("Color sensor is working.");
  } else {
    Serial.println("Could not find color sensor, check your connections.");
    while (1); // halt!
  }
}

// clear the screen
void clearScreen() {
  matrix.fillScreen(matrix.Color333(0, 0, 0));
}

// read in a color from the sensor. the comparison for each color is different
// as the color sensor responds differently to different colors. In particular,
// red seemed more easily detected than other colors.
// change these as needed.
Color getColor() {
  uint16_t clearVal, r, g, b;
  tcs.getRawData(&r, &g, &b, &clearVal);

  if (clearVal > 500) {
    if (r > (g + b)) {
      return red;
    }
    else if (b > g && b > r) {
      return blue;
    }
    else if (g > r && g > b) {
      return green;
    }
    else {
      return none;
    }
  }
  else {
    return none;
  }
}

// the cards are displayed inside borders, or "slots." this is useful because
// empty slots will signify that a wrong guess was made.
void drawSlots(int numSlots) {
  for (int i=0; i<numSlots; i++) {
    if (numSlots == 3) {
      int startWidth=i*11;
      matrix.drawRect(startWidth, 0, 10, 15, matrix.Color333(1, 1, 1));
    }
    if (numSlots == 4) {
      int startWidth=i*8;
      matrix.drawRect(startWidth, 0, 7, 15, matrix.Color333(1, 1, 1));
    }
    if (numSlots == 5) {
      int startWidth=i*6 + 1 ;
      matrix.drawRect(startWidth, 0, 5, 15, matrix.Color333(1, 1, 1));
    }
  }
}

// this function draw a card on the display. The values are different for r, g, and b
// to make the actual brightness of the colors equal.
void drawCard(Color color, int slot, int numSlots) {
  int r;
  int g;
  int b;
  int startWidth;

  if (color == none) {
    r = 0;
    g = 0;
    b = 0;
  }
  if (color == red) {
    r = 5;
    g = 0;
    b = 0;
  }
  if (color == green) {
    r = 0;
    g = 3;
    b = 0;
  }
  if (color == blue) {
    r = 0;
    g = 0;
    b = 7;
  }
  if (numSlots == 3) {
    startWidth=(slot * 11) + 1;
    matrix.fillRect(startWidth, 1, 8, 13, matrix.Color333(r, g, b));
  }
  if (numSlots == 4) {
    startWidth=(slot * 8) + 1;
    matrix.fillRect(startWidth, 1, 5, 13, matrix.Color333(r, g, b));
  }
  if (numSlots == 5) {
    startWidth=(slot * 6) + 2;
    matrix.fillRect(startWidth, 1, 3, 13, matrix.Color333(r, g, b));
  }
}

// this calls getColor to read the color sensor and draws a card based on
// what color it reads.
// because we only detect a card was read when the color changes, it is not
// possible to play two cards in a row of the same color.
Color displayNextCard(Color prev, int slot, int numSlots) {
  Color color = prev;

  while (color == prev || color == none) {
    color = getColor();
  }
  drawCard(color, slot, numSlots);
  return color;   
}

// this generates the "correct" set that the player will attempt to guess
// using a random number generator. 
void getCorrectSet(Color correctSet[], int numSlots) {
  int index = 0;

  // total number of combinations is just the number of colors
  // to the power of the number of slots. e.g. if there 4 slots, there would be 
  // 81 possible combinations of red, green, and blue.
  int numCombos = pow(3, numSlots);

  // seed the random number generator with a unique value
  // be sure to use an unused pin, otherwise the program
  // will have undefined and undesired behavior
  srand((unsigned)analogRead(A5));

  // standard method to get random numbers within a range
  int num = rand() % numCombos;
  int prev = -1;
  
  while (index < numSlots) {
    // calculate the next digit using modulus. this is just the standard algorithm
    // for converting decimal to binary, except here it is base-3 (3 cards) rather than base-2.
    int digit = (enum Color)((num % 3) + 1);

    // don't allow two digits in a row since we can't read two in a row from the sensor
    if (digit == prev) {
      if (digit != 3) {
        digit++;
      }
      else {
        digit = 1;
      }
    }
    correctSet[index] = (enum Color)(digit);

    // save off digit in case its the same next time
    prev = digit;

    // divide by setSize to move to next place value
    num /= numSlots; 
    index++;
  }
}

// startscreen toggles between two screens, the actual flashy title-screen and another
// screen with instructions for choosing levels. 
int startScreen() {
  Color color = none;
  drawSlots(3);
  bool startSignal = false;
  while (startSignal == false) {
    int counter = 0;
    while (color == none && counter < 10) {
      color = getColor();
      titleScreen();
      counter++;
    }
    if (color != none) {
      startSignal = true;
    }
    clearScreen();
    while (color == none && counter < 90) {
      color = getColor();
      levelChoose();
      counter++;
    }
    if (color != none) {
      startSignal = true;
    }
    clearScreen();
  }
  if (color == green) {
    return 1;
  }
  if (color == blue) {
    return 2;
  }
  if (color == red) {
    return 3;
  }
}

// display a title screen with the name and flashing cards 
void titleScreen() {
  drawSlots(1);
  matrix.setCursor(1, 16);   
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(0,7,7));
  matrix.print("COLOR");
  matrix.setCursor(1, 24);
  matrix.print("CRAZE");
  drawCard(red, 0, 3);
  drawCard(green, 1, 3);
  drawCard(blue, 2, 3);
  delay(100);
  drawCard(green, 0, 3);
  drawCard(blue, 1, 3);
  drawCard(red, 2, 3);
  delay(100);
  drawCard(blue, 0, 3);
  drawCard(red, 1, 3);
  drawCard(green, 2, 3);
  delay(100);
}

// display a screen with visual instructions for choosing a level
void levelChoose() {
  matrix.fillRect(0, 1, 7, 9, matrix.Color333(0, 7, 0));
  matrix.setCursor(8, 2);   
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(0,7,0));
  matrix.print("EASY");

  matrix.fillRect(0, 11, 7, 9, matrix.Color333(0, 0, 7));
  matrix.setCursor(8, 12);   
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(0,0,7));
  matrix.print("MED");

  matrix.fillRect(0, 21, 7, 9, matrix.Color333(7, 0, 0));
  matrix.setCursor(8, 22);   
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print("HARD");
}

// display a screen when the player loses
void loseScreen() {
  matrix.setCursor(8, 10);
  // print each letter with a rainbow color
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print('Y');
  matrix.setTextColor(matrix.Color333(0,7,0)); 
  matrix.print('O');
  matrix.setTextColor(matrix.Color333(0,0,7));
  matrix.print('U');
  matrix.setCursor(5, 18);   // next line
  matrix.setTextColor(matrix.Color333(7,0,0)); 
  matrix.print('L');
  matrix.setTextColor(matrix.Color333(0,7,0));  
  matrix.print('O');
  matrix.setTextColor(matrix.Color333(0,7,7)); 
  matrix.print('S');
  matrix.setTextColor(matrix.Color333(0,0,7));
  matrix.print('E');
}

// display a screen when the user whens
void winScreen() {
  matrix.setCursor(8, 10);
  // print each letter with a rainbow color
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print('Y');
  matrix.setTextColor(matrix.Color333(0,7,0)); 
  matrix.print('O');
  matrix.setTextColor(matrix.Color333(0,0,7));
  matrix.print('U');
  matrix.setCursor(8, 18);   // next line
  matrix.setTextColor(matrix.Color333(7,0,0)); 
  matrix.print('W');
  matrix.setTextColor(matrix.Color333(0,7,0));  
  matrix.print('I');
  matrix.setTextColor(matrix.Color333(0,0,7));
  matrix.print('N');
}

// after displaying the players guesses, remove the wrong guesses from the display, thus
// leaving the correct guesses to provide feedback to the player.
int removeWrongGuesses(Color guessedSet[], Color correctSet[], int numSlots) {
  int numWrongGuesses = 0;
  Color guessedCorrectlySet [numSlots];

  // caculate and array of right and wrong guess
  for (int i=0; i < numSlots; i++) {
    if (guessedSet[i] != correctSet[i]) {
      guessedCorrectlySet[i] = none;
      delay(500);
      numWrongGuesses++;
    }
    else {
      guessedCorrectlySet[i] = guessedSet[i];
    }
  } 
  
  // black out the wrong guesses
  for (int j=0; j < numSlots; j++) {
    drawCard(guessedCorrectlySet[j], j, numSlots);
  }    

  // print the number of right guesses
  int numRightGuesses = numSlots - numWrongGuesses;
  printNumRightGuesses(numRightGuesses);
  clearScreen();
  drawSlots(numSlots);

  return numWrongGuesses;
}

// in addition to diplaying the correctly guessed cards, specifically tell the player 
// how many they guessed correctly
void printNumRightGuesses(int numRightGuesses) {
  matrix.setCursor(8, 16);   // next line
  matrix.setTextColor(matrix.Color333(4,0,4)); 
  if (numRightGuesses == 0) {
    matrix.setCursor(5, 16);
    matrix.print("NONE");
  }
  if (numRightGuesses == 1) {
    matrix.print("ONE");
  }
  if (numRightGuesses == 2) {
    matrix.print("TWO");
  }
  if (numRightGuesses == 3) {
    matrix.setCursor(1, 16);
    matrix.print("THREE");
  }
  if (numRightGuesses == 4) {
    matrix.setCursor(5, 16);
    matrix.print("FOUR");
  }
  if (numRightGuesses == 5) {
    matrix.setCursor(5, 16);
    matrix.print("FIVE");
  }
  matrix.setCursor(2, 24);   // next line 
  matrix.print("RIGHT");
  delay(1000);
}

// display the instructions. have to keep it simple becuase: a) the font is very large, and
// b) people don't read :)
void Instructions() {
  matrix.setCursor(1, 0);   
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print("Guess");
  matrix.setCursor(7, 8);
  matrix.setTextColor(matrix.Color333(0,7,0));
  matrix.print("the");
  matrix.setCursor(4, 16);
  matrix.setTextColor(matrix.Color333(0,7,7));
  matrix.print("card");
  matrix.setCursor(1, 24);
  matrix.setTextColor(matrix.Color333(0,0,7));
  matrix.print("order");
}

// wraps reading and displaying each card over the full set of slots
// and sets the members of the guessedSet array which is used to compare
// to the correctSet array. 
// 
// the previous guess is passed to displayNextCard so that it knows
// what color it is comparing to when looking for a change of color
// from the color sensor.
void readAndDisplayCards(Color guessedSet[], int numSlots) {
  Color prev = none;

  for (int i = 0; i < numSlots; i++) {
    Color prev = displayNextCard(prev, i, numSlots);
    guessedSet[i] = prev;
  }
}

// the entry point for the game
//   - displays instructions
//   - draws slots
//   - gets the randomly generated correct set to be guessed
//   - for the number of guesses for a given level, 
//     reads the players guesses, displays them, and removes the wrong guesses
bool game(int level) {
  // change these as desired to change level difficulty
  int numSlots = level + 2;
  int numGuesses = level + 2;
  
  Instructions();
  delay(2000);
  clearScreen();
  
  drawSlots(numSlots);
  Color correctSet [numSlots];
  getCorrectSet(correctSet, numSlots);

  Color guessedSet [numSlots];
  
  for (int i = 0; i < numSlots; i++) {
    readAndDisplayCards(guessedSet, numSlots);
    int numWrongGuesses = removeWrongGuesses(guessedSet, correctSet, numSlots);
    if (numWrongGuesses == 0) {
      return true;
    }
  }
  return false;
}

// setup ...
void setup() {
  // testColorSensor();
  Serial.begin(9600);
  delay(2000);
  matrix.begin();
}

// control flow for the game
//   - start screen
//   - play game
//   - depending on outcome, display win or lose screen
void loop() {
  int level = startScreen();
  bool outcome;
  
  clearScreen();
  delay(1000);
  
  //true = win, false = lose
  Serial.println("Calling game");
  outcome = game(level);
  clearScreen();
  if (outcome) {
    winScreen();
  }
  else {
    loseScreen();
  }
  delay(3000);
  clearScreen();
}  
