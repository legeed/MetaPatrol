// ###########################################################################
//         
//
//         Title : MetaPatrol.ino
//         Author: Jean-Baptiste Chaffange Koch
//    Description: A moonpatrol-like game for Gamebuino Meta.
//        Version: 0bis
//           Date: 14 Sep 2018
//        License: GPLv3 (see LICENSE)
//
//    MetaPatrol Copyright (C) 2018 Jean-Baptiste Chaffange Koch
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    any later version.
//    
//    It's a modification of CosmicPods for Arduboy created by cubic9com 
//    <https://twitter.com/cubic9com>
//    <https://github.com/ArduboyCollection/CosmicPods<
//    Thanks to him !
//
//    
//###########################################################################


// ###########################################################################
//    VARIABLES
// ###########################################################################

#include <Gamebuino-Meta.h>

#define X_MAX 64
#define Y_MAX 80

#define NUM_PLAYER_BULLETS 4
#define MAX_NUM_ENEMIES 4
#define MAX_NUM_OBSTACLES 2
#define NUM_STARS 32

#define OBSTACLE_RECT {0, 0, 10, 10}
#define ENEMY_RECT {0, 0, 10, 10}
#define PLAYER_RECT {0, 1, 10, 10}
uint8_t STAR_COLORS [] = {1, 6, 7, 12, 13};

struct Rect {
  int16_t x;      
  int16_t y;      
  uint8_t w;      
  uint8_t h;      
};

struct Point {
  int16_t x;      
  int16_t y;      
};

struct Speed {
  int16_t dx;      
  int16_t dy; 
};

struct Vector2 {
  float x;
  float y;
};

struct FloatPoint {
  float x;
  float y;
};

struct PlayerBullet {
  Point point;
  Speed spd;
  boolean enabled;
};

struct Player {
  Rect rect;
  byte power;
  PlayerBullet bullets[NUM_PLAYER_BULLETS];
};

struct EnemyBullet {
  FloatPoint point;
  Vector2 delta;
  boolean enabled;
};

struct Enemy {
  Rect rect;
  EnemyBullet bullet;
  Speed spd;
  byte type;
};

struct Obstacle {
  Rect rect;
  byte type;
  byte state;
};

struct Star {
  Point point;
  byte spd;
  byte color;
};

struct Camera {
  Speed spd;
};

struct Player player;
struct Star stars[NUM_STARS];
struct Enemy enemies[MAX_NUM_ENEMIES];
struct Obstacle obstacles[MAX_NUM_OBSTACLES];

boolean is_gameover;
boolean is_title;
unsigned int score;
unsigned int best_score;
boolean is_highscore;
byte num_enemies;
byte num_obstacles;
byte bullet_speed_factor;
byte level;
byte WIDTH;
byte HEIGHT;
byte ymin = 40;
struct Camera camera;
const int XMap[80] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};
const int YMap[50] = {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0};


// FIN DES VARIABLES #########################################################
// ###########################################################################



// ###########################################################################
//    BITMAPS & SOUNDS
// ###########################################################################

const Gamebuino_Meta::Sound_FX sfx_shoot[] = {
  {Gamebuino_Meta::Sound_FX_Wave::SQUARE,0,255,-15,25,24,8},
};

const Gamebuino_Meta::Sound_FX sfx_explosion[] = {
  {Gamebuino_Meta::Sound_FX_Wave::SQUARE,1,180,25,35,100,2},
  {Gamebuino_Meta::Sound_FX_Wave::NOISE,1,200,0,19,112,9},
  {Gamebuino_Meta::Sound_FX_Wave::NOISE,0,90,0,20,0,16},
};

const Gamebuino_Meta::Sound_FX sfx_blop[] = {
  {Gamebuino_Meta::Sound_FX_Wave::SQUARE,0,255,30,30,100,13},
};

// FIN DES BITMAPS & SOUNDS ##################################################
// ###########################################################################



// ###########################################################################
//    SETUP
// ###########################################################################
void setup() {
    gb.begin();

    best_score = gb.save.get(0);

    WIDTH = gb.display.width();
    HEIGHT = gb.display.height();

    for (byte i = 0; i < MAX_NUM_ENEMIES; i++) {
      enemies[i].rect = ENEMY_RECT;
    }

    for (byte i = 0; i < MAX_NUM_OBSTACLES; i++) {
      obstacles[i].rect = OBSTACLE_RECT;
      obstacles[i].state = 0;
    }
  
    player.rect = PLAYER_RECT;
    player.power = 25;
  
    beginGame();
}
// FIN DU BEGIN ##############################################################
// ###########################################################################



// ###########################################################################
//    LOOP
// ###########################################################################
void loop() {
    
    while(!gb.update());
    
    gb.display.clear();
    gb.lights.clear();

     if (is_title) {
      moveStars();
      drawStars();
      displayTitle();
      if (gb.buttons.pressed(BUTTON_A) || gb.buttons.pressed(BUTTON_B)) {
        beginGame();
        is_title = false;
      }
       return;
     }
  
    if (is_gameover) {
       displayGameover();
       if (gb.buttons.pressed(BUTTON_A) || gb.buttons.pressed(BUTTON_B)) {
       beginGame();
       }
      return;
    }

    movePlayer();
    moveStars();
    moveEnemies();
    moveObstacles();
    movePlayerBullets();
    moveEnemiesBullet();
    
    drawStars();
    drawScene();
    
    drawPlayer();
    drawEnemies();
    drawObstacles();
    drawPlayerBullets();
    drawEnemiesBullet();

    drawScore();

    checkEnemyCollision();
    checkObstaclesCollision();
    checkPlayerCollision();
  
}

// FIN DU LOOP ###############################################################
// ###########################################################################



// ###########################################################################
//    FONCTIONS
// ###########################################################################

void beginGame() {
  
  score = 0;

  player.rect.x = 0;
  player.rect.y = 40;
  player.power = 25;

  for (byte i = 0; i < NUM_STARS; i++) {
    spawnStar(i);
  }

  for (byte i = 0; i < MAX_NUM_ENEMIES; i++) {
    spawnEnemy(i);
  }

  for (byte i = 0; i < MAX_NUM_OBSTACLES; i++) {
    spawnObstacles(i);
  }

  for (byte i = 0; i < NUM_PLAYER_BULLETS; i++) {
    player.bullets[i].enabled = false;
    player.bullets[i].point.x = 0;
    player.bullets[i].point.y = 0;
    player.bullets[i].spd.dx = 0;
    player.bullets[i].spd.dy = 0;
  }

  for (byte i = 0; i < MAX_NUM_ENEMIES; i++) {
    enemies[i].bullet.enabled = false;
    enemies[i].bullet.point.x = 0;
    enemies[i].bullet.point.y = 0;
  }

  is_gameover = false;
  is_title = true;
  is_highscore = false;

  shiftLevel();
}



void shiftLevel() {
  switch (score) {
    case 0:
      setLevel(1, 1, 1, 1.4);
      break;
    case 1:
      setLevel(2, 1, 1, 1.4);
      break;
    case 3:
      setLevel(3, 2, 1, 1.4);
      break;
    case 7:
      setLevel(4, 2, 1, 1.5);
      break;
    case 16:
      setLevel(5, 2, 2,  1.6);
      break;
    case 25:
      setLevel(6, 2, 2,  1.8);
      break;
    case 37:
      setLevel(7, 2, 2,  2.0);
      break;
    case 49:
      setLevel(8, 2, 2,  2.3);
      break;
    case 64:
      setLevel(9, 2, 2,  2.4);
      break;
    case 79:
      setLevel(10, 2, 2,  2.6);
      break;
    case 97:
      setLevel(11, 3, 2,  2.8);
      break;
    case 115:
      setLevel(12, 3, 2,  3.1);
      break;
    case 136:
      setLevel(13, 3, 2,  3.4);
      break;
    case 157:
      setLevel(14, MAX_NUM_ENEMIES, MAX_NUM_OBSTACLES,  3.7);
      break;
    case 181:
      setLevel(15, MAX_NUM_ENEMIES, MAX_NUM_OBSTACLES,  4.1);
      break;
    default:
      break;
  }
}


void setLevel(byte lv, byte ne, byte no, float spd) {
  level = lv;
  num_enemies = ne;
  num_obstacles = no;
  bullet_speed_factor = spd;
}

void spawnEnemy(byte i) {
  enemies[i].rect.y = -10 ;
  enemies[i].type = random(1,3);
  switch (enemies[i].type) {
    case 1:
      enemies[i].rect.x = random(12,69) ;
      enemies[i].spd.dx = -random(1,3);
      break;
    case 2:
      enemies[i].rect.x = random(12,69) ;
      enemies[i].spd.dx = -random(2,4);
      break;
  }
  enemies[i].spd.dy = 1;
}

void spawnObstacles(byte i) {
  obstacles[i].state = 1; //O : inactive , 1 : active
  obstacles[i].rect.x = WIDTH;
  obstacles[i].type = random(1,4);
  switch (obstacles[i].type ) {
    case 1:   //HOLE
      obstacles[i].rect.y = ymin + obstacles[i].rect.h - 1 ;
      break;
    case 2:   //ROCK
      obstacles[i].rect.y = ymin;
      break;   
    case 3:   //BONUS
      obstacles[i].rect.y = ymin;
      break;
   }
}

void spawnStar(byte i) {
  stars[i].point.x = random(WIDTH, WIDTH * 2);
  stars[i].point.y = random(0, HEIGHT);
  stars[i].spd = random(1, 5);
  stars[i].color = STAR_COLORS[random(0,4)];
}

void spawnPlayerBullet(byte i, byte dir) {
  player.bullets[i].enabled = true;
  player.bullets[i].point.x = player.rect.x+5;
  player.bullets[i].point.y = player.rect.y+5;
  switch (dir) {
    case 0:
      player.bullets[i].spd.dx = 0;
      player.bullets[i].spd.dy = 2;
      break;
    case 1:
      player.bullets[i].spd.dx = 2;
      player.bullets[i].spd.dy = 0;
      break;
    }
}

void spawnEnemyBullet(byte i) {
  enemies[i].bullet.enabled = true;
  enemies[i].bullet.point.x = enemies[i].rect.x+5;
  enemies[i].bullet.point.y = enemies[i].rect.y+5;

  Vector2 orig_delta;
  orig_delta.x = player.rect.x - enemies[i].rect.x;
  orig_delta.y = player.rect.y - enemies[i].rect.y;

  Vector2 new_delta = calcDelta(orig_delta);

  enemies[i].bullet.delta.x = new_delta.x;
  enemies[i].bullet.delta.y = new_delta.y;
}

Vector2 calcDelta(Vector2 v) {
  float mag = bullet_speed_factor / sqrt(v.x * v.x + v.y * v.y);

  Vector2 v2;
  v2.x = v.x * mag;
  v2.y = v.y * mag;

  return v2;
}

void movePlayer() {

  //player.power = min(player.power + 1 , 25);
  
  if (gb.buttons.repeat(BUTTON_RIGHT,1) && (player.rect.x < 70)) {
    player.rect.x++;
  }

  if (gb.buttons.repeat(BUTTON_LEFT,1) && (player.rect.x > 0)) {
    player.rect.x--;
  }

  if (gb.buttons.repeat(BUTTON_UP,1) && (player.rect.y > 0) && player.power > 0) {
    player.rect.y--;
    player.power = max(player.power - 1 , 0);
  } else {player.rect.y = min(ymin, player.rect.y + 1) ;}

  if (gb.buttons.repeat(BUTTON_DOWN,1) && (player.rect.y < ymin)) {
    player.rect.y++;
  }

  if (gb.buttons.repeat(BUTTON_A,12)) {
    for (byte i = 0; i < NUM_PLAYER_BULLETS; i++) {
      if (!player.bullets[i].enabled) {
        spawnPlayerBullet(i,0);
        gb.sound.fx(sfx_shoot);
        break;
      }
     }
  }

  if (gb.buttons.repeat(BUTTON_B,24)) {
    for (byte i = 0; i < NUM_PLAYER_BULLETS; i++) {
      if (!player.bullets[i].enabled) {
        spawnPlayerBullet(i,1);
        gb.sound.fx(sfx_shoot);
        break;
      }
     }
  }

   setupCamera(player.rect.x, player.rect.y);
    
}

void moveStars() {
  for (byte i = 0; i < NUM_STARS; i++) {
    stars[i].point.x -= stars[i].spd;
    if (stars[i].point.x < 0) {
      spawnStar(i);
    }
  }
}

void moveEnemies() {
  for (byte i = 0; i < num_enemies; i++) {
    
    enemies[i].rect.x =  enemies[i].rect.x - enemies[i].spd.dx;
    enemies[i].rect.y =  enemies[i].rect.y + enemies[i].spd.dy;

    if (
      (level > 2)
      && (enemies[i].rect.x > player.rect.x + 10)
      && (enemies[i].rect.x < WIDTH - 10)
      && (!enemies[i].bullet.enabled)
      && (random(0, 50) == 10)
    ) {
      spawnEnemyBullet(i);
      gb.sound.fx(sfx_blop);
    }

    if (enemies[i].rect.x < 1 || enemies[i].rect.x >= (WIDTH - enemies[i].rect.w)) {
      enemies[i].spd.dx = -enemies[i].spd.dx;
    }

    if (enemies[i].rect.y >= 10) {
      enemies[i].spd.dy = 0;
    }
  }
}

void moveObstacles() {
  for (byte i = 0; i < num_obstacles; i++) {
    obstacles[i].rect.x--;
    if (obstacles[i].rect.x <= 0) {
      spawnObstacles(i);
    }
  }
}

void movePlayerBullets() {
  for (byte i = 0; i < NUM_PLAYER_BULLETS; i++) {
    if (player.bullets[i].enabled) {
      player.bullets[i].point.x += player.bullets[i].spd.dx;
      player.bullets[i].point.y -= player.bullets[i].spd.dy;
      if (player.bullets[i].point.x > WIDTH || player.bullets[i].point.y < 0) {
        player.bullets[i].enabled = false;
      }
    }
  }
}

void moveEnemiesBullet() {
  for (byte i = 0; i < num_enemies; i++) {
    if (enemies[i].bullet.enabled) {
      enemies[i].bullet.point.x += enemies[i].bullet.delta.x;
      enemies[i].bullet.point.y += enemies[i].bullet.delta.y;
      if (
        (enemies[i].bullet.point.x <= 0)
        || (enemies[i].bullet.point.y <= 0)
        || (enemies[i].bullet.point.y >= HEIGHT)
      ) {
        enemies[i].bullet.enabled = false;
      }
    }
  }
}

boolean collidePointRect(Point test_point, Rect test_rect){
  if ( (test_point.x < test_rect.x) || (test_point.x > (test_rect.x + test_rect.w - 1)) || (test_point.y < test_rect.y) || (test_point.y > (test_rect.y + test_rect.h -1)) ) {
    return false;
  } else {
    return true;
  }
}

boolean collideRectRect(Rect test_rect_A, Rect test_rect_B){
     if((test_rect_B.x >= test_rect_A.x + test_rect_A.w)
        || (test_rect_B.x + test_rect_B.w <= test_rect_A.x) 
        || (test_rect_B.y >= test_rect_A.y + test_rect_A.h)
        || (test_rect_B.y + test_rect_B.h <= test_rect_A.y)) {
          return false; 
        } else {
          return true; 
        }
}

void checkEnemyCollision() {
  for (byte j = 0; j < num_enemies; j++) {
    boolean is_col = false;
    for (byte i = 0; i < NUM_PLAYER_BULLETS; i++) {
      if (player.bullets[i].enabled) {
        is_col = collidePointRect(player.bullets[i].point, enemies[j].rect);
        if (is_col) {
          player.bullets[i].enabled = false;
          break;
        }
      }
    }
    if (is_col) {
      player.power = min(player.power + 1, 25);
      score++;
      shiftLevel();
      spawnEnemy(j);
    }
  }
}

void checkObstaclesCollision() {
    for (byte j = 0; j < num_obstacles; j++) {
    if (obstacles[j].state == 0) { continue;}
    boolean is_col = false;
    for (byte i = 0; i < NUM_PLAYER_BULLETS; i++) {
      if (player.bullets[i].enabled) {
        is_col = collidePointRect(player.bullets[i].point, obstacles[j].rect);
        if (is_col) {
          player.bullets[i].enabled = false;
          break;
        }
      }
    }
    if (is_col) {
      player.power = min(player.power + 1, 25);
      score++;
      shiftLevel();
      obstacles[j].state = 0;
    }
  }
}

void checkPlayerCollision() {
  for (byte i = 0; i < num_enemies; i++) {
    Point tmp_point;
    tmp_point.x = enemies[i].bullet.point.x;
    tmp_point.y = enemies[i].bullet.point.y;
    boolean is_enemy_col = collideRectRect(enemies[i].rect, player.rect);
    boolean is_bullet_col = (enemies[i].bullet.enabled) && (collidePointRect(tmp_point, player.rect));
    if ((is_enemy_col) || (is_bullet_col)) {
      if (score > best_score) { 
        gb.save.set(0, score);
        best_score = score;
        is_highscore = true;
        } else {
          is_highscore = false;
        }
      displayGameover();
      return;
    }
  }
  for (byte i = 0; i < num_obstacles; i++) {
    if (obstacles[i].state == 0) { continue;}
    boolean is_obstacle_col = collideRectRect(obstacles[i].rect, player.rect);
    if (is_obstacle_col) {
      if (obstacles[i].type == 3) {
        player.power = 25;
        score++;
        shiftLevel();
        obstacles[i].state = 0;      
        continue;
      }
      if (score > best_score) { 
        gb.save.set(0, score);
        best_score = score;
        is_highscore = true;
        } else {
          is_highscore = false;
        }
      displayGameover();
      return;
    }
  }


  
}

void displayGameover() {
  for (byte i = 0; i <= 3; i++) {
    gb.lights.drawPixel(0, i, RED);
    gb.lights.drawPixel(1, i, RED);
  }
  drawScore();
  gb.display.setColor(WHITE);
  gb.display.setCursor(20,20);
  gb.display.print("GAME OVER");
  if (is_highscore) {
     gb.display.setColor(RED);
     gb.display.setCursor(12,30);
     gb.display.print("NEW HIGH SCORE");
  }
  is_gameover = true;
}

void displayTitle() {
  gb.display.setColor(WHITE);
  gb.display.setCursor(20,20);
  gb.display.println("Meta Patrol");
  gb.display.println("                  ");
  gb.display.print("   Highscore : ");
  gb.display.println(best_score);
}

void drawScore() {
 gb.display.setColor(BLACK);
 gb.display.fillRect(0,57,80,64); 
 gb.display.setColor(GRAY);
 gb.display.setCursor(1, HEIGHT - 6);
 gb.display.print("SCORE:");
 gb.display.setCursor(30, HEIGHT - 6);
 gb.display.print(score);
 gb.display.setColor(YELLOW);
 gb.display.fillRect(54,58,player.power,5);
 gb.display.setColor(BLACK);
 gb.display.setCursor(55, HEIGHT - 6);
 gb.display.print("POW");
}

void drawPlayer() {
  byte x = player.rect.x;
  byte y = player.rect.y;
  byte w = player.rect.w;
  byte h = player.rect.h;
  gb.display.setColor(WHITE); 
  gb.display.fillRect(x, y, w, h);
}

void drawStars() {
  for (byte i = 0; i < NUM_STARS; i++) {
    byte x = stars[i].point.x;
    byte y = stars[i].point.y;
    gb.display.setColor(stars[i].color);
    gb.display.drawPixel(x, y);
  }
}

void drawEnemies() {
  for (byte i = 0; i < num_enemies; i++) {
    byte x = enemies[i].rect.x;
    byte y = enemies[i].rect.y;
    byte w = enemies[i].rect.w;
    byte h = enemies[i].rect.h;
    switch (enemies[i].type) {
      case 1:
        gb.display.setColor(RED);
        gb.display.drawRect(x, y, w, h);
        break;
      case 2:
        gb.display.setColor(BLUE);
        gb.display.drawRect(x, y, w, h);
        break;
    }
  }
}

void drawObstacles() {
  for (byte i = 0; i < num_obstacles; i++) {
    if (obstacles[i].state == 0) { continue;}
    byte x = obstacles[i].rect.x;
    byte y = obstacles[i].rect.y;
    byte w = obstacles[i].rect.w;
    byte h = obstacles[i].rect.h;
    switch (obstacles[i].type) {
      case 1: //HOLE
        gb.display.setColor(BLACK);
        gb.display.fillRect(x, y+1, w, h);
        break;
      case 2: //ROCK
        gb.display.setColor(GRAY);
        gb.display.fillRect(x, y, w, h);
        break;
      case 3: //BONUS
        gb.display.setColor(GREEN);
        gb.display.fillRect(x, y, w, h);
        break;
    }
  }
}

void drawPlayerBullets() {
  for (byte i = 0; i < NUM_PLAYER_BULLETS; i++) {
    if (player.bullets[i].enabled) {
      byte x = player.bullets[i].point.x;
      byte y = player.bullets[i].point.y;
      gb.display.drawPixel(x, y, ORANGE);
    }
  }
}

void drawEnemiesBullet() {
  byte i;
  int x, y;

  for (i = 0; i < num_enemies; i++) {
    if (enemies[i].bullet.enabled) {
      x = enemies[i].bullet.point.x;
      y = enemies[i].bullet.point.y;
      gb.display.drawPixel(x, y, PINK);
     }
  }
}

void drawScene() {
  //sky
  gb.display.setColor(WHITE);
  gb.display.drawFastHLine(0,50+camera.spd.dy,WIDTH);
  gb.display.setColor(LIGHTBLUE);
  gb.display.drawFastHLine(0,49+camera.spd.dy,WIDTH);
  gb.display.setColor(BLUE);
  gb.display.drawFastHLine(0,48+camera.spd.dy,WIDTH);
  gb.display.setColor(DARKBLUE);
  gb.display.drawFastHLine(0,47+camera.spd.dy,WIDTH);
  gb.display.setColor(DARKGRAY);
  gb.display.drawFastHLine(0,46+camera.spd.dy,WIDTH);
  //mountain back

  //city

  //plants

  //ground
  gb.display.setColor(BROWN);
  gb.display.fillRect(0,ymin + player.rect.h,WIDTH,10);

}

void setupCamera(byte x, byte y) {    //  coord of the object
  camera.spd.dx = XMap[x];
  camera.spd.dy = YMap[y];
}

// FIN DES FONCTIONS##########################################################
// ###########################################################################
