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
static struct pt pt_asteroid, pt_asteroid2, pt_starship;  // each protothread needs one of these
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
  int health;
};

struct character *asteroid;
struct character *asteroid2;
struct character *starship;
struct character *heart;

void init_alloc_characters() {
  asteroid = malloc(sizeof(character));
  if(asteroid==NULL) {
    free(asteroid);
    Serial.println("Critical Error. Not enough memory.");
    exit(1);
  }
  asteroid2 = malloc(sizeof(character));
  if(asteroid2==NULL) {
    free(asteroid2);
    Serial.println("Critical Error. Not enough memory.");
    exit(1);
  }
  starship = malloc(sizeof(character));
  if(asteroid==NULL) {
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
  //Serial.println(random(1000));
}
void setup() {
  Serial.begin(9600);
  Serial.println("Initialisation...");

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
  lcd.createChar(asteroid->indexbyte, asteroidbyte);
  lcd.createChar(starship->indexbyte, starshipbyte);
  lcd.setCursor(0, 0);


  // ProtoThreads
  PT_INIT(&pt_asteroid);
  PT_INIT(&pt_starship);


  // Joystick
  attachInterrupt(digitalPinToInterrupt(joyClickpin), click_pressed, FALLING);
}
void init_asteroids() {
  """random generation of asteroids""";
  asteroid->b = asteroidbyte;
  asteroid->indexbyte = 1;
  asteroid->x = 20;
  asteroid->y = 0;
  asteroid->delay = 400;
  //asteroid2
  asteroid2->b = asteroidbyte;
  asteroid2->indexbyte = 1;
  asteroid2->x = 20;
  asteroid2->y = 1;
  asteroid2->delay = 600;
  //
  starship->b = starshipbyte;
  starship->x = 0;
  starship->y = 1;
  starship->delay = 2000;
  starship->indexbyte = 0;
  starship->health = 3;
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
  // outside
  if(posX < 0 || posX > 19 || posY < 0 || posY > 3) {
    lcd.setCursor(posX, posY);
    lcd.write(" ");
    free(c);
    Serial.println("Freed asteroid.");
    Serial.println(asteroid2->x);
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
  lcd.setCursor(c->x, c->y);
  lcd.write(" ");
  c->x -= 1;
  bool t = collision_check(c, c->x, c->y);
  if(!t){
    lcd.setCursor((c->x), (c->y));
    //print_coords(c->x, c->y);
    lcd.write(byte(c->indexbyte));
  }
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
static int protothreadasteroid(struct pt *pt, int delay) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > delay);
    timestamp = millis();
    move_asteroid(asteroid);
  }
  PT_END(pt);
}

static int protothreadasteroid2(struct pt *pt, int delay) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > delay);
    timestamp = millis();
    move_asteroid(asteroid2);
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
  protothreadasteroid(&pt_asteroid, asteroid->delay);
  protothreadasteroid2(&pt_asteroid2, asteroid2->delay);
  //rotothreadstarship(&pt_starship);
}