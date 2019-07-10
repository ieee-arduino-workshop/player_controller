/*
 game_engine.h - Simple example in creating your own Arduino Library
 Copyright (c) 2017 op of TMM. All right reserved.

 A pin is blinked automatically by one second intervals or by a specified interval
 Methods:
 
 Blink(pin) - Constructor. Specify pin to blink
 blink(value) - Enable blinking. Accepts 1, 0 or ON and OFF
 blink(value, length) - Enable blinking and specify interval of blinking.
*/

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <Arduino.h>
#include "packet.h"
#define ON true
#define OFF false

// Variable for the 8 directions
enum eDir
{
    STOP = 0,
    LEFT,
    UPLEFT,
    DOWNLEFT,
    RIGHT,
    UPRIGHT,
    DOWNRIGHT,
    UP,
    DOWN
};

/****************************************************************************************************
*****************************************************************************************************
** Name:        Player Class
** Description: Player's properties (E.g. movement abilities)
*****************************************************************************************************
****************************************************************************************************/
class Player
{
private:
    //Position of player
    int x, y;

    // Reset co-ordinates
    int original_x, original_y;

public:
    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Player constructor
        ** Description: Player class constructor and setting player location history variables
        *****************************************************************************************************
        ****************************************************************************************************/
    Player(int pos_x, int pos_y);

    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Position reset
        ** Description: Player's location on the field will be reset using the following function
        *****************************************************************************************************
        ****************************************************************************************************/
    void Reset();
    // Public - Get x and y function (use inline to replace those function definition wherever those are being called)
    inline int getX();          // { return x; }
    inline int getY();          // { return y; }
    inline void moveUp();       // { y--; }
    inline void moveDown();     // { y++; }
    inline void moveLeft();     // { x--; }
    inline void moveRight();    // { x++; }
    inline void setX(int newX); // { x = newX; }
    inline void setY(int newY); // { y = newY; }
};

/****************************************************************************************************
*****************************************************************************************************
** Name:        Ball Class
** Description: Ball's properties (E.g. movement properties)
*****************************************************************************************************
****************************************************************************************************/
class Ball
{
private:
    int x, y;
    int ori_x, ori_y;
    eDir direction;

public:
    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Ball constructor
        ** Description: Ball class constructor and setting ball location history variables
        *****************************************************************************************************
        ****************************************************************************************************/
    Ball(int pos_x, int pos_y);

    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Position reset
        ** Description: The ball's location on the field will be reset using the following function
        *****************************************************************************************************
        ****************************************************************************************************/
    void Reset();
    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Direction change
        ** Description: The ball's location is changed using the following function
        *****************************************************************************************************
        ****************************************************************************************************/
    void changeDirection(eDir d);
    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Random direction change
        ** Description: The ball's direction will be changed randomly
        *****************************************************************************************************
        ****************************************************************************************************/
    void randomDirection();
    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Ball direction change mechanics
        ** Description: The ball's direction change mechanics are handled by the following function
        *****************************************************************************************************
        ****************************************************************************************************/
    void Move();

    //Public - Get x and y functions (use inline to replace those function definition wherever those are being called)
    inline int getX();          // { return x; }
    inline int getY();          // { return y; }
    inline void setX(int newX); //{ x = newX; }
    inline void setY(int newY); //{ y = newY; }
    inline void moveUp();       // { y--; }
    inline void moveDown();     //{ y++; }
    inline void moveLeft();     //{ x--; }
    inline void moveRight();    //{ x++; }

    //Public - Get current direction
    inline int getDirection(); // { return direction; }
};

/****************************************************************************************************
*****************************************************************************************************
** Name:        Game manager class
** Description: Game mechanics (E.g. size of field, player movement restrictions, ball movement restrictions, ball bounch mechanics)
*****************************************************************************************************
****************************************************************************************************/
class game_manager
{
private:
    int width, height;
    char up, down, left, right;
    bool quit, dribble;
    Player *p1, *p2, *p3, *p4, *p5, *p6;
    Ball *b1;

public:
    /****************************************************************************************************
        *****************************************************************************************************
        ** Name:        Game manager constructor
        ** Description: Game manager class constructor and setting initial variables for game mechanics
        *****************************************************************************************************
        ****************************************************************************************************/
    game_manager(int w, int h);

    /****************************************************************************************************
    *****************************************************************************************************
    ** Name:        Game manager destructor
    ** Description: Game manager destructor and deletes a player in the process
    *****************************************************************************************************
    ****************************************************************************************************/
    ~game_manager();
    /****************************************************************************************************
    *****************************************************************************************************
    ** Name:        Position reset
    ** Description: The players's location on the field will be reset by calling the reset() function in the player class corresponding the player in question
    *****************************************************************************************************
    ****************************************************************************************************/
    void reset();
    /****************************************************************************************************
    *****************************************************************************************************
    ** Name:        Game draw function (debugging)
    ** Description: The following function will draw the field, ball and player within the command line insterface for debugging purposes only
    *****************************************************************************************************
    ****************************************************************************************************/
    void Draw(packet *player);

    /****************************************************************************************************
    *****************************************************************************************************
    ** Name:        Player input function
    ** Description: The following function will listen to the player's inputs and process it accordingly, currently using keyboard inputs for debugging purposes only
    *****************************************************************************************************
    ****************************************************************************************************/
    void Input(packet *player); //,uint8_t no_players);

    /****************************************************************************************************
    *****************************************************************************************************
    ** Name:        Game logic function
    ** Description: The core game logic is found here (E.g. ball bounce, kicking, player position boundary check, ball position boundary check)
    *****************************************************************************************************
    ****************************************************************************************************/
    void Logic(packet *player);

    /****************************************************************************************************
    *****************************************************************************************************
    ** Name:        Execute game manager
    ** Description: The following function will execute all functions within the game manager class and will run the game
    *****************************************************************************************************
    ****************************************************************************************************/
    void Run();

    //void Control(packet *player, uint8_t no_players);
};

#endif