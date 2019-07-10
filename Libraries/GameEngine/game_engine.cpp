/*
blink.cpp - Simple example in creating your own Arduino Library
Copyright (c) op from TMM. All right reserved.

*/

#include <Arduino.h>
#include "game_engine.h"
#include "packet.h"
Player::Player(int pos_x, int pos_y)
{
    // Used to reset the player later
    original_x = pos_x;
    original_y = pos_y;

    // Set the current position of the player
    x = pos_x;
    y = pos_y;
}

void Player::Reset()
{
    x = original_x;
    y = original_y;
}

// Public - Get x and y function (use inline to replace those function definition wherever those are being called)
inline int Player::getX() { return x; }
inline int Player::getY() { return y; }
inline void Player::moveUp() { y--; }
inline void Player::moveDown() { y++; }
inline void Player::moveLeft() { x--; }
inline void Player::moveRight() { x++; }
inline void Player::setX(int newX) { x = newX; }
inline void Player::setY(int newY) { y = newY; }

Ball::Ball(int pos_x, int pos_y)
{
    ori_x = pos_x;
    ori_y = pos_y;
    x = pos_x;
    y = pos_y;
    direction = STOP;
}

void Ball::Reset()
{
    x = ori_x;
    y = ori_y;
    direction = STOP;
}

void Ball::changeDirection(eDir d)
{ //function to change direction of the ball
    direction = d;
}

void Ball::randomDirection()
{
    direction = (eDir)((rand() % 8) + 1); //random number from 1-8
}

void Ball::Move()
{
    switch (direction)
    {
    case STOP:
        break;
    case LEFT:
        x--;
        break;
    case RIGHT:
        x++;
        break;
    case UPLEFT:
        x--;
        y--;
        break;
    case UPRIGHT:
        x++;
        y--;
        break;
    case UP:
        y--;
        break;
    case DOWN:
        y++;
        break;
    case DOWNLEFT:
        x--;
        y++;
        break;
    case DOWNRIGHT:
        x++;
        y++;
        break;
    default:
        break;
    }
}

//Public - Get x and y functions (use inline to replace those function definition wherever those are being called)
inline int Ball::getX() { return x; }
inline int Ball::getY() { return y; }
inline void Ball::setX(int newX) { x = newX; }
inline void Ball::setY(int newY) { y = newY; }
inline void Ball::moveUp() { y--; }
inline void Ball::moveDown() { y++; }
inline void Ball::moveLeft() { x--; }
inline void Ball::moveRight() { x++; }

//Public - Get current direction
inline int Ball::getDirection() { return direction; }

game_manager::game_manager(int w, int h)
{
    quit = false;
    dribble = true;
    up = 'w', down = 's', left = 'a', right = 'd';

    // Width 0 -> +X & Height 0 -> -Y
    width = w;
    height = h;

    // Set player in the middle
    p1 = new Player(w / 2 + 5, h / 2);
    p2 = new Player(w / 2 - 5, h / 2);
    p3 = new Player(w / 2 + 5, h / 2 + 5);
    p4 = new Player(w / 2 - 5, h / 2 + 5);
    p5 = new Player(w / 2 + 5, h / 2 - 5);
    p6 = new Player(w / 2 - 5, h / 2 - 5);

    b1 = new Ball(w / 2 + 1, h / 2);
}

game_manager::~game_manager()
{
    delete p1;
}

void game_manager::reset()
{
    p1->Reset();
}

void game_manager::Draw(packet *player_packet)
{
    // // Clear the terminal output
    // system("cls");

    // Draw the top wall for the game
    for (int i = 0; i < width + 2; i++)
    {
        Serial.print("_");
    }
    Serial.println();

    // Draw columns - Draw the 2 side wall and all the ojebcts inbetween
    for (int i = 0; i < height; i++)
    {

        // Draw rows
        for (int j = 0; j < width; j++)
        {
            // Get the location of all objects
            // int player_x = p1->getX();
            // int player_y = p1->getY();
            int ball_x = b1->getX();
            int ball_y = b1->getY();

            int player_x;
            int player_y;
            switch (player_packet->player_id)
            {
            case 1:
                player_x = p1->getX();
                player_y = p1->getY();
                //Draw player location
                if (player_x == j && player_y == i)
                {
                    Serial.print("1");
                }
                break;
            case 2:
                player_x = p2->getX();
                player_y = p2->getY();
                //Draw player location
                if (player_x == j && player_y == i)
                {
                    Serial.print("2");
                }
                break;
            case 3:
                player_x = p3->getX();
                player_y = p3->getY();
                //Draw player location
                if (player_x == j && player_y == i)
                {
                    Serial.print("3");
                }
                break;
            case 4:
                player_x = p4->getX();
                player_y = p4->getY();
                //Draw player location
                if (player_x == j && player_y == i)
                {
                    Serial.print("4");
                }
                break;
            case 5:
                player_x = p5->getX();
                player_y = p5->getY();
                //Draw player location
                if (player_x == j && player_y == i)
                {
                    Serial.print("5");
                }
                break;
            case 6:
                player_x = p6->getX();
                player_y = p6->getY();
                //Draw player location
                if (player_x == j && player_y == i)
                {
                    Serial.print("6");
                }
                break;
            }

            // Draw the wall part at the start of every row - Using HEX character codes
            if (j == 0)
            {
                Serial.print("|");
            }

            // //Draw player location
            // if (player_x == j && player_y == i)
            // {
            //     Serial.print("1");
            // }

            // Draw the ball
            else if (ball_x == j && ball_y == i)
            {
                Serial.print("O");
            }

            // Draw empty sapace
            else
            {
                Serial.print(" ");
            }

            // Draw the wall part at the end of every row
            if (j == width - 1)
            {
                Serial.print("|");
            }
        }
        Serial.println();
    }

    // Draw the bottom wall che
    for (int i = 0; i < width + 2; i++)
    {
        Serial.print("_");
    }
    Serial.println();
}

void game_manager::Input(packet *player_packet) //, uint8_t no_players)
{
    // Get location of all objects

    int ball_x = b1->getX();
    int ball_y = b1->getY();
    int player_x;
    int player_y;
    switch (player_packet->player_id)
    {
    case 1:
        player_x = p1->getX();
        player_y = p1->getY();
        break;
    case 2:
        player_x = p2->getX();
        player_y = p2->getY();
        break;
    case 3:
        player_x = p3->getX();
        player_y = p3->getY();
        break;
    case 4:
        player_x = p4->getX();
        player_y = p4->getY();
        break;
    case 5:
        player_x = p5->getX();
        player_y = p5->getY();
        break;
    case 6:
        player_x = p6->getX();
        player_y = p6->getY();
        break;
    }

    // Control(player_packet,no_players,p1);
    // Control(player_packet,no_players,p2);
    // Control(player_packet,no_players,p3);
    // Control(player_packet,no_players,p4);
    // Control(player_packet,no_players,p5);

    // Get user input - Asynchronous ASCII key press check
    // if (_kbhit()) {

    // W - Key press
    // Serial.println(player->packet_data,BIN);
    if (player_packet->up == 1)
    {
        if ((player_y > 0) && (ball_y > 0))
        {

            // Move player
            p1->moveUp();

            // Dribble ball if dribble flag is set
            if (dribble)
            {

                // Check if the ball is besides the adjacent wall
                // If TRUE - move the player position to prevent ball clipping
                if (p1->getY() == 0)
                {

                    // Setting the player 1 units away from the border
                    p1->setY(0 + 1);

                    // Setting the ball to be next to the border
                    b1->setY(p1->getY() - 1);
                    b1->setX(p1->getX());
                }
                else
                {
                    // Set ball position near the player
                    b1->setY(p1->getY() - 1);
                    b1->setX(p1->getX());

                    // Change current direction of the ball
                    b1->changeDirection(UP);
                }
            }
        }
    }

    // S - Key press
    //if (GetAsyncKeyState(83))
    if (player_packet->down == 1)
    {
        if ((player_y < height - 1) && (ball_y < height - 1))
        {
            p1->moveDown();
            if (dribble)
            {
                if (p1->getY() == height - 1)
                {
                    p1->setY(height - 2);
                    b1->setY(p1->getY() + 1);
                    b1->setX(p1->getX());
                }
                else
                {
                    b1->setY(p1->getY() + 1);
                    b1->setX(p1->getX());
                    b1->changeDirection(DOWN);
                }
            }
        }
    }

    // A - Key press
    // if (GetAsyncKeyState(65))
    if (player_packet->left == 1)
    {
        if ((player_x > 0) && (ball_x > 0))
        {
            p1->moveLeft();
            if (dribble)
            {
                if (p1->getX() == 1)
                {
                    p1->setX(0 + 2);
                    b1->setX(p1->getX() - 1);
                    b1->setY(p1->getY());
                }
                else
                {
                    b1->setX(p1->getX() - 1);
                    b1->setY(p1->getY());
                    b1->changeDirection(LEFT);
                }
            }
        }
    }

    // D = Key press
    // if (GetAsyncKeyState(68))
    if (player_packet->right == 1)
    {
        if ((player_x < width - 1) && (ball_x < width - 1))
        {
            p1->moveRight();
            if (dribble)
            {
                if (p1->getX() == width - 1)
                {
                    p1->setX(width - 2);
                    b1->setX(p1->getX() + 1);
                    b1->setY(p1->getY());
                }
                else
                {
                    b1->setX(p1->getX() + 1);
                    b1->setY(p1->getY());
                    b1->changeDirection(RIGHT);
                }
            }
        }
    }

    /// If dribbling has been detected
    if (dribble)
    {

        // W&D - Key press
        // if (GetAsyncKeyState(87) && GetAsyncKeyState(68))
        if (player_packet->up && player_packet->right)
        {
            if ((ball_y > 0) && (ball_x < width - 1))
            {
                b1->setY(p1->getY() - 1);
                b1->setX(p1->getX() + 1);
                b1->changeDirection(UPRIGHT);
            }
        }

        // W&A - Key press
        // else if (GetAsyncKeyState(87) && GetAsyncKeyState(65))
        else if (player_packet->up && player_packet->left)
        {
            if ((ball_y > 0) && (ball_x > 0))
            {
                b1->setY(p1->getY() - 1);
                b1->setX(p1->getX() - 1);
                b1->changeDirection(UPLEFT);
            }
        }

        // S&A - Key press
        // else if (GetAsyncKeyState(83) && GetAsyncKeyState(65))
        else if (player_packet->down && player_packet->left)
        {
            if ((ball_y < height - 1) && (ball_x > 0))
            {
                b1->setY(p1->getY() + 1);
                b1->setX(p1->getX() - 1);
                b1->changeDirection(DOWNLEFT);
            }
        }

        // S&D - Key press
        // else if (GetAsyncKeyState(83) && GetAsyncKeyState(68))
        else if (player_packet->down && player_packet->right)
        {
            if ((ball_y < height - 1) && (ball_x < width - 1))
            {
                b1->setY(p1->getY() + 1);
                b1->setX(p1->getX() + 1);
                b1->changeDirection(DOWNRIGHT);
            }
        }
    }

    // SPACE - Key press
    // if (GetAsyncKeyState(VK_SPACE))
    if (player_packet->kick == 1)
    {
        dribble = false;
    }

    // // R - Key press
    // if (GetAsyncKeyState(82)) {
    //     p1->Reset();
    //     b1->Reset();
    // }

    // // Q - Key press - quit the game with q
    // if (GetAsyncKeyState(81)) {
    //     quit = true;
    // }
    // }
}

void game_manager::Logic(packet *player_packet)
{
    // If ball is not being dribble by player it moves freely
    if (!dribble)
    {
        b1->Move();
    }

    // Get location of all objects
    // int player_x = p1->getX();
    // int player_y = p1->getY();
    int ball_x = b1->getX();
    int ball_y = b1->getY();

    int player_x;
    int player_y;
    switch (player_packet->player_id)
    {
    case 1:
        player_x = p1->getX();
        player_y = p1->getY();
        break;
    case 2:
        player_x = p2->getX();
        player_y = p2->getY();
        break;
    case 3:
        player_x = p3->getX();
        player_y = p3->getY();
        break;
    case 4:
        player_x = p4->getX();
        player_y = p4->getY();
        break;
    case 5:
        player_x = p5->getX();
        player_y = p5->getY();
        break;
    case 6:
        player_x = p6->getX();
        player_y = p6->getY();
        break;
    }

    // Calculate the distance between the ball and player
    int distance_between_player_and_ball = sqrt(pow((ball_x - player_x), 2) + pow((ball_y - player_y), 2));

    // Catching the ball when it touches the player
    if (distance_between_player_and_ball == 1 || distance_between_player_and_ball == 0)
    {
        dribble = true;
    }

    // Bottom wall hit. TODO: ADD REFLECTED BOUNCE FOR STRAIGHT DIRECTION
    if (ball_y == height - 1)
    {
        switch (b1->getDirection())
        {
        case DOWN:
            b1->changeDirection(UP);
            break;
        case DOWNRIGHT:
            b1->changeDirection(UPRIGHT);
            break;
        case DOWNLEFT:
            b1->changeDirection(UPLEFT);
            break;
        default:
            break;
        }
    }

    // Top wall hit
    if (ball_y == 0)
    {
        switch (b1->getDirection())
        {
        case UP:
            b1->changeDirection(DOWN);
            break;
        case UPRIGHT:
            b1->changeDirection(DOWNRIGHT);
            break;
        case UPLEFT:
            b1->changeDirection(DOWNLEFT);
            break;
        default:
            break;
        }
    }

    // Right wall hit
    if (ball_x == width - 1)
    {
        switch (b1->getDirection())
        {
        case RIGHT:
            b1->changeDirection(LEFT);
            break;
        case UPRIGHT:
            b1->changeDirection(UPLEFT);
            break;
        case DOWNRIGHT:
            b1->changeDirection(DOWNLEFT);
            break;
        default:
            break;
        }
    }

    // Left wall hit
    if (ball_x == 0)
    {
        switch (b1->getDirection())
        {
        case LEFT:
            b1->changeDirection(RIGHT);
            break;
        case UPLEFT:
            b1->changeDirection(UPRIGHT);
            break;
        case DOWNLEFT:
            b1->changeDirection(DOWNRIGHT);
            break;
        default:
            break;
        }
    }
}

void game_manager::Run()
{
    //Serial.begin(250000);
    // if (DEBUG)
    {
        Serial.println("game_manager::Run ref[587]");
    }

    // While q (quit) button is not pressed
    while (!quit)
    {
        // if (DEBUG)
        {
            Serial.println("game_manager::Run:: Game is runing ref[592]");
        }
        // Draw the board
        // Draw();

        // Record input from player
        //Input();

        // Check logic each frame (not needed atm)
        // Logic();
        //vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}
