//
// breakout.c
//
// Computer Science 50
//

// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Stanford Portable Library
#include "gevents.h"
#include "gobjects.h"
#include "gwindow.h"

// height and width of game's window in pixels
#define HEIGHT 600
#define WIDTH 400

// number of rows of bricks
#define ROWS 5

// number of columns of bricks
#define COLS 10

// radius of ball in pixels
#define RADIUS 10

// lives
#define LIVES 3

//paddle width and height
#define PWIDTH 60
#define PHEIGHT 10

//paddle's y pos
#define PADDLE_Y 530

//brick's width and height
#define BWIDTH 34
#define BHEIGHT 15

// prototypes
void initBricks(GWindow window);
GOval initBall(GWindow window);
GRect initPaddle(GWindow window);
GLabel initScoreboard(GWindow window);
void updateScoreboard(GWindow window, GLabel label, int points);
void printwon(GWindow window);
void printlost(GWindow window);
GObject detectCollision(GWindow window, GOval ball);
void LivesLabel(GWindow window);
GLabel initLives(GWindow window);
void updateLives(GWindow window, GLabel lives_left, int lives);

int main(void)
{
    // seed pseudorandom number generator
    srand48(time(NULL));

    // instantiate window
    GWindow window = newGWindow(WIDTH, HEIGHT);

    // instantiate bricks
    initBricks(window);

    // instantiate ball, centered in middle of window
    GOval ball = initBall(window);

    // instantiate paddle, centered at bottom of window
    GRect paddle = initPaddle(window);

    // instantiate scoreboard, centered in middle of window, just above ball
    GLabel label = initScoreboard(window);

    // number of bricks initially
    int bricks = COLS * ROWS;

    // number of lives initially
    int lives = LIVES;

    // number of points initially
    int points = 0;

    //ball initial velocity
    double xvelocity=drand48();
    double yvelocity=0.3;

    GObject obj;

    //show lives
    LivesLabel(window);
    GLabel lives_left = initLives(window);

    // keep playing until game over
    while (lives > 0 && bricks > 0)
    {
        updateScoreboard(window,label,points);
        GEvent event = getNextEvent(MOUSE_EVENT);
        if(event!=NULL)
        {
            if(getEventType(event) == MOUSE_MOVED)
            {
                double x = getX(event) - PWIDTH/2;
                setLocation(paddle, x, PADDLE_Y);
            }
        }
        move(ball,xvelocity,yvelocity);
        pause(2);
        if(getX(ball)+getWidth(ball)>=getWidth(window) || getX(ball) <= 0)
        {
            xvelocity = -xvelocity;
        }
        if(getY(ball) <= 0)
        {
            yvelocity = -yvelocity;
        }
        obj = detectCollision(window , ball);
        if(obj!=NULL)
        {
            if(obj==paddle)
            {
                if(getX(ball)<=getX(paddle)+getWidth(paddle)/2)
                    xvelocity = -xvelocity;
                yvelocity = -yvelocity;
            }
            else if(strcmp(getType(obj), "GRect") == 0)
            {
                if(getX(ball)<=getX(obj)+getWidth(obj)/2)
                    xvelocity = -xvelocity;
                yvelocity = -yvelocity;
                removeGWindow(window,obj);
                bricks--;
                points++;
            }
        }
        if(getY(ball) + getHeight(ball) >=getHeight(window))
        {
            lives--;
            updateLives(window,lives_left,lives);
            waitForClick();
            removeGWindow(window,ball);
            ball = initBall(window);
        }
    }

    if(lives>0)
    {
        updateScoreboard(window,label,points);
        printwon(window);
    }
    else if(bricks>0)
    {
        updateLives(window,lives_left,lives);
        printlost(window);
    }

    // wait for click before exiting
    waitForClick();

    // game over
    closeGWindow(window);
    return 0;
}

/**
 * Initializes window with a grid of bricks.
 */
void initBricks(GWindow window)
{
    GRect brick[ROWS][COLS];

    int xpos=5;
    int ypos=5;
    string color[ROWS]={"RED","BLUE","GREEN","YELLOW","ORANGE"};

    int i,j;
    for(i=0;i<ROWS;i++)
    {
        xpos=5;
        for(j=0;j<COLS;j++)
        {
            brick[i][j] = newGRect(xpos,ypos,BWIDTH, BHEIGHT);
            setColor(brick[i][j],color[i]);
            setFilled(brick[i][j], true);
            add(window, brick[i][j]);
            xpos+=39;
        }
        ypos+=20;
    }
}

/**
 * Instantiates ball in center of window.  Returns ball.
 */
GOval initBall(GWindow window)
{
    GOval ball = newGOval(195,295,10,10);
    setColor(ball,"BLACK");
    setFilled(ball, true);
    add(window,ball);
    return ball;
}

/**
 * Instantiates paddle in bottom-middle of window.
 */
GRect initPaddle(GWindow window)
{
    // TODO
    GRect paddle = newGRect(170, PADDLE_Y, PWIDTH, PHEIGHT);
    setFilled(paddle, true);
    setColor(paddle, "BLACK");
    add(window, paddle);
    return paddle;
}

/**
 * Instantiates, configures, and returns label for scoreboard.
 */
GLabel initScoreboard(GWindow window)
{
    GLabel score = newGLabel("0");
    double x = (getWidth(window) - getWidth(score)) / 2;
    double y = (getHeight(window) - getHeight(score)) / 2;
    setLocation(score, x, y);
    setFont(score,"SansSerif-40");
    setColor(score,"GRAY");
    add(window,score);
    return score;
}

/**
 * Updates scoreboard's label, keeping it centered in window.
 */
void updateScoreboard(GWindow window, GLabel label, int points)
{
    // update label
    char s[12];
    sprintf(s, "%i", points);
    setLabel(label, s);

    // center label in window
    double x = (getWidth(window) - getWidth(label)) / 2;
    double y = (getHeight(window) - getHeight(label)) / 2;
    setLocation(label, x, y);
}

/**
 * Detects whether ball has collided with some object in window
 * by checking the four corners of its bounding box (which are
 * outside the ball's GOval, and so the ball can't collide with
 * itself).  Returns object if so, else NULL.
 */
GObject detectCollision(GWindow window, GOval ball)
{
    // ball's location
    double x = getX(ball);
    double y = getY(ball);

    // for checking for collisions
    GObject object;

    // check for collision at ball's top-left corner
    object = getGObjectAt(window, x, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's top-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-left corner
    object = getGObjectAt(window, x, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // no collision
    return NULL;
}

//game won message
void printwon(GWindow window)
{
    GLabel won = newGLabel("GAME WON");
    setColor(won,"GREEN");
    setFont(won, "SansSerif-65");
    setLocation(won, 10,265);
    add(window,won);
}

//game lost message
void printlost(GWindow window)
{
    GLabel lost = newGLabel("GAME LOST");
    setColor(lost,"RED");
    setFont(lost, "SansSerif-65");
    setLocation(lost, 10,265);
    add(window,lost);
}

//initialise lives
void LivesLabel(GWindow window)
{
    GLabel label = newGLabel("LIVES: ");
    setColor(label, "GRAY");
    setFont(label, "SansSerif-20");
    setLocation(label, 20, 575);
    add(window,label);
}

//show initial lives
GLabel initLives(GWindow window)
{
    GLabel lives = newGLabel("3");
    double x = 90;
    double y = 575;
    setLocation(lives, x, y);
    setFont(lives,"SansSerif-20");
    setColor(lives,"GRAY");
    add(window,lives);
    return lives;
}

//update lives
void updateLives(GWindow window, GLabel lives_left, int lives)
{
    // update label
    char s[12];
    sprintf(s, "%i", lives);
    setLabel(lives_left, s);

    double x = 90;
    double y = 575;
    setLocation(lives_left, x, y);
}
