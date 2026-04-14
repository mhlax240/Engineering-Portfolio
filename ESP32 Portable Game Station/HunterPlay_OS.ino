//HUNTERPLAY OPERATING SYSTEM

// =========================================================
// Includes & Hardware Defines
// =========================================================
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>  // [web:6]

// TFT pins
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
//sck  18, mosi 23, miso 19

// Button pins
const int BUTTON1_PIN = 25;
const int BUTTON2_PIN = 27;
const int BUTTON3_PIN = 22;

// Joystick pins
const int JOY_X_PIN   = 32;
const int JOY_Y_PIN   = 33;

// Joystick thresholds
const int JOY_UP_THRESH   = 600;
const int JOY_DOWN_THRESH = 3000;

// Buzzer pin
const int BUZZ_PIN = 26;

// =========================================================
// Types & Global State
// =========================================================
enum SystemState {
  STATE_BOOT,
  STATE_MENU_MAIN,

  STATE_G1_MENU,
  STATE_G1_PLAY,

  STATE_G2_MENU,
  STATE_G2_PLAY,

  STATE_G3_MENU,
  STATE_G3_PLAY
};

SystemState currentState = STATE_BOOT;

struct Controls {
  bool b1;
  bool b2;
  bool b3;
  int  x;
  int  y;
};

// Button edge tracking
bool lastB1 = false;
bool lastB2 = false;
bool lastB3 = false;

// TFT object
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// ----- Game 2 (SEQUENCE) state -----
const int G2_MAX_STEPS = 20;
int g2Sequence[G2_MAX_STEPS];
int g2Length      = 0;     // current sequence length
bool g2PlayerTurn = false; // false = showing sequence, true = player's turn
int g2PlayerIndex = 0;     // index in sequence during player input
bool g2GameOver   = false; // used for game-over screen


// =========================================================
// Menu Data
// =========================================================
struct MenuItem {
  const char* label;
  int x;
  int y;
};

MenuItem menuItems[] = {
  { "Game 1", 70,  20 },
  { "Sequence",  70,  80 },
  { "Game 3", 70, 140 }
};

const int MENU_ITEMS = sizeof(menuItems) / sizeof(menuItems[0]);
int menuIndex = 0;

uint16_t menuColor(int i) {
  return (i == menuIndex) ? ILI9341_YELLOW : ILI9341_WHITE;
}

// =========================================================
// I/O Helpers
// =========================================================
void configureInputs() {
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(JOY_X_PIN, INPUT);
  pinMode(JOY_Y_PIN, INPUT);
  pinMode(BUZZ_PIN, OUTPUT);
}

Controls readInputs() {
  Controls c;
  c.b1 = (digitalRead(BUTTON1_PIN) == LOW);
  c.b2 = (digitalRead(BUTTON2_PIN) == LOW);
  c.b3 = (digitalRead(BUTTON3_PIN) == LOW);
  c.x  = analogRead(JOY_X_PIN);
  c.y  = analogRead(JOY_Y_PIN);
  return c;
}

// =========================================================
// Screen / Drawing Helpers
// =========================================================
void configureScreen() {
  SPI.begin(18, 19, 23, TFT_CS);
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
}

void drawBootScreen() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 90);
  tft.println("HunterPlay");
}

void drawMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);

  for (int i = 0; i < MENU_ITEMS; i++) {
    uint16_t c = menuColor(i);

    tft.drawRect(menuItems[i].x,
                 menuItems[i].y,
                 200, 40,
                 c);

    tft.setCursor(menuItems[i].x + 15,
                  menuItems[i].y + 10);
    tft.setTextColor(c);
    tft.print(menuItems[i].label);
  }
}

// -------------  Game 1 -------------
void drawG1MenuScreen() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(2);
}

void drawG1PlayScreen() {
  tft.fillScreen(ILI9341_BLUE);
}

// ------------- Game 2 Screens -------------
void drawG2MenuScreen() {
  tft.fillScreen(ILI9341_WHITE);

  // Title: "SEQUENCE"
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(40, 40);   // adjust to taste
  tft.print("SEQUENCE");

  // Subtitle: "Press 1 to play"
  tft.setTextSize(2);
  tft.setCursor(40, 90);   // slightly below
  tft.print("Press 1 to play");
}

void drawG2PlayScreen() {
  tft.fillScreen(ILI9341_GREEN);
  tft.setTextSize(2);
}
// Moves: 0=BTN1, 1=BTN2, 2=BTN3, 3=UP, 4=DOWN, 5=LEFT, 6=RIGHT
int g2ReadMove(const Controls& in) {
  // buttons (your Controls already reflect pressed = true)
  if (in.b1) return 0;
  if (in.b2) return 1;
  if (in.b3) return 2;

  // joystick directions using existing thresholds
  if (in.y < JOY_UP_THRESH)   return 3; // Up
  if (in.y > JOY_DOWN_THRESH) return 4; // Down
  if (in.x > JOY_DOWN_THRESH) return 5; // Left
  if (in.x < JOY_UP_THRESH)   return 6; // Right

  return -1;  // nothing pressed / centered
}

const char* g2MoveName(int m) {
  switch (m) {
    case 0: return "button 1";
    case 1: return "button 2";
    case 2: return "button 3";
    case 3: return "Up";
    case 4: return "Down";
    case 5: return "Left";
    case 6: return "Right";
  }
  return "?";
}

void g2ShowMessage(const char* line1, const char* line2 = nullptr, uint16_t bg = ILI9341_WHITE) {
  tft.fillScreen(bg);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.print(line1);
  if (line2) {
    tft.setCursor(20, 100);
    tft.print(line2);
  }
}

void g2ShowMoveName(const char* name) {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 70);
  tft.print(name);
}
// ------------- Game 3 Screens -------------
void drawG3MenuScreen() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(2);
}

void drawG3PlayScreen() {
  tft.fillScreen(ILI9341_RED);
  tft.setTextSize(2);
}

// =========================================================
// State Update Functions
// =========================================================

// ---------- Main Menu ----------
SystemState updateMainMenu(const Controls& in) {
  if (in.y > JOY_DOWN_THRESH) {
    menuIndex = (menuIndex + 1) % MENU_ITEMS;
    delay(200);
    drawMainMenu();
  } else if (in.y < JOY_UP_THRESH) {
    menuIndex = (menuIndex - 1 + MENU_ITEMS) % MENU_ITEMS;
    delay(200);
    drawMainMenu();
  }

  bool b1PressedEdge = (in.b1 && !lastB1);
  lastB1 = in.b1;

  Serial.print("menuIndex = ");
  Serial.print(menuIndex);
  Serial.print("  b1 = ");
  Serial.print(in.b1);
  Serial.print("  edge = ");
  Serial.println(b1PressedEdge);

  if (b1PressedEdge) {
    if (menuIndex == 0) {
      drawG1MenuScreen();
      return STATE_G1_MENU;
    } else if (menuIndex == 1) {
      drawG2MenuScreen();
      return STATE_G2_MENU;
    } else if (menuIndex == 2) {
      drawG3MenuScreen();
      return STATE_G3_MENU;
    }
  }

  return STATE_MENU_MAIN;
}

// ---------- Game 1 ----------
SystemState updateG1Menu(const Controls& in) {
  bool b1Edge = (in.b1 && !lastB1);
  bool b2Edge = (in.b2 && !lastB2);
  lastB1 = in.b1;
  lastB2 = in.b2;

  if (b2Edge) {
    drawMainMenu();
    return STATE_MENU_MAIN;
  }

  if (b1Edge) {
    drawG1PlayScreen();
    delay(10);
    return STATE_G1_PLAY;
  }

  return STATE_G1_MENU;
}

SystemState updateG1Play(const Controls& in) {
  (void)in;  // unused for now
  drawG1PlayScreen();
  delay(3000);
  drawG1MenuScreen();
  return STATE_G1_MENU;
}

// ---------- GAME 2 ----------
SystemState updateG2Menu(const Controls& in) {
  bool b1Edge = (in.b1 && !lastB1);
  bool b2Edge = (in.b2 && !lastB2);
  lastB1 = in.b1;
  lastB2 = in.b2;

  if (b2Edge) {
    drawMainMenu();
    return STATE_MENU_MAIN;
  }

  if (b1Edge) {
    // init sequence game state
    g2Length      = 0;
    g2PlayerTurn  = false;
    g2PlayerIndex = 0;
    g2GameOver    = false;

    g2ShowMessage("Sequence Game");
    delay(800);

     // Reset button edge tracking so the start-press doesn't leak into play
  lastB1 = false;
  lastB2 = false;
  lastB3 = false;
  
    return STATE_G2_PLAY;
  }

  return STATE_G2_MENU;
}

SystemState updateG2Play(const Controls& in) {
  // Button edges for this state
  bool b1Edge = (in.b1 && !lastB1);
  bool b2Edge = (in.b2 && !lastB2);
  bool b3Edge = (in.b3 && !lastB3);
  lastB1 = in.b1;
  lastB2 = in.b2;
  lastB3 = in.b3;

  // If game over, wait for B2 to go back to Sequence menu
  if (g2GameOver) {
    if (b2Edge) {
      drawG2MenuScreen();
      g2GameOver = false;
      return STATE_G2_MENU;
    }
    return STATE_G2_PLAY;
  }

  // Phase 1: not yet in player turn → grow sequence and show it
  if (!g2PlayerTurn) {
    if (g2Length < G2_MAX_STEPS) {
      g2Sequence[g2Length] = random(0, 7);  // 0..6 (all moves)
      g2Length++;
    }

    // showSequence()
    g2ShowMessage("Watch...");
    delay(700);

    for (int i = 0; i < g2Length; i++) {
      g2ShowMoveName(g2MoveName(g2Sequence[i]));
      delay(600);
      tft.fillScreen(ILI9341_WHITE);
      delay(200);
    }

    // switch to player input phase
    g2PlayerTurn  = true;
    g2PlayerIndex = 0;
    g2ShowMessage("Your turn");
    delay(200);
    return STATE_G2_PLAY;
  }

  // Phase 2: player's turn – read and compare moves

  // We treat any current pressed control as a move
 // Map joystick + button edges to a single move code
  int move = -1;

  // Joystick dead zone
  const int centerLow  = 1400;
  const int centerHigh = 2600;

  bool joyUp    = (in.y < JOY_UP_THRESH);
  bool joyDown  = (in.y > JOY_DOWN_THRESH);
  bool joyLeft  = (in.x < JOY_UP_THRESH);
  bool joyRight = (in.x > JOY_DOWN_THRESH);

  bool joyCentered = (in.x > centerLow && in.x < centerHigh &&
                      in.y > centerLow && in.y < centerHigh);

  if (!joyCentered) {
    if (joyUp)         move = 3;
    else if (joyDown)  move = 4;
    else if (joyLeft)  move = 5;
    else if (joyRight) move = 6;
  }

  // If no joystick move, check button EDGES (not held)
  if (move == -1) {
    if (b1Edge)      move = 0;
    else if (b2Edge) move = 1;
    else if (b3Edge) move = 2;
  }


  if (move == -1) {
    // nothing yet; wait
    return STATE_G2_PLAY;
  }

  // simple debounce / ignore repeats briefly
  delay(150);

  g2ShowMoveName(g2MoveName(move));
  delay(300);

  // compare with expected
  if (move != g2Sequence[g2PlayerIndex]) {
    // wrong move → show score and set game over
    char buf[16];
    snprintf(buf, sizeof(buf), "Score: %d", g2Length - 1);

    tft.fillScreen(ILI9341_RED);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 60);
    tft.print("Game Over!");
    tft.setCursor(20, 100);
    tft.print(buf);

    g2GameOver = true;
    return STATE_G2_PLAY;
  }

  // correct move
  g2PlayerIndex++;

  if (g2PlayerIndex >= g2Length) {
    // completed this round
    g2ShowMessage("Correct!");
    delay(800);
    g2PlayerTurn = false;  // next call will grow & show next sequence
  }

  return STATE_G2_PLAY;
}


// ---------- GAME 3 ----------
SystemState updateG3Menu(const Controls& in) {
  bool b1Edge = (in.b1 && !lastB1);
  bool b2Edge = (in.b2 && !lastB2);
  lastB1 = in.b1;
  lastB2 = in.b2;

  if (b2Edge) {
    drawMainMenu();
    return STATE_MENU_MAIN;
  }

  if (b1Edge) {
    drawG3PlayScreen();
    delay(10);
    return STATE_G3_PLAY;
  }

  return STATE_G3_MENU;
}

SystemState updateG3Play(const Controls& in) {
  (void)in;
  drawG3PlayScreen();
  delay(3000);
  drawG3MenuScreen();
  return STATE_G3_MENU;
}

// =========================================================
// Arduino Entry Points
// =========================================================
void setup() {
  Serial.begin(115200);
  delay(2000);

  configureInputs();
  configureScreen();

  // Initialize button edge tracking
  Controls in = readInputs();
  lastB1 = (digitalRead(BUTTON1_PIN) == LOW);
  lastB2 = (digitalRead(BUTTON2_PIN) == LOW);
  lastB3 = (digitalRead(BUTTON3_PIN) == LOW);

  drawBootScreen();
  delay(4000);

  currentState = STATE_MENU_MAIN;
  drawMainMenu();

  Serial.println("setup done, in MENU");
}

void loop() {
  Controls in = readInputs();

  Serial.print("x=");
  Serial.print(in.x);
  Serial.print(" y=");
  Serial.println(in.y);

  switch (currentState) {
    case STATE_MENU_MAIN:
      Serial.println("loop: calling updateMainMenu");
      currentState = updateMainMenu(in);
      break;

    case STATE_G1_MENU:
      currentState = updateG1Menu(in);
      break;

    case STATE_G1_PLAY:
      currentState = updateG1Play(in);
      break;

    case STATE_G2_MENU:
      currentState = updateG2Menu(in);
      break;

    case STATE_G2_PLAY:
      currentState = updateG2Play(in);
      break;

    case STATE_G3_MENU:
      currentState = updateG3Menu(in);
      break;

    case STATE_G3_PLAY:
      currentState = updateG3Play(in);
      break;

    default:
      break;
  }
}
