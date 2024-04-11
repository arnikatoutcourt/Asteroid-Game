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

// INCLUDES
#include <pt.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(2,3,4,5,6,7);

const int nb_asteroids = 3;
static struct pt pt_starship;  // each protothread needs one of these
static struct pt pt_asteroids[nb_asteroids];
static struct pt pt_print;
int buffer[4][20];

// Const
const char initstarshipX = 0;
const char initstarshipY = 1;
const char min_asteroid_x = 15;
const unsigned char min_asteroid_delay = 400;
const unsigned char max_asteroid_delay = 800;
const unsigned char starship_delay = 180;
// chars
const char starship_indexbyte = 0;
const char asteroid_indexbyte = 1;
const char asteroiddead_indexbyte = 6;
const char starshipdead_indexbyte = 7;

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
byte starship_deadbyte[8] = {
  0b00100,
  0b00100,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};
byte asteroid_deadbyte[8] = {
  0b01000,
  0b01010,
  0b10110,
  0b00101,
  0b01001,
  0b00110,
  0b01010,
  0b10000
};
// INITIALISATION
struct character {
  byte b;
  byte deadbyte;
  int x;
  int y;
  int index_byte;
  int index_deadbyte;
  int delay;
};
struct character *asteroids[nb_asteroids];
struct character *starship;

void init_alloc_asteroids() {
  for(int i=0;i<nb_asteroids;i++) {
    asteroids[i] = (character*) malloc(sizeof(character));
    if(asteroids[i]==NULL) {
      free(asteroids[i]);
      Serial.print("Critical Error. Not enough memory for new asteroid.");
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

void clear_buffer() {
  for(int i=0; i<4; i++) {
    for(int j=0; j<20; j++) {
      buffer[i][j] = 9;
    }
  }
}

void print_buffer() {
  for(int i=0;i<4;i++) {
    for(int j=0;j<20;j++) {
      if(buffer[i][j] == 9) {
        Serial.print(" ");
      } else {
        Serial.print(buffer[i][j]);
      }
      Serial.print(".");
    }
    Serial.println("");
  }
  Serial.println(millis());
}

void update_buffer(int posX, int posY, int indexbyte) {
  buffer[posY][posX] = indexbyte;
}

void init_asteroids() {
  for(int i=0;i<nb_asteroids;i++) {
    asteroids[i]->b = *asteroidbyte;
    asteroids[i]->deadbyte = *asteroid_deadbyte;
    asteroids[i]->index_byte = asteroid_indexbyte;
    asteroids[i]->index_deadbyte = asteroiddead_indexbyte;
    asteroids[i]->x = random(min_asteroid_x, 20);
    asteroids[i]->y = random(4);
    asteroids[i]->delay = random(min_asteroid_delay, max_asteroid_delay);
    update_buffer(asteroids[i]->x, asteroids[i]->y, asteroids[i]->index_byte);
   }
}

void init_characters() {
  """Initialisation of characters, except asteroids (see init_characters()). """;
  """Prints the starship in the screen.""";
  // Starship
  starship->b = *starshipbyte;
  starship->deadbyte = *starship_deadbyte;
  starship->x = initstarshipX;
  starship->y = initstarshipY;
  starship->delay = starship_delay;
  starship->index_byte = starship_indexbyte;
  starship->index_deadbyte = starshipdead_indexbyte;
  update_buffer(starship->x, starship->y, starship->index_byte);
}

void init_display_starship() {
  // Display starship
  init_characters();
  lcd.setCursor(starship->x,starship->y);
  lcd.write(byte(starship->index_byte));
}

void click_pressed() {
  Serial.println("SHOOOOOOT.");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Initialisation...");

  randomSeed(analogRead(A2)); 
  // Initialisation
  init_alloc_asteroids();
  init_alloc_characters();
  clear_buffer(); // init

  // Characters
  init_asteroids();
  init_characters();

  // LCD
  lcd.begin(20, 4);
  lcd.createChar(asteroid_indexbyte, asteroidbyte); 
  lcd.createChar(starship_indexbyte, starshipbyte);
  lcd.createChar(starshipdead_indexbyte, starship_deadbyte);
  lcd.createChar(asteroiddead_indexbyte, asteroid_deadbyte);
  lcd.setCursor(0,0);

  // Display starship at start position
  init_display_starship();

  // Joystick
  pinMode(joyXpin, INPUT);
  pinMode(joyYpin, INPUT);
  pinMode(joyClickpin, INPUT);
  attachInterrupt(digitalPinToInterrupt(joyClickpin), click_pressed, FALLING);

  // ProtoThreads
  for(int i=0;i<nb_asteroids;i++) {
    PT_INIT(&pt_asteroids[i]);
  }
  PT_INIT(&pt_starship);
  PT_INIT(&pt_print);
}

void respawn() {
  clear_buffer();
  init_display_starship();
  init_asteroids();
}

void die(int posY) { // ! hardcode
  Serial.println("You're dead.");
  // destroy old starship
  lcd.setCursor(starship->x, starship->y);
  lcd.write(" ");
  // update and display dead starship
  update_buffer(starship->x, posY, 7);
  lcd.setCursor(starship->x, posY);
  lcd.write(char(7));
  // wait for a sec
  delay(1000);
  // erase deadstarship
  update_buffer(starship->x, posY, 9);
  // write Game Over
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.write("Game Over");
  // wait for a sec
  delay(1000);
  lcd.clear();
  respawn();
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

bool is_asteroid(struct character *c) {
  return c->index_byte == asteroids[0]->index_byte;
}

bool is_starship(struct character *c) {
  return c->index_byte == starship->index_byte;
}

void create_heart(int posX, int posY) {
  Serial.println("Created heart.");
}

void destroy_heart(struct character *c) {
  Serial.println("Destroyed heart.");
}

void create_mun(int posX, int posY) {
  Serial.println("Created munition.");
}

bool collision_check(struct character *c, int posX, int posY) {
  /* posX and posY are the next positions (where the character goes)
     The previous is already blank */
  char charact = buffer[posY][posX];
  if(charact != 9) {
    // Asteroid
    if(is_asteroid(c)) {
      // Starship
      if(charact == starship->index_byte) {
        die(posY);
        //destroy_and_init_single_asteroid(c);
      }
      // Asteroid
      /*else if(charact == asteroids[0]->indexbyte) {
        create_heart(posX, posY);
        destroy_and_init_single_asteroid(c);
      }*/
    }
    // Starship
    if(is_starship(c)) {
      // Starship
      if(charact == starship->index_byte) {
        Serial.println("Critical error. Multiple instances of starship detected.");
        exit(1);
      }
      // Asteroid
      else if(charact == asteroids[0]->index_byte) {
        die(posY);
      }
    }
  }
  // outside
  if(posX < 0 || posX > 19 || posY < 0 || posY > 3) {
    // if it's an asteroid
    if(is_asteroid(c)) {
      // respawn it
      destroy_and_init_single_asteroid(c);
    }
    return true;
  }
  return false;
}
void move_asteroid(struct character *c) {
  // Erase old character
  update_buffer(c->x, c->y, 9);
  lcd.setCursor(c->x, c->y);
  lcd.write(" ");
  // Move the asteroid
  (c->x)--;
  // No collisions, so we write the character at new pos
  if(!collision_check(c, c->x, c->y)) {
    update_buffer(c->x, c->y, c->index_byte);
    lcd.setCursor((c->x), (c->y));
    lcd.write(byte(c->index_byte));
  }
}

void move_starship(uint16_t *pos) {
  // Assume that we can move
  // HAUT
  if(*pos > 820) {
    if(!collision_check(starship, starship->x, (starship->y)+1)) {
      update_buffer(starship->x, starship->y, 9);
      lcd.setCursor(starship->x, starship->y);
      lcd.write(" ");
      (starship->y)++;
      // write starship en x, y+1
      update_buffer(starship->x, starship->y, starship->index_byte);
      lcd.setCursor(starship->x, starship->y);
      lcd.write(byte(starship->index_byte));
    }
  }
  // BAS
  if(*pos < 200) {
    if(!collision_check(starship, starship->x, (starship->y)-1)) {
      update_buffer(starship->x, starship->y, 9);
      lcd.setCursor(starship->x, starship->y);
      lcd.write(" ");
      (starship->y)--;
      // write starship en x, y-1
      update_buffer(starship->x, starship->y, starship->index_byte);
      lcd.setCursor(starship->x, starship->y);
      lcd.write(byte(starship->index_byte));
    }
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

static int protothreadprint(struct pt *pt, int delay) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > delay);
    timestamp = millis();
    print_buffer();
  }
  PT_END(pt);
}

static int protothreadstarship(struct pt *pt, uint16_t *joyX, int delay) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, (millis() - timestamp > delay));
    timestamp = millis(); //100 = délai entre chaque mouvement du vaisseau
    move_starship(joyX);
  }
  PT_END(pt);
}

void loop() {
  protothreadasteroids1(&(pt_asteroids[0]), asteroids[0]->delay);
  protothreadasteroids2(&(pt_asteroids[1]), asteroids[1]->delay);
  protothreadasteroids3(&(pt_asteroids[2]), asteroids[2]->delay);
  protothreadprint(&pt_print, 250);
  joyX = analogRead(joyXpin);
  if(joyX < 200 || joyX > 820) {
    protothreadstarship(&pt_starship, &joyX, starship->delay);
  }
  
}
