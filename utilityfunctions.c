#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <stdlib.h>
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"

int nofOrbs = 170;

void moveGhost(x, y){
  int i;
  // row 1
  for (i = 1; i < 4; i++) display2d[y][x + i] = 1;
  // row 2 and 5
  for (i = 0; i < 5; i += 2) {
    display2d[y + 1][x + i] = 1;
    display2d[y + 4][x + i] = 1;
    }

  // row 3 and 4
  for (i = 0; i < 5; i++){
    display2d[y + 2][x + i] = 1;
    display2d[y + 3][x + i] = 1;
    }
}

// display 5x5 pacman
void movePacman(int x, int y, char dir){
  int i;

  if(dir == 'e'){
  for (i = 1; i < 5; i++) {
    display2d[y][x + i] = 1; // moves row 1
    display2d[y + 4][x + i] = 1; // moves row 5
    }
  for (i = 0; i < 4; i++){ 
    display2d[y + 1][x + i] = 1; // moves row 2
    display2d[y + 3][x + i] = 1; // moves row 4
  }
  for (i = 0; i < 3; i++) display2d[y + 2][x + i] = 1; // moves row 3
  }
  else if(dir == 'w')
  {
  for (i = 0; i < 4; i++) {
    display2d[y][x + i] = 1; // moves row 1
    display2d[y + 4][x + i] = 1; // moves row 5
    }
  for (i = 1; i < 5; i++){ 
    display2d[y + 1][x + i] = 1; // moves row 2
    display2d[y + 3][x + i] = 1; // moves row 4
  }
  for (i = 2; i < 5; i++) display2d[y + 2][x + i] = 1; // moves row 3
  }
  else if(dir == 'n')
  {
  for (i = 0; i < 4; i++) {
    display2d[y + i][x] = 1; // moves row 1
    display2d[y + i][x + 4] = 1; // moves row 5
    }
  for (i = 1; i < 5; i++){ 
    display2d[y + i][x + 1] = 1; // moves row 2
    display2d[y + i][x + 3] = 1; // moves row 4
  }
  for (i = 2; i < 5; i++) display2d[y + i][x + 2] = 1; // moves row 3
  }
  else if(dir == 's')
  {
  for (i = 1; i < 5; i++) {
    display2d[y + i][x] = 1; // moves row 1
    display2d[y + i][x + 4] = 1; // moves row 5
    }
  for (i = 0; i < 4; i++){ 
    display2d[y + i][x + 1] = 1; // moves row 2
    display2d[y + i][x + 3] = 1; // moves row 4
  }
  for (i = 0; i < 3; i++) display2d[y + i][x + 2] = 1; // moves row 3
  }
}

void labinit( void ){
  TRISD = TRISD & 0xf01f;
  TRISD += 0xfe0;
  TRISF = TRISF | (0x20);
  PORTE = 0x7;
  T2CONSET = 0 << 15; // turn timer on
  TMR2 = 0; // set timer to 0
  T2CONSET = 0x70; // set the prescaling to 1:256 
  PR2 = 31250/4; // set period to 25ms;
  IFSCLR(0) = 1 << 8; // set timer interupt flag to 0
  T2CONSET = 1 << 15; // start the timer
  IECSET(0) = (1 << 8); // timer 2 interrupt enable to 1 
  IPCSET(2) = 0x1f; // set timer2 interrupt priority and sub-priority control to 1 

  AD1PCFG = 0xFBFF;
  AD1CON1 = 4 << 8 | 7 << 5;
  AD1CHS = 1 << 17;
  AD1CON2 = 0;
  AD1CON3SET = 1 << 15;

  /* Set up output pins */

  /* Turn on ADC */
  AD1CON1SET = 1 << 15;

  enable_interrupt();
  score = 0;

  clearDisplay();
  display_image(0, display);
}


void display2dToArray() {
  int page, c, r;
  uint8_t powerOfTwo = 1;
  uint8_t bitN = 0;

  for(page = 0; page < 4; page++) {
    for(c = 0; c < 128; c++) {
      powerOfTwo = 1;
      bitN = 0;

      for(r = 0; r < 8; r++) {
        if(display2d[8 * page + r][c]) {
          bitN |= powerOfTwo;
        }
        powerOfTwo <<= 1;
      }
      display[c + page * 128] = bitN;
    }
  }
}

// add walls to display
void addWallsAndOrbs(){
int i;
  for (i = 0; i < 512; i++) {
    display[i] = display[i] | walls[i] | orbs[i];
  }
}

// check if entity will collide with wall
int checkCollisionWithWall(char dir, entity *ent){
  int i;
  uint8_t collisionBool = 0;

  for(i = 0; i < 4; i ++){
    if(dir == 'w'){
      collisionBool = collisionBool | walls2d[ent->y + i][ent->x - 1];
    }
    else if(dir == 's'){
      collisionBool = collisionBool | walls2d[ent->y + 5][ent->x + i];
    }
    else if(dir == 'n'){
      collisionBool = collisionBool | walls2d[ent->y - 1][ent->x + i];
    }
    else {
      collisionBool = collisionBool | walls2d[ent->y + i][ent->x + 5];
    }
  }
  return collisionBool;
}

// check if pacman is colliding with wall
void checkCollisionWithOrb(entity *ent){
  int i, j;
  uint8_t collisionBool = 0;

  for(i = 0; i < 5; i++){
    for(j = 0; j < 5; j++){
      if (orbs2d[ent->y + i][ent->x + j] == 1){
        collisionBool = 1;
        orbs2d[ent->y + i][ent->x + j] = 0;
      }
    }
  }
  if(collisionBool == 1) {
    orbs2dToArray();
    nofOrbs--;
  }
}

// check if pacman is colliding with any ghost
void checkCollisionWithGhost(entity *pacman, entity *ghost){
  int i, xDifference, yDifference;
  int collisionBool = 0;
  xDifference = (ghost->x) - (pacman->x);
  yDifference = (ghost->y) - (pacman->y);

  if((xDifference <= 5) && (xDifference >= -5 ) && (yDifference <= 5) && (yDifference >= -5)) collisionBool = 1; 

  if(collisionBool == 1){
    pacman->lives = pacman->lives - 1;
    PORTE = PORTE >> 1;
    if(pacman->lives == 0){
      gameState = 4;
    }
    else {
      // move pacman and ghost to starting positions
        ghost->x = 60;
        ghost->y = 8;
        ghost->dir = 'n';

        pacman->x = 1;
        pacman->y = 1;
        pacman->dir = 'e';
    }
  }
}

// remake orbs array
void orbs2dToArray() {
  int page, c, r;
  uint8_t powerOfTwo = 1;
  uint8_t bitN = 0;

  for(page = 0; page < 4; page++) {
    for(c = 0; c < 128; c++) {
      powerOfTwo = 1;
      bitN = 0;

      for(r = 0; r < 8; r++) {
        if(orbs2d[8 * page + r][c]) {
          bitN |= powerOfTwo;
        }
        powerOfTwo <<= 1;
      }
      orbs[c + page * 128] = bitN;
    }
  }
}

void orbsRemap(){
  int r, c;
  for (r = 0; r < 32; r++){
    for (c = 0; c < 128; c++){
      orbs2d[r][c] = orbs2dConstant[r][c];
    }
  } 

  int page;
  uint8_t powerOfTwo = 1;
  uint8_t bitN = 0;

  for(page = 0; page < 4; page++) {
    for(c = 0; c < 128; c++) {
      powerOfTwo = 1;
      bitN = 0;

      for(r = 0; r < 8; r++) {
        if(orbs2dConstant[8 * page + r][c]) {
          bitN |= powerOfTwo;
        }
        powerOfTwo <<= 1;
      }
      orbs[c + page * 128] = bitN;
    }
  }
}

int calculateScore(int lives, int scoreClock){
  int s = lives * 100; // the less lives you lose, the better
  s += (120 - scoreClock / 500) * 20; // the faster you finish, the better

  return s;
}

void updatePacman(entity *pacman){
  // check if direction change
  if((btn4) && checkCollisionWithWall('w', pacman) == 0) pacman->dir = 'w';
  else if((btn3) && checkCollisionWithWall('n', pacman) == 0) pacman->dir = 'n';
  else if((btn2) && checkCollisionWithWall('s', pacman) == 0) pacman->dir = 's';
  else if((btn1) && checkCollisionWithWall('e', pacman) == 0) pacman->dir = 'e';

  // check if collision, then change coordinates
  if(checkCollisionWithWall(pacman->dir, pacman) == 0){
    if(pacman->dir == 'w') pacman->x = pacman->x - 1;
    else if(pacman->dir == 'n')pacman->y = pacman->y - 1;
    else if(pacman->dir == 's')pacman->y = pacman->y + 1;
    else if(pacman->dir == 'e')pacman->x = pacman->x + 1;
  }
}