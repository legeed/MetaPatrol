// ###########################################################################
//         
//
//         Title : MetaPatrol.ino
//         Author: Jean-Baptiste Chaffange Koch
//    Description: A moonpatrol-like game for Gamebuino Meta.
//        Version: 0
//           Date: 29 Mar 2018
//        License: GPLv3 (see LICENSE)
//
//    MetaPatrol Copyright (C) 2018 Jean-Baptiste Chaffange Koch
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    any later version.
//
//    
//###########################################################################


// ###########################################################################
//    VARIABLES
// ###########################################################################



#include <Gamebuino-Meta.h>
#include "bitmaps.h"
#include "worlds.h"

//mode du jeu
#define GAME 0
#define MENU 1 
#define PAUSE 2
#define GAMEOVER 3
byte gamemode = MENU;    //mode du jeu

//variables diverses du jeu
boolean testvar;
byte xmin,xmax,ymin,ymax; 
int timecount;
byte gamespeed[7] = {0,1,2,4,8,16,32};    //Attention à la taille du tableau !

//boites fixes
typedef struct {
  byte w;           //largeur
  float x;          //pos X
  byte h;           //hauteur
  float y;          //pos Y
  byte type;        //type
  byte frame;       //compteur de frame pour l'animation
  boolean flag;        //flag animation
} Box;

//boites mouvantes (joueur, obstacles, tirs, UFO, etc.)
typedef struct {
  byte w;           //largeur
  float x;          //pos X
  float xv;         //vitesse X
  byte h;           //hauteur
  float y;          //pos Y
  float yv;         //vitesse Y
  byte mode;        //mode
  boolean shoot;    //tire ?
  byte state;       //etat
  byte life;        //vies
  byte power;       //power
  byte frame;       //compteur de frame pour l'animation
  boolean anim;     //flag animation
} MovingBox;

#define NUM_OBSTACLES 10               //Ennemis = obstacles à la progression du joueur
Box obstacle[NUM_OBSTACLES];
#define NUM_PSHOOTS 50                //Tirs du joueur
MovingBox p_shoot[NUM_PSHOOTS];
#define NUM_ESHOOTS 50                //Tirs des Ennemis
MovingBox e_shoot[NUM_ESHOOTS];

MovingBox player;                       //création de  l'objet qui représente le joueur 

//Etat possible du joueur ou d'un ennemi ou d'un tir
#define NORMAL 0
#define HIT 1
#define DEAD 2
#define INACTIVE 10        // Tirs
#define ACTIVE 11          
#define EXPLOSION 12       // Explosion

//Modes du Joueur
#define NORMAL 0
#define CROUCH 1
#define JUMP 2 

//Types possibles d'un obstacle
#define INACTIVE 0
#define ROCK 1
#define MAN 2    
#define TANK 3
#define FLYER 4

//Types possibles d'une tile
#define EMPTY 0            //aucun effet
#define WALL 1         //bloque le joueur
#define BLOC 2        // = Bloc mais destructible avec un tir
#define WATER 3 

byte world_buffer_w = 30;
byte world_buffer_h = 8;
byte tile_w = 8;
byte tile_h = 8;
int world_camera_x = 0;       //position de la caméra en x (à droite de l'écran)

// FIN DES VARIABLES #########################################################
// ###########################################################################


// ###########################################################################
//    SETUP
// ###########################################################################
void setup() {
gb.begin();
#define DISPLAY_MODE DISPLAY_MODE_INDEX
}
// FIN DU BEGIN ##############################################################
// ###########################################################################


// ###########################################################################
//    LOOP
// ###########################################################################
void loop() {
while(!gb.update());
gb.display.clear();
switch( gamemode ) {
  case GAME:
    updateGame();
    updateScrolling();
    updateShoots();
    updatePlayer();
    drawScene();
    break;
  case MENU:
    initGame();
    initPlayer();
    initScrolling();
    gamemode = GAME;
    break;
  case PAUSE:
    /*drawScene();
    drawInterface;
    drawObstacle();
    drawPlayer();
    drawPause();*/
    break;
  case GAMEOVER:
    /*drawGameover();*/
    break;  
}
}
// FIN DU LOOP ###############################################################
// ###########################################################################



// ###########################################################################
//    FONCTIONS DE CALCUL
// ###########################################################################

// ---------------------------------------------------------------------------
//    Initialisation du jeu
// ---------------------------------------------------------------------------
void initGame(){
  timecount = 0;
  world_camera_x = 0;
  for (byte i=0; i < NUM_PSHOOTS; i++) {
    p_shoot[i].state = INACTIVE;
    p_shoot[i].w = 4;
    p_shoot[i].h = 4;
  }
}
//---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Initialisation du joueur
// ---------------------------------------------------------------------------
void initPlayer(){
  player.w = 16;
  player.h = 16; 
  player.x = 0;
  player.xv = 0;
  player.y = 0;
  player.yv = 0;
  player.mode = NORMAL;
  player.state = NORMAL;
  player.life = 255;
  player.power = 255;
  player.frame = 0;
  player.anim = false;
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Initialisation du scrolling (sol, étoiles, fonds, etc.)
// ---------------------------------------------------------------------------
void initScrolling(){
  //Background 0

  //Background 1

  //Background 2

  //Sol
  
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Mise à jour du jeu
// ---------------------------------------------------------------------------
void updateGame(){
  timecount++;
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Détermine la tileID sous les coordonnées données
// ---------------------------------------------------------------------------
byte getTileID(byte x, byte y){             //détermine le ID (pour le graphisme)
  return world_buffer[x + (y * world_buffer_w)]; 
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Détermine la tileID sous les coordonnées données
// ---------------------------------------------------------------------------
boolean playerCollision(){             //détermine si le joueur entre en collision avec un BLOC ou WALL
  
 for (byte x = xmin; x <= xmax; x++) {    
    if (getTileID(x,ymax) == BLOC || getTileID(x,ymax) == WALL) {  
      return true;
      }
  }
 
 for (byte x = xmin; x <= xmax; x++) {    
    if (getTileID(x,ymin) == BLOC || getTileID(x,ymin) == WALL) {  
      return true;
      }
  }
  
 for(byte y = ymin; y <= ymax; y++) {
    if (getTileID(xmax,y) == WALL || getTileID(xmax,y) == BLOC) {
        return true;
        }  
    }
 for(byte y = ymin; y <= ymax; y++) {
    if (getTileID(xmin,y) == WALL || getTileID(xmin,y) == BLOC) {
        return true;
        }  
    }
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Mise à jour du joueur - position, score, vies, etc.
// ---------------------------------------------------------------------------
void updatePlayer(){
  //Mouvement - les valeurs ont été définies de manière empirique
  player.mode = NORMAL;
  player.xv *= 0.55;      //gravité et friction
  player.yv += 0.5;
  player.yv *= 0.55;  
  
  if(gb.buttons.repeat(BUTTON_RIGHT, 1)){
    player.xv += 1;}              //à droite
  if(gb.buttons.repeat(BUTTON_LEFT,1)){
    player.xv -= 1;}              //à gauche
  if(gb.buttons.repeat(BUTTON_B,9)){   //jetpack
    player.yv -= 5;
  }
  if(gb.buttons.pressed(BUTTON_DOWN)){    //accroupi
    player.h = 8;
    player.y += 8;
    player.mode = CROUCH;
  }
  if(!gb.buttons.timeHeld(BUTTON_DOWN)){ //se relève
    if(player.h ==8){
      player.h = 16;
      player.y -= 8; 
      /*if(playerCollision()){
        player.h = 8;
        player.y += 8;
        player.mode = CROUCH;
      }*/
    }
  }
  
  player.y = max(0,min(gb.display.height()-player.h,player.y + player.yv));
  player.x = max(0,min(gb.display.width()-player.w,player.x + player.xv));

  //détermine les tiles impactées par le joueur
  xmin = (min((player.x)/tile_w,world_buffer_w));
  xmax = (min((player.x+player.w-1)/tile_w,world_buffer_w));
  ymin = (min(player.y/tile_h,world_buffer_h));
  ymax = (min((player.y+player.h-1)/tile_h,world_buffer_h));

  testvar = false;
  testvar = playerCollision();
  /*if (playerCollision()){
    player.y -= player.yv;
    player.x -= player.xv;
  }*/

 
//Tir
  
  if(gb.buttons.repeat(BUTTON_A,5)) {   
    for (byte i=0; i<NUM_PSHOOTS;i++) {       //cherche une balle "dispo" dans le tableau pour le tir horizontal
      if(p_shoot[i].state == INACTIVE) {      //si une de dispo, alors je la génère et sort de la boucle avec break
          p_shoot[i].state = ACTIVE;
          player.shoot = true;
          player.power --;
          p_shoot[i].x = player.x;
          p_shoot[i].y = player.y;
          p_shoot[i].xv = 5;
          p_shoot[i].yv = 0;
          p_shoot[i].frame = 5;
          break;
          }
     }
     /*for (byte i=0; i<NUM_PSHOOTS;i++) {       //cherche une balle "dispo" dans le tableau pour le tir vertical
      if(p_shoot[i].state == INACTIVE) {      //si une de dispo, alors je la génère et sort de la boucle avec break
          p_shoot[i].state = ACTIVE;
          player.shoot = true;
          player.power --;
          timer_shoot_player = 0;
          p_shoot[i].x = player.x;
          p_shoot[i].y = player.y;
          p_shoot[i].xv = 0;
          p_shoot[i].yv = 5;
          p_shoot[i].frame = 5;
          break;
          }
     }*/
    } else {player.shoot = false;}

//Blocage sur bord gauche
  
//Application des effets d'une touche

//Regénération de power

//Perte d'une vie

}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Mise à jour du sol, des fonds et du ciel
// ---------------------------------------------------------------------------
void updateScrolling(){
  //Background 0

  //Background 1

  //Background 2

  //Sol
  //if (timecount % gamespeed[1] == 0) {camera_x++;}
 }
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    Mise à jour des tirs
// ---------------------------------------------------------------------------
void updateShoots(){
  for (byte i=0; i<NUM_PSHOOTS;i++) {   //balayage des tirs du joueur et et vérification s'ils rencontrent un obstacle (ennemi ou equivalent)
    if(p_shoot[i].state == EXPLOSION) {      //timer pour les animations d'explosions des balles qu'ont fait BOOM
        p_shoot[i].frame --;
        if(p_shoot[i].frame == 0) {p_shoot[i].state = INACTIVE;} //l'animation du BOOM est terminée
        continue;           // la balle suivante est scrutée, celle-ci ne provoquera rien
      }
    if(p_shoot[i].state == ACTIVE) {    //uniquement les balles "actives" sont déplacées
      p_shoot[i].x += p_shoot[i].xv;
      p_shoot[i].y -= p_shoot[i].yv;
    if (p_shoot[i].x < 0 || p_shoot[i].y < 0 || p_shoot[i].y > gb.display.height() || p_shoot[i].x > gb.display.width() ) {
      p_shoot[i].state = INACTIVE;
      continue;}          //la balle est sortie de l'écran, la balle suivante est scrutée
    for (byte p=0; p < NUM_OBSTACLES ; p++) {     //collisions pour les tirs du joueur
        if(gb.collideRectRect(p_shoot[i].x, p_shoot[i].y, p_shoot[i].w, p_shoot[i].h,obstacle[p].x,obstacle[p].y,obstacle[p].w,obstacle[p].h)) {
          obstacle[p].type = INACTIVE;
          p_shoot[i].xv = -1;
          p_shoot[i].yv = 0;
          p_shoot[i].state = EXPLOSION;
        }
      }
    } else {continue;}
  }
  for (byte i=0; i < NUM_ESHOOTS ; i++) {     //collisions pour les tirs des ennemis, balayage des tirs et vérification s'ils rencontrent le joueur
    if(e_shoot[i].state == EXPLOSION) {      //timer pour les animations d'explosions des balles qu'ont fait BOOM
        e_shoot[i].frame --;
        if(e_shoot[i].frame == 0) {e_shoot[i].state = INACTIVE;} //l'animation du BOOM est terminée
        continue;           // la balle suivante est scrutée, celle-ci ne provoquera rien 
      }
    if(e_shoot[i].state == ACTIVE) {    //uniquement les balles "actives" sont déplacées
      e_shoot[i].x += e_shoot[i].xv;
      e_shoot[i].y -= e_shoot[i].yv;
    if (e_shoot[i].x < 0 || e_shoot[i].y < 0 || e_shoot[i].y > gb.display.height() || e_shoot[i].x > gb.display.width() ) {
      e_shoot[i].state = INACTIVE;
      continue;}                    //la balle est sortie de l'écran, la balle suivante est scrutée
    for (byte p=0; p < NUM_OBSTACLES ; p++) {     //collisions pour les tirs du joueur
        if(gb.collideRectRect(e_shoot[i].x, e_shoot[i].y, e_shoot[i].w, e_shoot[i].h,obstacle[p].x,obstacle[p].y,obstacle[p].w,obstacle[p].h)) {
          obstacle[p].type = INACTIVE;
          p_shoot[i].xv = 0;
          p_shoot[i].yv = 0;
          p_shoot[i].state = EXPLOSION;
        }
      }
    }
  }
}
// ---------------------------------------------------------------------------

// FIN DES FONCTIONS DE CALCUL ###############################################
// ###########################################################################



// ###########################################################################
//    FONCTIONS DE DESSIN
// ###########################################################################

// ---------------------------------------------------------------------------
//    drawScene - Assemble les éléments de jeu pour l'affichage
// ---------------------------------------------------------------------------
void drawScene() {
  drawSky();
  drawGreyMountain();
  drawGreenMountain();
  drawGround();
  drawUFOS();
  drawBullets();
  drawPlayer();
  //drawHitbox();
  drawInterface();
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawInterface - Interface
// ---------------------------------------------------------------------------
void drawInterface() {
  gb.display.setColor(WHITE);
  uint16_t ram = gb.getFreeRam();
  gb.display.setCursor(1, 1);
  gb.display.print("RAM:");
  gb.display.println(ram);
  uint8_t load = gb.getCpuLoad();
  gb.display.setCursor(1, 10);
  gb.display.print("CPU:");
  gb.display.print(load);
  gb.display.println("%");
  gb.display.setCursor(1, 20);
  gb.display.print("player : ");
  gb.display.print(xmin);
  gb.display.print(xmax);
  gb.display.print(ymin);
  gb.display.print(ymax);
  gb.display.setCursor(1, 30);
  gb.display.print("coll : ");
  gb.display.print(testvar);
  
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawHitbox - Dessine les hitbox et autres trucs utiles
// ---------------------------------------------------------------------------
void drawHitbox() {
  
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawPlayer - Dessine le joueur
// ---------------------------------------------------------------------------
void drawPlayer() {
  gb.display.setColor(PINK);
  gb.display.drawRect(player.x, player .y, player.w, player.h);  
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawSky - Dessine le ciel
// ---------------------------------------------------------------------------
void drawSky() {
  
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawGround - Dessine le sol
// ---------------------------------------------------------------------------
void drawGround() {
  for (int i = 0; i < world_buffer_w; i++) {
    for (int j = 0; j < world_buffer_h; j++) {
      int tileID = world_buffer[i + (j * world_buffer_w)];
      int x_screen = i*tile_w - world_camera_x;
      if (tiletype[tileID] == EMPTY){continue;}
      if  (x_screen < -tile_w || x_screen > gb.display.width() ) {continue;}
      img_tilemap.setFrame(tileID);
      gb.display.drawImage(x_screen, j*tile_h, img_tilemap);
    }
   }
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawGreyMountain - Dessine les montagnes 1 (grises)
// ---------------------------------------------------------------------------
void drawGreyMountain(){

}  
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawGreenMountain - Dessine les montagnes 2 (vertes)
// ---------------------------------------------------------------------------
void drawGreenMountain() {

}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawUFO - Dessine les UFO
// ---------------------------------------------------------------------------
void drawUFOS() {
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//    drawBullets() - Dessine les tirs
// ---------------------------------------------------------------------------
void drawBullets() {
  for (byte i=0; i<NUM_PSHOOTS;i++) {
   if(p_shoot[i].state == ACTIVE) {
      gb.display.setColor(LIGHTGREEN);
      gb.display.fillRect(p_shoot[i].x,p_shoot[i].y,p_shoot[i].w,p_shoot[i].h);
   }
   if(p_shoot[i].state == EXPLOSION) {
     gb.display.setColor(RED);
     gb.display.fillRect(p_shoot[i].x,p_shoot[i].y,p_shoot[i].w,p_shoot[i].h);
    }
  }
}


// ---------------------------------------------------------------------------
// FIN DES FONCTIONS DE DESSIN ###############################################
// ###########################################################################





