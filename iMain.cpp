#include "iGraphics.h"
#include<iostream>
#include<windows.h>
#include<mmsystem.h>
#include<stdio.h>
#include <process.h>

#define screenWidth GetSystemMetrics(SM_CXSCREEN) //1366
#define screenHight GetSystemMetrics(SM_CYSCREEN) //900
#pragma comment(lib,"winmm.lib")
#define yAxisMargin 50
#define pi 3.1416
#define MAX_PLAYERS 10
#define NAME_LENGTH 20
#define FILENAME "scores.txt"
#define MAX_VELOCITY 20
#define NUM_STARS 100
#define STAR_COUNT 300
#define STAR_SIZE 2

using namespace std;
bool music_on=true,is_started=false,show_menu=true;
int r=255,g=0,b=0;
char bc[][100]= {"image\\final_bc.bmp","image\\15.bmp","image\\life.bmp"};
int xBoard=screenWidth/2-100,yBoard=yAxisMargin-10-20,dxBoard=35,board_width=200;
int xBall=screenWidth/2,yBall=yAxisMargin,radius=10,v=14;
float dxBall=15,dyBall=15;
int dxBrick=50,dyBrick=40;
int ms=0,s=0,m=0,h=0;
int score = 0,life=4;
int game_state=-1;
char Key[20];
int key_x=570,sz=0;
int is_danger=0,xdanger=rand()%screenWidth,ydanger=screenHight;
int is_cup=0,xcup=rand()%screenWidth,ycup=screenHight;
int is_fireBall=0,is_fire=0,xfire=rand()%screenWidth,yfire=screenHight;

// particle system

#define MAX_PARTICLES 100
typedef struct {
    int x, y;
    int dx, dy;
    int lifetime;
    bool active;
    int r, g, b;
} Particle;

Particle particles[MAX_PARTICLES];

void createParticles(int x, int y, int r, int g, int b) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].x = x + (rand() % 50);          // Spread particles within brick width
            particles[i].y = y + (rand() % 40);          // Spread particles within brick height
            particles[i].dx = (rand() % 10) - 5;       // Random horizontal velocity
            particles[i].dy = (rand() % 10) - 5;       // Random vertical velocity
            particles[i].lifetime = 30 + rand() % 20;  // Lifetime between 30 to 50 frames
            particles[i].active = true;
            particles[i].r = r;
            particles[i].g = g;
            particles[i].b = b;
        }
    }
}
void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].dx;
            particles[i].y += particles[i].dy;
            particles[i].lifetime--;
            if (particles[i].lifetime <= 0) {
                particles[i].active = false;  // Deactivate particle when lifetime expires
            }
        }
    }
}
void drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            iSetColor(particles[i].r, particles[i].g, particles[i].b);
            iFilledCircle(particles[i].x, particles[i].y, 2);  // Small particle size
        }
    }
}

int drawRandomStars_called=0;
void drawRandomStars()
{
    drawRandomStars_called++;
    for (int i = 0; i < NUM_STARS; i++)
    {
        int x = rand() % screenWidth;  // Random X coordinate within the window width
        int y = rand() % screenHight; // Random Y coordinate within the window height
        iSetColor(255,255,255);
        iFilledCircle(x, y, STAR_SIZE);
    }
}

typedef struct
{
    int x, y;      // Position
    int brightness;  // Brightness (0 to 255)
} Star;

Star stars[STAR_COUNT];

// Function to initialize stars with random positions
void initializeStars()
{
    srand(time(0));
    for (int i = 0; i < STAR_COUNT; i++)
    {
        stars[i].x = rand() % screenWidth;
        stars[i].y = rand() % screenHight;
        stars[i].brightness = rand() % 256;
    }
}

void updateStarBrightness()
{
    for (int i = 0; i < STAR_COUNT; i++)
    {
        stars[i].brightness = rand() % 256;
    }
    iResumeTimer(0);  // Resume timer to update again after a delay
}

// Function to generate random star positions
void generateStars()
{
    for (int i = 0; i < NUM_STARS; i++)
    {
        int x = rand() % screenWidth;
        int y = rand() % screenHight;
        iSetColor(255,255,255); // white stars
        // Draw the star as a filled circle (you can also use iPoint if you prefer)
        iFilledCircle(x, y, STAR_SIZE);
    }
}
void drawMoon(int x, int y, int radius)
{
    iSetColor(255, 255, 200);       // Light yellow color for the moon
    iFilledCircle(x, y, radius);    // Full moon
}

typedef struct
{
    char name[NAME_LENGTH];
    int score;
} Player;

// Array to store the leaderboard
Player leaderboard[MAX_PLAYERS+2];
int numPlayers = 0;

// Current score
//int score=0;
char playerName[NAME_LENGTH] = "";

// Function to load scores from the file
void loadScores()
{
    FILE *file = fopen(FILENAME, "r");
    if (file != NULL)
    {
        numPlayers = 0;
        while (fscanf(file,"%s %d",leaderboard[numPlayers].name,&leaderboard[numPlayers].score)!=EOF)
        {
            numPlayers++;
            if(numPlayers>=MAX_PLAYERS) break;
        }
        fclose(file);
    }
}

// Function to save scores to the file
void saveScores()
{
    FILE *file = fopen(FILENAME, "w");
    for (int i = 0; i < numPlayers; i++)
    {
        fprintf(file, "%s %d\n", leaderboard[i].name, leaderboard[i].score);
    }
    fclose(file);
}

// Function to add a new score to the leaderboard
void addScore(const char *name, int score)
{
    if(numPlayers<MAX_PLAYERS){
        strcpy(leaderboard[numPlayers].name, name);
        leaderboard[numPlayers].score = score;
        numPlayers++;
    }else{
        if(score > leaderboard[MAX_PLAYERS - 1].score) {
            strcpy(leaderboard[MAX_PLAYERS - 1].name, name);
            leaderboard[MAX_PLAYERS - 1].score = score;
        }
    }
    // Sort scores in descending order
    for (int i = 0; i < numPlayers - 1; i++)
    {
        for (int j = i + 1; j < numPlayers; j++)
        {
            if (leaderboard[i].score < leaderboard[j].score)
            {
                Player temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }
    if(numPlayers>(int)MAX_PLAYERS)
    {
        numPlayers=(int)MAX_PLAYERS;
    }
    saveScores();
}

// Display the leaderboard
void drawLeaderboard()
{
    // Draw background
    iSetColor(30, 30, 80); // Dark blue background
    iFilledRectangle(0, 0, screenWidth, screenHight);

    // Draw leaderboard header
    iSetColor(255, 165, 0); // Orange header background
    iFilledRectangle(200, 550, 440, 50);

    iSetColor(255, 255, 255); // White text for headers
    iText(250, 570, "Rank", GLUT_BITMAP_HELVETICA_18);
    iText(350, 570, "Player Name", GLUT_BITMAP_HELVETICA_18);
    iText(450, 570, "Score", GLUT_BITMAP_HELVETICA_18);

    // Draw leaderboard rows with alternating colors
    for (int i = 0; i < numPlayers; i++) {
        if (i % 2 == 0) {
            iSetColor(70, 130, 180); // Steel blue for even rows
        } else {
            iSetColor(100, 149, 237); // Cornflower blue for odd rows
        }
        iFilledRectangle(200, 500 - (i * 40), 440, 40);

        iSetColor(255, 255, 255); // White text for player data

        // Display rank
        char rank[5];
        sprintf(rank, "%d", i + 1);
        iText(250, 510 - (i * 40), rank, GLUT_BITMAP_HELVETICA_18);

        // Display player name
        iText(350, 510 - (i * 40), leaderboard[i].name, GLUT_BITMAP_HELVETICA_18);

        // Display player score
        char scoreText[10];
        sprintf(scoreText, "%d", leaderboard[i].score);
        iText(550, 510 - (i * 40), scoreText, GLUT_BITMAP_HELVETICA_18);
    }
}

#define total 3*6*10
struct Brick
{
    int x=0,y=0;
    bool show=true;
    int red,green,blue;
    int life;
} bricks[10][18];
void restart();

void setAll()
{
    // brick height = 40, width = 50
    int xx=0,yy=300; //400;
    for(int k=0; k<10; k++)
    {
        xx=100;
        for(int i=1; i<=18; i++)
        {
            bricks[k][i-1].x=xx;
            bricks[k][i-1].y=yy;
            if(k==0 || k==9 || i==1 || i==6 || i==7 || i==12 || i==13 || i==18)
            {
                bricks[k][i-1].red=255;
                bricks[k][i-1].green=200;
                bricks[k][i-1].blue=0;
                bricks[k][i-1].life=2;
            }
            else
            {
                bricks[k][i-1].red=rand()%256;
                bricks[k][i-1].green=rand()%256;
                bricks[k][i-1].blue=rand()%100;
                bricks[k][i-1].life=1;
            }
            bricks[k][i-1].show=1;
            if(i%6==0) xx+=100;
            xx+=50;
        }
        yy+=40;
    }
}

void setAll2()
{
    // brick height = 40, width = 50
    int xx=0,yy=300; //400;
    for(int k=0; k<10; k++)
    {
        xx=100;
        for(int i=1; i<=18; i++)
        {
            bricks[k][i-1].x=xx;
            bricks[k][i-1].y=yy;
            if(k==0 || k==9 || i==1 || i==6 || i==7 || i==12 || i==13 || i==18)
            {
                bricks[k][i-1].red=0;
                bricks[k][i-1].green=200;
                bricks[k][i-1].blue=150;
                bricks[k][i-1].life=2;
            }
            else
            {
                bricks[k][i-1].red=rand()%200;
                bricks[k][i-1].green=rand()%256;
                bricks[k][i-1].blue=rand()%100;
                bricks[k][i-1].life=1;

            }
            bricks[k][i-1].show=1;
            if(i%6==0) xx+=100;
            xx+=50;
        }
        yy+=40;
    }
}

int last_state=-1;

void iDraw()
{
    iClear();
    //show menu
    if(game_state==-1){
        iText(screenWidth/2-75,screenHight/2+90,"ABOUT",GLUT_BITMAP_TIMES_ROMAN_24);
        iText(screenWidth/2-100,screenHight/2+40,"Sujoy Mollick",GLUT_BITMAP_TIMES_ROMAN_24);
        iText(screenWidth/2-75,screenHight/2,"2305109",GLUT_BITMAP_TIMES_ROMAN_24);
        iText(screenWidth/2-80,screenHight/2-30,"Supervised by",GLUT_BITMAP_HELVETICA_12);
        iText(screenWidth/2-125,screenHight/2-70,"Md. Mostafa Akbar",GLUT_BITMAP_TIMES_ROMAN_24);
    }else if(game_state==0)
    {
        iShowBMP2(20,0,(char*)bc[0],0);
        return;
    }
    else if(game_state==1)   // Level 1
    {
        last_state=1;
        iSetColor(1,42,74);
        iFilledRectangle(0,0,screenWidth,screenHight);
        //generateStars();
        for (int i = 0; i < STAR_COUNT; i++)
        {
            iSetColor(stars[i].brightness, stars[i].brightness, stars[i].brightness);
            iFilledCircle(stars[i].x, stars[i].y, 2);
        }

        // Home menu
        iSetColor(255,255,255);
        iText(screenWidth-300,40,"Press HOME to return to Main Menu");
        iText(screenWidth-300,20,"Press SPACE to PAUSE");
        // Time
        char timer[50];
        sprintf(timer, "Time: %02d:%02d:%02d", h, m, s);
        iText(screenWidth-150,screenHight-50,timer,GLUT_BITMAP_HELVETICA_18);
        // score
        char scoreText[20];
        sprintf(scoreText, "Score: %d", score);
        iText(20, screenHight-50, scoreText, GLUT_BITMAP_HELVETICA_18);
        // level
        iSetColor(255,255,255);
        iFilledRectangle(345,screenHight-55,100,40);
        iSetColor(255,0,0);
        iText(350,screenHight-50,"LEVEL 1",GLUT_BITMAP_TIMES_ROMAN_24);
        // show ball and pad
        iSetColor(0,255,100);
        iFilledRectangle(xBoard,yBoard,board_width,20);
        if(is_fireBall) iSetColor(255,0,0);
        iFilledCircle(xBall,yBall,radius,1000);
        // collision
        int remaining_bricks=0;
        for(int i=0; i<10; i++)
        {
            for(int j=0; j<18; j++)
            {
                //if bricks[i][j] collides with brick
                if (bricks[i][j].show && xBall + radius > bricks[i][j].x && xBall - radius < bricks[i][j].x + 50 && yBall + radius > bricks[i][j].y && yBall - radius < bricks[i][j].y + 40)
                {
                    bricks[i][j].life--;
                    if((i==1 && j==17)||(i==3 && j==8) || (i==7 && j==17)||(i==7 && j==8) || (i==5 && j==16)||(i==6 && j==9) )is_danger=1;
                    if((i==1 && j==8) || (i==3 && j==2) || (i==7 && j==8) || (i==7 && j==2)) is_fire=1;
                    if((i==1 && j==4)||(i==3 && j==13) || (i==7 && j==4)||(i==7 && j==13) || (i==6 && j==4)||(i==5 && j==15)) is_cup=1;
                    bricks[i][j].red-=25,bricks[i][j].green-=25,bricks[i][j].blue+=10;
                    if(bricks[i][j].life==0)
                    {
                        bricks[i][j].show=0;
                        score+=10;
                        createParticles(bricks[i][j].x, bricks[i][j].y, bricks[i][j].red, bricks[i][j].green, bricks[i][j].blue);
                        //Particle draw
                        drawParticles();
                    }


                    //PlaySound("music\\HitBrick.wav",NULL,SND_ASYNC);
                    //dyBall*=-1;
                    int overlap_left = abs((xBall + radius) - bricks[i][j].x);
                    int overlap_right = abs((xBall - radius) - (bricks[i][j].x + 50));
                    int overlap_top = abs((yBall - radius) - (bricks[i][j].y + 40));
                    int overlap_bottom = abs((yBall + radius) - bricks[i][j].y);

                    // Determine the smallest overlap
                    int min_overlap = min(min(overlap_left, overlap_right), min(overlap_top, overlap_bottom));

                    if (min_overlap == overlap_left || min_overlap == overlap_right)
                    {
                        // Horizontal collision (left or right)
                        if(!is_fireBall) dxBall = -dxBall;
                    }
                    else if (min_overlap == overlap_top || min_overlap == overlap_bottom)
                    {
                        // Vertical collision (top or bottom)
                        if(!is_fireBall) dyBall = -dyBall;
                    }

                }
                // printing the existing bricks
                if(bricks[i][j].show)
                {
                    iSetColor(bricks[i][j].red,bricks[i][j].green,bricks[i][j].blue);
                    iFilledRectangle(bricks[i][j].x,bricks[i][j].y,dxBrick,dyBrick);
                    int R=bricks[i][j].red,G=bricks[i][j].green,B=bricks[i][j].blue;
                    for(int m=1; m<dxBrick/4; m++)
                    {
                        iSetColor(R,G,B);
                        iRectangle(bricks[i][j].x+m,bricks[i][j].y+m/2,dxBrick-m,dyBrick-m);
                        R-=10,G-=10;
                    }
                    remaining_bricks++;
                }
            }
        }
        //life
        for(int k=0; k<life; k++)
        {
            iShowBMP2(screenWidth/2+(k-2)*35,screenHight-50,(char *)bc[2],-1);
        }
        // if no remaining bricks showmenu
        if(remaining_bricks==0 || life==0)
        {
            game_state=2;
            restart();
            remaining_bricks=total;
            life=3;
            setAll();
            for(int i=0; i<10; i++)
            {
                for(int j=0; j<18; j++)
                {
                    bricks[i][j].show=true;
                }
            }
            return;
        }
        // extra points
        if(is_danger)
        {
            iShowBMP(xdanger,ydanger,"image/1.bmp");
        }
        if(is_fire)
        {
            iShowBMP(xfire,yfire,"image/5.bmp");
        }
        if(is_cup)
        {
            iShowBMP(xcup,ycup,"image/10.bmp");
        }

    }
    else if(game_state==2)
    {
        drawLeaderboard();
        iSetColor(255,255,255);
        iFilledRectangle(920,screenHight/2+95,127,30);
        iSetColor(255,0,0);
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        iText(930, screenHight/2+100,scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
        iSetColor(255,255,255);
        iText(760,250,"Player Name: ",GLUT_BITMAP_HELVETICA_18);
        iSetColor(255,255,255);
        iFilledRectangle(key_x+300,245,200,30);  //(key_x+200,245,key_x+80,245);
        iSetColor(255,0,0);
        iText(key_x+300+5,250,Key,GLUT_BITMAP_HELVETICA_18);
        iSetColor(250,250,250);
        iText(760,200,"Press ESC button to return to Menu",GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else if(game_state==3)
    {

        last_state=3;
        iSetColor(255,255,255);
        iFilledRectangle(345,screenHight-55,100,40);
        iSetColor(255,0,0);
        iText(350,screenHight-50,"LEVEL 2",GLUT_BITMAP_TIMES_ROMAN_24);
        for (int i = 0; i < STAR_COUNT; i++)
        {
            iSetColor(stars[i].brightness, stars[i].brightness, stars[i].brightness);
            iFilledCircle(stars[i].x, stars[i].y, 2);
        }
        // Home menu
        iSetColor(255,255,255);
        iText(screenWidth-300,40,"Press ESC to return to Main Menu");
        iText(screenWidth-300,20,"Press SPACE to PAUSE");
        // Time
        char timer[50];
        sprintf(timer, "Time: %02d:%02d:%02d", h, m, s);
        iText(screenWidth-150,screenHight-50,timer,GLUT_BITMAP_HELVETICA_18);
        // score
        char scoreText[20];
        sprintf(scoreText, "Score: %d", score);
        iText(20, screenHight-50, scoreText, GLUT_BITMAP_HELVETICA_18);
        // Ball & Pad
        iSetColor(0,255,100);
        iFilledRectangle(xBoard,yBoard,board_width,20);
        if(is_fireBall) iSetColor(255,0,0);
        iFilledCircle(xBall,yBall,radius,1000);
        // collision
        int remaining_bricks=0;
        for(int i=0; i<10; i++)
        {
            for(int j=0; j<18; j++)
            {
                //if bricks[i][j] collides with brick
                if (bricks[i][j].show && xBall + radius > bricks[i][j].x && xBall - radius < bricks[i][j].x + 50 && yBall + radius > bricks[i][j].y && yBall - radius < bricks[i][j].y + 40)
                {
                    bricks[i][j].life--;
                    bricks[i][j].red-=25,bricks[i][j].green-=25,bricks[i][j].blue+=10;
                    if((i==1 && j==17)||(i==3 && j==8) || (i==7 && j==12)||(i==7 && j==11) || (i==5 && j==16)||(i==6 && j==9)||(i==8 && j==8) )is_danger=1;
                    if((i==1 && j==8) || (i==3 && j==2) || (i==7 && j==8) || (i==7 && j==2)) is_fire=1;
                    if((i==1 && j==4)||(i==3 && j==13) || (i==7 && j==4)||(i==7 && j==13) || (i==6 && j==4)||(i==5 && j==15) || (i==4 && j==12)||(i==3 && j==4)||(i==9 && j==8) ) is_cup=1;

                    if(bricks[i][j].life==0)
                    {
                        bricks[i][j].show=0;
                        score+=10;
                        createParticles(bricks[i][j].x, bricks[i][j].y, bricks[i][j].red, bricks[i][j].green, bricks[i][j].blue);
                        drawParticles();
                    }
                    //PlaySound("music\\HitBrick.wav",NULL,SND_ASYNC);
                    //dyBall*=-1;
                    int overlap_left = abs((xBall + radius) - bricks[i][j].x);
                    int overlap_right = abs((xBall - radius) - (bricks[i][j].x + 50));
                    int overlap_top = abs((yBall - radius) - (bricks[i][j].y + 40));
                    int overlap_bottom = abs((yBall + radius) - bricks[i][j].y);

                    // Determine the smallest overlap
                    int min_overlap = min(min(overlap_left, overlap_right), min(overlap_top, overlap_bottom));

                    if (min_overlap == overlap_left || min_overlap == overlap_right)
                    {
                        // Horizontal collision (left or right)
                        if(!is_fireBall) dxBall = -dxBall;
                    }
                    else if (min_overlap == overlap_top || min_overlap == overlap_bottom)
                    {
                        // Vertical collision (top or bottom)
                        if(!is_fireBall) dyBall = -dyBall;
                    }

                }
                // printing the existing bricks
                if(bricks[i][j].show)
                {
                    iSetColor(bricks[i][j].red,bricks[i][j].green,bricks[i][j].blue);
                    iFilledRectangle(bricks[i][j].x,bricks[i][j].y,dxBrick,dyBrick);
                    int R=bricks[i][j].red,G=bricks[i][j].green,B=bricks[i][j].blue;
                    for(int m=1; m<dxBrick/4; m++)
                    {
                        iSetColor(R,G,B);
                        iRectangle(bricks[i][j].x+m,bricks[i][j].y+m/2,dxBrick-m,dyBrick-m);
                        R-=10,G-=10;
                    }
                    remaining_bricks++;
                }
            }
        }
        //life
        for(int k=0; k<life; k++)
        {
            iShowBMP2(screenWidth/2+(k-2)*35,screenHight-50,(char *)bc[2],0);
        }
        // Moon
        drawMoon(screenWidth-50,screenHight-100,30);
        // if no remaining bricks showmenu
        if(remaining_bricks==0 || life==0)
        {
            game_state=2;
            restart();
            remaining_bricks=total;
            life=3;
            setAll();
            for(int i=0; i<10; i++)
            {
                for(int j=0; j<18; j++)
                {
                    bricks[i][j].show=true;
                }
            }
            return;
        }
        // extra points
        if(is_danger)
        {
            iShowBMP(xdanger,ydanger,"image/1.bmp");
        }
        if(is_fire)
        {
            iShowBMP(xfire,yfire,"image/5.bmp");
        }
        if(is_cup)
        {
            iShowBMP(xcup,ycup,"image/10.bmp");
        }

    }
    else if(game_state==4)
    {
        iSetColor(255,255,0);
        iFilledRectangle(screenWidth/2-50,screenHight/2-50-5,100,50);
        iSetColor(0,0,0);
        iText(screenWidth/2-40,screenHight/2-45,"LEVEL 2",GLUT_BITMAP_HELVETICA_18);
        iSetColor(255,255,0);
        iFilledRectangle(screenWidth/2-50,screenHight/2+5,100,50);
        iSetColor(0,0,0);
        iText(screenWidth/2-40,screenHight/2+15,"LEVEL 1",GLUT_BITMAP_HELVETICA_18);
    }
}

/*
	function iKeyboard() is called whenever the user hits a key in keyboard.
	key- holds the ASCII value of the key pressed.
*/

void iKeyboard(unsigned char key)
{
    if (key == 'q')
    {
        is_started=false;
        iText(150,50,"Do you want to quit?",GLUT_BITMAP_HELVETICA_18);
    }
    else if(key=='y')
    {
        //exit(0);
    }
    else if(key=='w')
    {

    }
    else if(key=='f')
    {
        glutFullScreen();
    }
    else if(key==27)
    {
        if(is_started) is_started=0;
        game_state=0;
        PlaySound("music\\new_pubg.wav",NULL,SND_LOOP|SND_ASYNC|SND_FILENAME);
        if(game_state==2) game_state=0;
    }
    else if(key=='g')
    {

    }
    else if(key=='b')
    {

    }
    else if(key==' ')
    {
        if(is_started)
        {
            is_started=false;
        }
        else
        {
            is_started=true;
        }
    }
    else
    {

    }
    if(key=='\r')
    {
        if (strlen(playerName) > 0) {
        addScore(playerName, score);
        // Clear the input fields after adding the score
        playerName[0] = '\0';
        //Key[0] = '\0';
        int j=0;
        while(Key[j]) Key[j++]='\0';
        sz = 0;
    } else {
        printf("Player name is empty. Cannot add score.\n");
    }
    }else if(((key>='a' && key<='z')||(key>='A' && key<='Z')|| key=='\b' || key==' ') && game_state==2)
    {
        int len = strlen(playerName);
        if (key == '\b' && len > 0)    // Backspace to delete a character
        {
            Key[--sz]='\0';
            playerName[len - 1] = '\0';
        }
        else if (len < NAME_LENGTH - 1)
        {
            Key[sz++]=key;
            playerName[len] = key;
            playerName[len + 1] = '\0';
        }
    }
}

/*
	function iSpecialKeyboard() is called whenver user hits special keys like-
	function keys, home, end, pg up, pg down, arraows etc. you have to use
	appropriate constants to detect them. A list is:
	GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
	GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
	GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,
	GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
	*/
void iSpecialKeyboard(unsigned char key)
{

    if (key == GLUT_KEY_END)
    {
        show_menu=true;
        PlaySound("music\\theme_music.wav",NULL,SND_LOOP|SND_ASYNC);
        music_on=true;
    }
    else if(key==GLUT_KEY_DOWN)
    {

    }
    else if(key==GLUT_KEY_LEFT)
    {
        if(xBoard>=0)
        {
            xBoard-=dxBoard;
            if(!is_started)
                xBall-=dxBoard;
        }
    }
    else if(key==GLUT_KEY_RIGHT)
    {
        if(xBoard+200<=screenWidth)
        {
            xBoard+=dxBoard;
            if(!is_started)
                xBall+=dxBoard;
        }
    }
    else if(key==GLUT_KEY_UP)
    {

    }
    else if(key==GLUT_KEY_HOME)
    {
        game_state=0;
    }
    else if(key==GLUT_KEY_INSERT)
    {

    }
    else if(key==GLUT_KEY_F4)
    {
        if(music_on)
        {
            PlaySound(0,0,0);
            music_on=false;
        }
        else
        {
            PlaySound("music\\theme_music.wav",NULL,SND_LOOP|SND_ASYNC);
            music_on=true;
        }
    }
}

/*
	function iMouseMove() is called when the user presses and drags the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouseMove(int mx, int my)
{
    xBoard = mx - board_width / 2;

    // Boundary checks
    if (xBoard< 0) xBoard = 0; // Prevent going off the left side
    if (xBoard + board_width > 1366) xBoard = 1366 - 200; // Prevent going off the right side
    printf("x = %d, y= %d\n",mx,my);
    //place your codes here
    //x=mx,y=my;
}

/*
	function iMouse() is called when the user presses/releases the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/

void iMouse(int button, int state, int mx, int my)
{
    if(game_state==0)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            if(my<=320 && my>=250 && mx>=320 && mx<=1040)
            {
                game_state=1;
                score=0,h=0,m=0,s=0;
                restart();
                PlaySound("music\\bgmusic.wav",NULL,SND_LOOP|SND_ASYNC|SND_FILENAME);
                music_on=true;
            }
            else if(my>=180 && my<=250 && mx>=320 && mx<=1040)
            {
                if(last_state==1) game_state=1;
                if(last_state==3) game_state=3;
            }
            else if(my>=110 && my<=180 && mx>=320 && mx<=1040)
            {
                if(game_state==0) game_state=4;
            }
            else if(my>=40 && my<=110 && mx>=320 && mx<=1040)
            {
                if(game_state==0) exit(0);
            }
        }
        if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
        {
        }
    }
    else if(game_state==4)
    {
        if((mx>=screenWidth/2-50 && mx<screenWidth/2+50)&&(my>screenHight/2-50 && my<screenHight/2))
        {
            game_state=3;
            setAll2();
            restart();
            score=0;
            v=16;
            life=3;
            h=0,m=0,s=0;
            is_fireBall=0;
            PlaySound("music\\bgmusic.wav",NULL,SND_LOOP|SND_ASYNC|SND_FILENAME);
        }
        else if((mx>=screenWidth/2-50 && mx<screenWidth/2+50)&&(my>screenHight/2 && my<screenHight/2+50))
        {
            game_state=1;
            setAll();
            life=4;
            restart();
            h=0,m=0,s=0;
            score=0;
            is_fireBall=0;
            v=18;
            PlaySound("music\\theme_music.wav",NULL,SND_LOOP|SND_ASYNC|SND_FILENAME);
        }
    }else{
        if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
        {
            if(is_started) is_started=0;
            game_state=0;
            PlaySound("music\\new_pubg.wav",NULL,SND_LOOP|SND_ASYNC);
        }
    }
}
void iPassiveMouseMove(int mx, int my)
{
    // Update paddle position with mouse x-coordinate
    xBoard = mx - board_width / 2;

    // Boundary checks
    if (xBoard< 0) xBoard = 0; // Prevent going off the left side
    if (xBoard + board_width > 1366) xBoard = 1366 - 200; // Prevent going off the right side
}

void restart()
{
    is_started=false;
    xBall=screenWidth/2,yBall=yAxisMargin,radius=10;
    xBoard=xBall-100,yBoard=yBall-radius-20,dxBoard=25;
    is_fireBall=0;
}

int state_1=0;
void change()
{
    if(state_1<300){
        state_1++;
    }else{
        if(game_state==-1)
            game_state=0;
    }
    updateParticles();
    if(is_danger)
    {
        ydanger-=8;
        if(xBoard<=xdanger && xBoard+board_width>=xdanger && ydanger<yBoard)
        {
            score-=100;
            is_danger=0;
            xdanger=rand()%screenWidth;
        }
        else if(ydanger<0)
        {
            ydanger=screenHight;
            is_danger=0;
            xdanger=rand()%screenWidth;
        }
    }
    if(is_cup)
    {
        ycup-=7;
        if(xBoard<xcup && xBoard+board_width>xcup && ycup<yBoard)
        {
            score+=200;
            is_cup=0;
            xcup=rand()%screenWidth;
        }
        else if(ycup<0)
        {
            ycup=screenHight;
            is_cup=0;
            xcup=rand()%screenWidth;
        }
    }
    if(is_fire)
    {
        yfire-=8;
        if(xBoard<xfire && xBoard+board_width>xfire && yfire<=yBoard)
        {
            is_fireBall=1;
        }
        else if(yfire<0)
        {
            yfire=screenHight;
            is_fire=0;
            xfire=rand()%screenWidth;
        }
    }
    if(is_started)
    {
        xBall+=dxBall,yBall+=dyBall;
        // collides with wall
        if(xBall>=screenWidth || xBall<=0) dxBall*=-1;
        if(yBall>=screenHight) dyBall*=-1;
    }
    if((xBall>=xBoard && xBall<=xBoard+board_width)&&(yBall-radius<=yBoard+20))
    {
        //dyBall*=-1;
        float theta=pi-((xBall-xBoard)*1.0/200.0)*pi;
        if(theta<0.5254) theta+=0.523;
        if(theta>2.053) theta-=0.523;
        dxBall = v*cos(theta);
        dyBall=v*sin(theta);
        v=sqrt(dxBall*dxBall+dyBall*dyBall);
        if(v>MAX_VELOCITY) v=MAX_VELOCITY;
        //playSoundAsync(hitSound);
    }
    else
    {
        if(yBall+radius<yAxisMargin)
        {
            //PlaySound(0,0,0);
            if(is_fire) is_fireBall=0;
            life--;
            restart();
        }
    }
    // counting time
    if(is_started) ms+=50;
    if(ms>=1000)
    {
        ms=0;
        s++;
        if(s==60)
        {
            s=0;
            m++;
            if(m==60)
            {
                m=0;
                h++;
            }
        }
    }
}


int main()
{

    loadScores();
    setAll();
    initializeStars();
    iSetTimer(600, updateStarBrightness);
    iSetTimer(10,change);
    PlaySound("music\\new_pubg.wav",NULL,SND_LOOP|SND_ASYNC);
    iInitialize(screenWidth,screenHight, "My Game"); // gives frame of the code
    glutFullScreen();
    return 0;
}
