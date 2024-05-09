//---#defines---
#include "gba.h"
//---Math functions---
#include <math.h> 

//---Global variables---
#define  GBA_SW 160                                        //actual gba screen width
#define  SW     120                                        //game screen width
#define  SH      80                                        //game screen height
#define RGB(r,g,b) ((r)+((g)<<5)+((b)<<10))                //15 bit, 0-31, 5bit=r, 5bit=g, 5bit=b 
int lastFr=0,FPS=0;                                        //for frames per second
int gameState=0;                                           //game state, title, game, ending
int endScreenTimer=0;                                      //time in second to hold on the ending

typedef struct                                             //player
{
    int x,y;                                                  //position
}Player; Player P;

void clearBackground()
{   
    int x,y;
    for(x=0;x<SW;x++)
    {                                                         //rgb values 0-31
        for(y=0;y<SH;y++){ VRAM[y*GBA_SW+x]=RGB(8,12,16);}       //clear all 120x80 pixels
    }
}

void buttons()                                             //buttons to press
{
    if(KEY_R ){ P.x+=3; if(P.x>SW-1){ P.x=SW-1;}}             //move right
    if(KEY_L ){ P.x-=3; if(P.x<   0){ P.x=   0;}}             //move left
    if(KEY_U ){ P.y-=3; if(P.y<   0){ P.y=   0;}}             //move up
    if(KEY_D ){ P.y+=3; if(P.y>SH-1){ P.y=SH-1;}}             //move down
    if(KEY_A ){ } 
    if(KEY_B ){ } 
    if(KEY_LS){ } 
    if(KEY_RS){ } 
    if(KEY_ST){ } 
    if(KEY_SL){ } 
}

//IN_IWRAM void drawImage(int w,int h, int xo,int yo, const u16* map, int to);
//{
//    int x,y,c;          //image w/h, position offset,  texture name, texture offset
//    for(x=0;x<w;x++) 
//    {                
//      for(y=0;y<h;y++){ c=map[(y+to*h)*w+x]; if(c>0){ VRAM[(y+yo)*GBA_SW+x+xo]=c;}} 
//    }
//}

void gameUpdates()
{	 
}

void init()
{
    P.x=70; P.y=35;                                                     //init player
    endScreenTimer=0;                                                   //clear timer
}

int main()
{int x,y; 
    //Init mode 5------------------------------------------------------------------
    *(u16*)0x4000000 = 0x405;                                           //mode 5 background 2
    *(u16*)0x400010A = 0x82;                                            //enable timer for fps
    *(u16*)0x400010E = 0x84;                                            //cnt timer overflow    
    //scale small mode 5 screen to full screen-------------------------------------
    REG_BG2PA=256/2;                                                    //256=normal 128=scale 
    REG_BG2PD=256/2;                                                    //256=normal 128=scale 
    
    init();                                                             //init game variables   
    while(1) 
    { 
        if(REG_TM2D>>12!=lastFr)                                           //draw 15 frames a second
        {
            if(gameState==0)  //title screen--------------------------------------------- 
            {  
                playSong(0,1);                                                   //play title song
                //drawImage(120,80, 0,0, title_Map, 0);                            //draw title screen
                if(KEY_STATE != 0x03FF){ init(); gameState=1;}                   //any button pressed
            } 
            if(gameState==1)  //play the game-------------------------------------------- 
            {
                clearBackground();                                               //clear the background
                playSong(1,1);                                                   //play game song
                gameUpdates();                                                   //animate and collision
                buttons();                                                       //Buttons pressed  
                //drawImage(16,16, P.x,P.y, P.map, 0);                             //draw player
                //drawImage( 8, 6, B.x,B.y, ball_Map, B.frame);                    //draw ball
            } 

            if(gameState==2)  //end screen-----------------------------------------------
            { 
                playSong(2,0);                                                   //play end song once
                //drawImage(120,80, 0,0, end_Map, 0);                              //draw end screen
                endScreenTimer+=1; if(endScreenTimer>15*3){ gameState=0;}        //hold for 3 seconds
            }


            //frames per second---------------------------------------------------------- 
            //VRAM[15]=0; VRAM[FPS]=RGB(31,31,0);                               //draw fps 
            FPS+=1; if(lastFr>REG_TM2D>>12){ FPS=0;}                          //increase frame
            lastFr=REG_TM2D>>12;                                              //reset counter           
            //swap buffers---------------------------------------------------------------
            while(*Scanline<160){}	                                         //wait all scanlines 
            if  ( DISPCNT&BACKB){ DISPCNT &= ~BACKB; VRAM=(u16*)VRAM_B;}      //back  buffer
            else{                 DISPCNT |=  BACKB; VRAM=(u16*)VRAM_F;}      //front buffer  
        }
    }
}

