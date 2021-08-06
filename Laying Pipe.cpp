#define OLC_PGE_APPLICATION
#include <iostream>
#include "olcPixelGameEngine.h"
#include <algorithm>
#include <random>
#include <ctime>

class LayingPipe : public olc::PixelGameEngine
{
private:
    //Pixels per pipe tile
    const int PIPE_WH = 15;
    //Number of pipe tiles per row (and column)
    const int PIPES_PER_ROW = 10;
    //The different pipe layouts. "0" indicates flow, "X" the pipe barrier, "." tile space
    const std::string PIPE_HORIZONTAL =
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "XXXXXXXXXXXXXXX"
        "000000000000000"
        "XXXXXXXXXXXXXXX"
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "...............";

    const std::string PIPE_VERTICAL =
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......";

    const std::string PIPE_LEFTDOWN =
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "XXXXXXXXX......"
        "00000000X......"
        "XXXXXXX0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......";

    const std::string PIPE_LEFTUP =
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "XXXXXXX0X......"
        "00000000X......"
        "XXXXXXXXX......"
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "...............";

    const std::string PIPE_RIGHTUP =
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0XXXXXXX"
        "......X00000000"
        "......XXXXXXXXX"
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "...............";

    const std::string PIPE_RIGHTDOWN =
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "..............."
        "......XXXXXXXXX"
        "......X00000000"
        "......X0XXXXXXX"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......"
        "......X0X......";

    //The top left location of the game field in pixels
    const int FIELD_TOP = 30;
    const int FIELD_LEFT = 30;

    std::string gameField;
    std::string selectedPipe;
    std::string pipe1, pipe2, pipe3, pipe4;
    int fieldWH = PIPE_WH * PIPES_PER_ROW;
    //Starting position of the flow
    int flowPosX = 0;
    int flowPosY = PIPE_WH / 2;
    //Selected Pipe
    int selection = 1;
    int score = 0;
    //How many seconds it takes for the flow to advance one pixel
    float flowSecondsPerPixel;
    //Seconds since last flow progression
    float flowSecondsSinceLast = 0;
    //Seconds before pipes 1,2,3,4 can be randomized again
    float shuffleCooldown;
    //"Game Over" condition
    bool flowRunning = true;
    //Expedite makes the flow run faster towards the end
    bool expedite = false;
    //To control whether the player is in the start menu
    bool menu = true;
public:

    void ResetGame() {
        flowPosX = 0;
        flowPosY = PIPE_WH / 2;
        selection = 1;
        flowRunning = true;
        flowSecondsSinceLast = 0;
        flowSecondsPerPixel = 0.4;
        std::fill(gameField.begin(), gameField.end(), ' ');
        InsertPipe(0, 0, PIPE_HORIZONTAL);
        gameField[flowPosX + flowPosY * PIPE_WH * PIPES_PER_ROW] = '1';
        srand(time(NULL));
        pipe1 = GetRandomPipe();
        pipe2 = GetRandomPipe();
        pipe3 = GetRandomPipe();
        pipe4 = GetRandomPipe();
        selectedPipe = pipe1;
        score = 0;
        shuffleCooldown = 0;
        expedite = false;
    }

    bool OnUserCreate() override {
        sAppName = "Laying Pipe";
        gameField.resize(PIPE_WH * PIPE_WH * PIPES_PER_ROW * PIPES_PER_ROW);
        ResetGame();
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        /*Flow Logic*/
        if (expedite)
            flowSecondsPerPixel = 0.01f;
        else //Flow speed increases depending on how far the flow already is (score)
            flowSecondsPerPixel = std::max(0.4f - score/2000.0f, 0.01f);
        if (!menu)
            flowSecondsSinceLast += fElapsedTime;
        if (flowSecondsSinceLast > flowSecondsPerPixel) { //Enough time has passed to advance the flow one step
            flowRunning = AdvanceFlow();
            flowSecondsSinceLast = 0;
        }

        /*User Inputs Handling & Pipe Selection & Spawning*/
        int x = GetMouseX();
        int y = GetMouseY();
        int x_snapped = (x - FIELD_LEFT) - (x - FIELD_LEFT) % PIPE_WH;
        int y_snapped = (y - FIELD_TOP) - (y - FIELD_TOP) % PIPE_WH;
        shuffleCooldown = std::max(shuffleCooldown - fElapsedTime, 0.0f);
        if ((GetMouse(0).bReleased) &&
            (x_snapped >= 0) && (x_snapped < fieldWH) && (y_snapped >= 0) && (y_snapped < fieldWH) &&
            (gameField[x_snapped + y_snapped * PIPE_WH * PIPES_PER_ROW] == ' ') &&
            (flowRunning)) { //Check whether player wants to insert a new pipe segment & whether location is legal
            InsertPipe(x_snapped, y_snapped, selectedPipe);
            if (selection == 1) {
                pipe1 = GetRandomPipe();
                selectedPipe = pipe1;
            }
            else if (selection == 2) {
                pipe2 = GetRandomPipe();
                selectedPipe = pipe2;
            }
            else if (selection == 3) {
                pipe3 = GetRandomPipe();
                selectedPipe = pipe3;
            }
            else {
                pipe4 = GetRandomPipe();
                selectedPipe = pipe4;
            }

        }
        if (GetKey(olc::Key::K1).bPressed) {
            selection = 1;
            selectedPipe = pipe1;
        }
        else if (GetKey(olc::Key::K2).bPressed) {
            selection = 2;
            selectedPipe = pipe2;
        }
        else if (GetKey(olc::Key::K3).bPressed) {
            selection = 3;
            selectedPipe = pipe3;
        }
        else if (GetKey(olc::Key::K4).bPressed) {
            selection = 4;
            selectedPipe = pipe4;
        }
        else if (GetKey(olc::Key::F).bPressed) {
            expedite = true;
        }
        else if ((GetKey(olc::Key::SPACE).bPressed) && (shuffleCooldown <= 0)) { //Randomize ("shuffle") pipes 1 to 4
            pipe1 = GetRandomPipe();
            pipe2 = GetRandomPipe();
            pipe3 = GetRandomPipe();
            pipe4 = GetRandomPipe();
            if (selection == 1)
                selectedPipe = pipe1;
            else if (selection == 2) 
                selectedPipe = pipe2;
            else if (selection == 3) 
                selectedPipe = pipe3;
            else
                selectedPipe = pipe4;
            shuffleCooldown = 10;
        }
        else if (GetKey(olc::Key::ENTER).bPressed) {
            ResetGame();
            menu = false;
        }

        /*Drawing*/
        Clear(olc::BLACK);
        FillRect(FIELD_LEFT - 5, FIELD_TOP - 5, fieldWH + 10, fieldWH + 10, olc::VERY_DARK_GREEN);
        FillRect(FIELD_LEFT, FIELD_TOP, fieldWH, fieldWH, olc::BLACK);
        DrawField();

        if (!flowRunning) {
            DrawString(ScreenWidth() / 3, 5, "GAME OVER", olc::RED, 1);
            DrawString(ScreenWidth() / 4, 15, "Enter: New game", olc::RED, 1);
        }
        else if ((x_snapped >= 0) && (x_snapped < fieldWH) && (y_snapped >= 0) && (y_snapped < fieldWH) &&
            (gameField[x_snapped + y_snapped * PIPE_WH * PIPES_PER_ROW] == ' ')) {
            DrawPipe(x_snapped + FIELD_LEFT, y_snapped + FIELD_TOP, selectedPipe, 0.5); //Draw Pipe "preview" at 50% transparency
        }

        FillRect(3, 48 + (selection - 1) * 20, PIPE_WH+4, PIPE_WH+4, olc::GREEN);
        DrawPipe(5, 50, pipe1, 1);
        DrawPipe(5, 70, pipe2, 1);
        DrawPipe(5, 90, pipe3, 1);
        DrawPipe(5, 110, pipe4, 1);
        DrawString(9, 130, std::to_string(int(ceil(shuffleCooldown))), olc::WHITE, 1);
        DrawString(ScreenWidth() / 2, ScreenHeight()-10, std::to_string(score), olc::WHITE, 1);
        if (menu) {
            DrawString(FIELD_LEFT + 5, FIELD_TOP + 50, "1, 2, 3, 4: Select", olc::WHITE, 1);
            DrawString(FIELD_LEFT + 5, FIELD_TOP + 60, "Space: Reshuffle", olc::WHITE, 1);
            DrawString(FIELD_LEFT + 5, FIELD_TOP + 70, "Enter: Start game", olc::WHITE, 1);
            DrawString(FIELD_LEFT + 5, FIELD_TOP + 80, "F: Expedite flow", olc::WHITE, 1);
            DrawString(FIELD_LEFT + 5, FIELD_TOP + 120, "By B.C.Oner", olc::WHITE, 1);
        }
        return true;
    }

    //Returns a random pipe tile
    std::string GetRandomPipe() {
        int rnd = std::rand() % 6;
        if (rnd == 0)
            return PIPE_HORIZONTAL;
        else if (rnd == 1)
            return PIPE_VERTICAL;
        else if (rnd == 2)
            return PIPE_LEFTDOWN;
        else if (rnd == 3)
            return PIPE_LEFTUP;
        else if (rnd == 4)
            return PIPE_RIGHTDOWN;
        else
            return PIPE_RIGHTUP;
    }

    //Returns false if the flow could not be continued, i.e. game over
    bool AdvanceFlow() {
        if (flowPosX < fieldWH) {
            if (gameField[flowPosX + 1 + flowPosY * PIPE_WH * PIPES_PER_ROW] == '0') {
                gameField[flowPosX + 1 + flowPosY * PIPE_WH * PIPES_PER_ROW] = '1';
                flowPosX++;
                score++;
                return true;
            }
        }
        if (flowPosX > 0) {
            if (gameField[flowPosX - 1 + flowPosY * PIPE_WH * PIPES_PER_ROW] == '0') {
                gameField[flowPosX - 1 + flowPosY * PIPE_WH * PIPES_PER_ROW] = '1';
                flowPosX--;
                score++;
                return true;
            }
        }
        if (flowPosY > 0) {
            if (gameField[flowPosX + (flowPosY - 1) * PIPE_WH * PIPES_PER_ROW] == '0') {
                gameField[flowPosX + (flowPosY - 1) * PIPE_WH * PIPES_PER_ROW] = '1';
                flowPosY--;
                score++;
                return true;
            }
        }
        if (flowPosY < fieldWH) {
            if (gameField[flowPosX + (flowPosY + 1) * PIPE_WH * PIPES_PER_ROW] == '0') {
                gameField[flowPosX + (flowPosY + 1) * PIPE_WH * PIPES_PER_ROW] = '1';
                flowPosY++;
                score++;
                return true;
            }
        }
        return false;
    }

    //Insert the passed pipe tile into the game field
    void InsertPipe(int x, int y, std::string pipe) {
        for (int i = 0; i < PIPE_WH; i++)
            for (int j = 0; j < PIPE_WH; j++)
            {
                gameField[x + i + (y+j) * PIPE_WH * PIPES_PER_ROW] = pipe[i + j * PIPE_WH];
            }
    }

    //Draw a pipe tile at (x,y) where alpha controls the brightness
    void DrawPipe(int x, int y, std::string pipe, float alpha) {
        for (int i = 0; i < PIPE_WH; i++)
            for (int j = 0; j < PIPE_WH; j++)
            {
                switch (pipe[i + j * PIPE_WH])
                {
                case '.':
                    Draw(x + i , y + j, olc::GREY*alpha);
                    break;
                case 'X': 
                    Draw(x + i, y + j, olc::VERY_DARK_GREY*alpha);
                case '1':
                    Draw(x + i, y + j, olc::WHITE*alpha);
                    break;
                }
            }

    }

    //Draws the entire game field
    void DrawField() {
        for (int i = 0; i < PIPE_WH * PIPES_PER_ROW; i++)
            for (int j = 0; j < PIPE_WH * PIPES_PER_ROW; j++)
            {
                switch (gameField[i + j * PIPE_WH * PIPES_PER_ROW])
                {
                case '.':
                    Draw(i + FIELD_LEFT, j + FIELD_TOP, olc::GREY);
                    break;
                case 'X':
                    Draw(i + FIELD_LEFT, j + FIELD_TOP, olc::VERY_DARK_GREY);
                    break;
                case '1':
                    Draw(i + FIELD_LEFT, j + FIELD_TOP, olc::GREEN);
                    break;
                case '0':
                    Draw(i + FIELD_LEFT, j + FIELD_TOP, olc::WHITE);
                    break;
                }
            }
    }
};

int main() {
    LayingPipe game;
    if (game.Construct(200, 200, 4, 4)) {
        game.Start();
    }
}