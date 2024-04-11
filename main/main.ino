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

//JOYSTICK VARIABLES
const char joyXpin = A0;
const char joyYpin = A1;
const char joyClickpin = 18;
uint16_t joyX;
uint16_t joyY;
bool joyClick;

// UTILS
void print_coords(int x, int y) {
  Serial.print("(");
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(")");
}

static struct pt pt_starship;  // each protothread needs one of these
const int nb_asteroids = 20;
static struct pt pt_asteroids[nb_asteroids];

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

void malloc_asteroids() {
  for(int i=0;i<nb_asteroids;i++) {
    asteroids[i] = malloc(sizeof(character));
    if(asteroids[i]==NULL) {
      free(asteroids[i]);
      Serial.print("Critical Error. Not enough memory for asteroid #");
      Serial.println(i);
      return 1;
    }
  }
}

int random_y() {
  int t = random(0,3);
  Serial.print("Random to : ");
  Serial.println(t);
  return t;
}

void init_asteroids() {
  for(int i=0;i<nb_asteroids;i++) {
    asteroids[i]->b = asteroidbyte;
    asteroids[i]->indexbyte = 1;
    asteroids[i]->x = 20;
    asteroids[i]->y = random_y();
    asteroids[i]->delay = random_value(400,600);
  }
}

void init_alloc_characters() {
  starship = malloc(sizeof(character));
  if(starship==NULL) {
    free(starship);
    Serial.println("Critical Error. Not Enough memory.");
    exit(1);
  }
  heart = malloc(sizeof(character));
  if(heart==NULL) {
    free(heart);
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
// JOYSTICK
void click_pressed() {
}
void setup() {
  Serial.begin(9600);
  Serial.println("Initialisation...");
  randomSeed(100);

  // Pins
  pinMode(joyXpin, INPUT);
  pinMode(joyYpin, INPUT);
  pinMode(joyClickpin, INPUT);
  
  // Initialisation
  init_alloc_characters();
  init_buffer();
  init_asteroids();

  // LCD
  lcd.begin(20, 4);
  // take the 0th asteroid but they all have the same indexbyte
  lcd.createChar(asteroids[0]->indexbyte, asteroidbyte); 
  lcd.createChar(starship->indexbyte, starshipbyte);
  lcd.setCursor(0, 0);


  // ProtoThreads
  PT_INIT(&pt_asteroids[0]);
  PT_INIT(&pt_asteroids[1]);
  PT_INIT(&pt_starship);


  // Joystick
  attachInterrupt(digitalPinToInterrupt(joyClickpin), click_pressed, FALLING);
}
void init_characters() {
  """Initialisation of characters, except asteroids (see init_characters())""";
  // Starship
  starship->b = starshipbyte;
  starship->x = 0;
  starship->y = 1;
  starship->delay = 2000;
  starship->indexbyte = 0;
  starship->health = 3;
  // Heart
  //heart->b = heartbyte;
  // heart->x = 0;
  // heart->y = 0;
  // heart->delay = 2000;
  // heart->indexbyte = 1;
  // heart->health = 3;
}
void die() {
  Serial.println("You're dead.");
}
void heal(int hearts) {
  starship->health += hearts;
}
void destroy(int x, int y) {
  buffer[y][x] = 9;
}
void destroy(struct character *c) {
  buffer[c->y][c->x] = 9;
  // del struct
}
void set_buffer(int indexbyte, int x, int y) {
  lcd.setCursor(x,y);
  buffer[y][x] = indexbyte;
  lcd.write(byte(indexbyte));
}

bool collision_check(struct character *c, int posX, int posY) {
  /* posX and posY are the next positions (where the character goes)
     The previous is already blank */
  // outside
  if(posX < 0 || posX > 19 || posY < 0 || posY > 3) {
    lcd.setCursor(posX, posY);
    lcd.write(" ");
    free(c);
    Serial.println("Freed character.");
    return true;
  }
  // other
  /*int target = buffer[posY][posX];
  if(target != 9){
    if(c->indexbyte == starship->indexbyte)  {
      // starship
      if(target == asteroid->indexbyte) {
        die();
      }
      else if(target == heart->indexbyte) {
        heal(1);
      }
    }
    else if(c->indexbyte == asteroid->indexbyte) {
      // asteroid
      if(target == starship->indexbyte) {
        die();
      }
      else if(target == asteroid->indexbyte) {
        set_buffer(heart->indexbyte, posX, posY);
      }
    }
    else if(c->indexbyte == heart->indexbyte) {
      if(target == starship->indexbyte) {
        heal(1);
      }
      else if(target == asteroid->indexbyte) {
        //! DESTROY THE HEART
        destroy(posX, posY);
      }
    }
  }*/
  return false;
// IN THIS, DESTROY THE HEART THAT COLLIDED (STRUCT)
}
void move_asteroid(struct character *c) {
  // Erase old character
  lcd.setCursor(c->x, c->y);
  lcd.write(" ");
  // Move the asteroid
  c->x -= 1;
  // Check for collisions with the new positions
  bool t = collision_check(c, c->x, c->y);
  // If collisions, boum
  if(t) {
    Serial.println("Boum !");
    return;
  }
  // No collisions, so we write the character at new pos
  lcd.setCursor((c->x), (c->y));
  lcd.write(byte(c->indexbyte));
}
void move_starship(uint16_t *pos) {
  // BAS
  if(*pos < 200 && (starship->y < 3)) {
    collision_check(starship, starship->x, (starship->y)+1);
  }
  // HAUT
  if((*pos > 820) && (starship->y > 0)) {
    collision_check(starship, starship->x, (starship->y)-1);
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
static int protothreadstarship(struct pt *pt) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    joyX = analogRead(joyXpin);
    PT_WAIT_UNTIL(pt, (joyX >= 200 && joyX <= 820));
    move_starship(&joyX);
  }
  PT_END(pt);
}
void loop() {
  protothreadasteroids1(&(pt_asteroids[0]), asteroids[0]->delay);
  protothreadasteroids2(&(pt_asteroids[1]), asteroids[1]->delay);
  //Serial.print("PosX for 0: ");
  //Serial.println(asteroids[0]->x);
  protothreadstarship(&pt_starship);
}