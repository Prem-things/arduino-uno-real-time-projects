#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define JOY_X A0
#define JOY_Y A1

// Buzzer pin
#define BUZZER_PIN 6

// Snake variables
#define MAX_LENGTH 100
int snakeX[MAX_LENGTH];
int snakeY[MAX_LENGTH];
int snakeLength = 5;
int dir = 0; // 0 = right, 1 = down, 2 = left, 3 = up

// Food position
int foodX, foodY;

bool gameOver = false;
int score = 0;
int highScore = 0;
bool newHighScore = false;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  randomSeed(analogRead(A2));
  display.clearDisplay();
  display.display();

  pinMode(BUZZER_PIN, OUTPUT);
  playStartSound();

  highScore = EEPROM.read(0);
  if (highScore == 255) {
    highScore = 0;
    EEPROM.write(0, 0);
  }

  initGame();
}

void loop() {
  if (!gameOver) {
    readJoystick();
    moveSnake();
    checkCollision();
    drawGame();
    delay(120);
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 15);
    display.print("Game Over!");
    display.setCursor(10, 30);
    display.print("Your Score: ");
    display.print(score);
    display.setCursor(10, 42);
    display.print("High Score: ");
    display.print(highScore);
    display.display();
    delay(1500);

    if (newHighScore) {
      playHighScoreSound();
      display.clearDisplay();
      display.setCursor(10, 25);
      display.setTextSize(1);
      display.print("Woow! You made a");
      display.setCursor(10, 38);
      display.print("New High Score!");
      display.display();
      delay(1500);
    }

    initGame();
  }
}

void initGame() {
  snakeLength = 5;
  dir = 0;
  gameOver = false;
  score = 0;
  newHighScore = false;

  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = 10 - i;
    snakeY[i] = 10;
  }

  spawnFood();
}

void readJoystick() {
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  // Map directions:
  // 0 = right, 1 = down, 2 = left, 3 = up
  if (x > 800 && dir != 0) dir = 2;     // Left
  else if (x < 200 && dir != 2) dir = 0; // Right
  else if (y > 800 && dir != 3) dir = 1; // Down
  else if (y < 200 && dir != 1) dir = 3; // Up
}

void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  switch (dir) {
    case 0: snakeX[0]++; break; // Right
    case 1: snakeY[0]++; break; // Down
    case 2: snakeX[0]--; break; // Left
    case 3: snakeY[0]--; break; // Up
  }
}

void checkCollision() {
  if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH / 4 || snakeY[0] < 0 || snakeY[0] >= (SCREEN_HEIGHT - 8) / 4) {
    endGame();
    playGameOverSound();
    return;
  }

  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      endGame();
      playGameOverSound();
      return;
    }
  }

  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    if (snakeLength < MAX_LENGTH) snakeLength++;
    score++;
    spawnFood();
    playEatSound();
  }
}

void endGame() {
  if (score > highScore) {
    EEPROM.write(0, score);
    highScore = score;
    newHighScore = true;
  }
  gameOver = true;
}

void spawnFood() {
  bool valid;
  do {
    valid = true;
    foodX = random(0, SCREEN_WIDTH / 4);
    foodY = random(0, (SCREEN_HEIGHT - 8) / 4);

    for (int i = 0; i < snakeLength; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        valid = false;
        break;
      }
    }
  } while (!valid);
}

void drawGame() {
  display.clearDisplay();

  // Draw food
  display.fillRect(foodX * 4, foodY * 4, 4, 4, SSD1306_WHITE);

  // Draw snake
  for (int i = 0; i < snakeLength; i++) {
    display.fillRect(snakeX[i] * 4, snakeY[i] * 4, 4, 4, SSD1306_WHITE);
  }

  // Draw score bar
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print("Score:");
  display.print(score);

  display.setCursor(75, 56);
  display.print("High:");
  display.print(highScore);

  display.display();
}

// Sound functions
void playStartSound() {
  tone(BUZZER_PIN, 1000, 100);
  delay(150);
  tone(BUZZER_PIN, 1500, 100);
  delay(150);
  noTone(BUZZER_PIN);
}

void playEatSound() {
  tone(BUZZER_PIN, 1200, 80);
  delay(100);
  noTone(BUZZER_PIN);
}

void playGameOverSound() {
  tone(BUZZER_PIN, 400, 300);
  delay(300);
  noTone(BUZZER_PIN);
}

void playHighScoreSound() {
  tone(BUZZER_PIN, 1000, 150);
  delay(200);
  tone(BUZZER_PIN, 1400, 150);
  delay(200);
  tone(BUZZER_PIN, 1800, 150);
  delay(200);
  noTone(BUZZER_PIN);
}
