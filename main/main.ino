//
//  $$$$$$\              $$\                                   $$\       $$\        $$$$$$\
// $$  __$$\             $$ |                                  \__|      $$ |      $$  __$$\
// $$ /  $$ | $$$$$$$\ $$$$$$\    $$$$$$\   $$$$$$\   $$$$$$\  $$\  $$$$$$$ |      $$ /  \__| $$$$$$\  $$$$$$\$$$$\   $$$$$$\
// $$$$$$$$ |$$  _____|\_$$  _|  $$  __$$\ $$  __$$\ $$  __$$\ $$ |$$  __$$ |      $$ |$$$$\  \____$$\ $$  _$$  _$$\ $$  __$$\
// $$  __$$ |\$$$$$$\    $$ |    $$$$$$$$ |$$ |  \__|$$ /  $$ |$$ |$$ /  $$ |      $$ |\_$$ | $$$$$$$ |$$ / $$ / $$ |$$$$$$$$ |
// $$ |  $$ | \____$$\   $$ |$$\ $$   ____|$$ |      $$ |  $$ |$$ |$$ |  $$ |      $$ |  $$ |$$  __$$ |$$ | $$ | $$ |$$   ____|
// $$ |  $$ |$$$$$$$  |  \$$$$  |\$$$$$$$\ $$ |      \$$$$$$  |$$ |\$$$$$$$ |      \$$$$$$  |\$$$$$$$ |$$ | $$ | $$ |\$$$$$$$\
// \__|  \__|\_______/    \____/  \_______|\__|       \______/ \__| \_______|       \______/  \_______|\__| \__| \__| \_______|
//

// Compile : arduino-cli compile --fqbn arduino:samd:mkr1000 main
// Upload : arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:samd:mkr1000 main

// INCLUDES
#include <pt.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(2,3,4,5,6,7);

const int nb_asteroids = 3;
static struct pt pt_starship;  // each protothread needs one of these
static struct pt pt_asteroids[nb_asteroids];

//JOYSTICK VARIABLES
const char joyXpin = A0;
const char joyYpin = A1;
const char joyClickpin = 18;
uint16_t joyX;
uint16_t joyY;
bool joyClick;

byte asteroidbyte[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b11101,
  0b11011,
  0b11111,
  0b01110,
  0b00000
};
byte starshipbyte[8] = {
  0b00000,
  0b11000,
  0b01100,
  0b01100,
  0b11110,
  0b01100,
  0b01100,
  0b11000
};
// INITIALISATION
struct character {
  byte b;
  int x;
  int y;
  int indexbyte;
  int delay;
};
struct character *asteroids[nb_asteroids];
struct character *starship;

void init_alloc_asteroids() {
  for(int i=0;i<nb_asteroids;i++) {
    asteroids[i] = (character*) malloc(sizeof(character));
    if(asteroids[i]==NULL) {
      free(asteroids[i]);
      Serial.print("Critical Error. Not enough memory for asteroid #");
      exit(1);
    }
  }
}

void init_alloc_characters() {
  starship = (character *) malloc(sizeof(character));
  if(starship==NULL) {
    free(starship);
    Serial.println("Critical Error. Not Enough memory.");
    exit(1);
  }
}

int buffer[4][20];
void init_buffer() {
  for(int i=0; i<4; i++) {
    for(int j=0; j<20; j++) {
      buffer[i][j] = 9;
    }
  }
}

void init_asteroids() {
  Serial.println("Init asteroids");
  for(int i=0;i<nb_asteroids;i++) {
    asteroids[i]->b = *asteroidbyte;
    asteroids[i]->indexbyte = 1;
    asteroids[i]->x = random(10, 20);
    asteroids[i]->y = random(4);
    asteroids[i]->delay = random(400, 800);
   }
}
void init_characters() {
  """Initialisation of characters, except asteroids (see init_characters())""";
  // Starship
  starship->b = *starshipbyte;
  starship->x = 0;
  starship->y = 1;
  starship->delay = 2000;
  starship->indexbyte = 0;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Initialisation...");
  
  // Initialisation
  init_alloc_asteroids();
  init_alloc_characters();
  init_buffer();

  // Characters
  init_asteroids();
  init_characters();

  // LCD
  lcd.begin(20, 4);
  randomSeed(millis());
  // take the 0th asteroid but they all have the same indexbyte
  lcd.createChar(asteroids[0]->indexbyte, asteroidbyte); 
  lcd.createChar(starship->indexbyte, starshipbyte);
  lcd.setCursor(0, 0);


  // ProtoThreads
  for(int i=0;i<nb_asteroids;i++) {
    PT_INIT(&pt_asteroids[i]);
  }
  PT_INIT(&pt_starship);
}

void die() {
  Serial.println("You're dead.");
  exit(0);
}
void heal(int hearts) {
  Serial.println("Healed");
}

void destroy(int x, int y) {
  buffer[y][x] = 9;
}

void destroy_and_init_single_asteroid(struct character *c) {
  c->x = 20;
  c->y = random(4);
  c->delay = random(400,800);
}
void destroy_buffer(struct character *c) {
  buffer[c->y][c->x] = 9;
}
void set_buffer(int indexbyte, int x, int y) {
  lcd.setCursor(x,y);
  buffer[y][x] = indexbyte;
}

bool collision_check(struct character *c, int posX, int posY) {
  /* posX and posY are the next positions (where the character goes)
     The previous is already blank */
  // outside
  if(posX < 0 || posX > 19 || posY < 0 || posY > 3) {
    if(c->indexbyte == asteroids[0]->indexbyte) {
      destroy_and_init_single_asteroid(c);
    }
    return true;
  }
  return false;
}
void move_asteroid(struct character *c) {
  // Erase old character
  lcd.setCursor(c->x, c->y);
  lcd.write(" ");
  // Move the asteroid
  c->x -= 1;
  // Check for collisions with the new positions
  bool t = collision_check(c, c->x, c->y);
  // No collisions, so we write the character at new pos
  if(!t) {
    lcd.setCursor((c->x), (c->y));
    lcd.write(byte(c->indexbyte));
  }
}

void move_starship(uint16_t *pos) {
  // BAS
  if(*pos < 200) {
    if(!collision_check(starship, starship->x, (starship->y)+1)) {
      (starship->y)++;
      // write starship en x, y+1
      lcd.setCursor(starship->x, starship->y);
      lcd.write(byte(starship->indexbyte));
    };
  }
  // HAUT
  if(*pos > 820) {
    if(!collision_check(starship, starship->x, (starship->y)-1)) {
      (starship->y)--;
      // write starship en x, y-1
      lcd.setCursor(starship->x, starship->y);
      lcd.write(byte(starship->indexbyte));
    };
  }
}
static int protothreadasteroids1(struct pt *pt, int delay) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > delay);
    timestamp = millis();
    move_asteroid(asteroids[0]);
  }
  PT_END(pt);
}

static int protothreadasteroids2(struct pt *pt, int delay) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > delay);
    timestamp = millis();
    move_asteroid(asteroids[1]);
  }
  PT_END(pt);
}

static int protothreadasteroids3(struct pt *pt, int delay) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > delay);
    timestamp = millis();
    move_asteroid(asteroids[2]);
  }
  PT_END(pt);
}

void loop() {
  protothreadasteroids1(&(pt_asteroids[0]), asteroids[0]->delay);
  protothreadasteroids2(&(pt_asteroids[1]), asteroids[1]->delay);
  protothreadasteroids3(&(pt_asteroids[2]), asteroids[2]->delay);
}
