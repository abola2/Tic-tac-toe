#include <map>


#include "raylib.h"

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

enum Shift
{
    FirstPlayer, //black
    SecondPlayer, //white
    None,
};

enum GameState
{
    Menu,
    Playing
};


struct PositionData {
    Rectangle rect;
    Shift shift;
    bool claimed;
};

struct MenuButton {
    const char* message;
    Rectangle rect;
    int gameWidth;
};

// Todo: remove global
Shift currentTurn = FirstPlayer;
std::pmr::map<std::pair<int, int>, PositionData> positions;
Shift winner = None;
void setup(int gameWidth, int cubeWidth, int cubeHeight);
bool isInside(int gameWidth);
bool checkWin(int gameWidth, Shift currentPlayer);
bool isFull(int gameWidth);
void game(int gameWidth, int cubeWidth, int cubeHeight, GameState &state);
void menu(int &gWidth, GameState &state, int &cubeWidth, int &cubeHeight);

bool hasWinner();

int main()
{
    GameState state = Menu;
    int gameWidth = 2;

    int cubeWidth = windowWidth / gameWidth;
    int cubeHeight = windowHeight / gameWidth;
    
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);

    InitWindow(windowWidth, windowHeight, "RikRakRoe");
    
    Image image = LoadImage("tic-tac-toe.png"); //random image from pixabay
    SetWindowIcon(image);
    UnloadImage(image);
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);
        if (state == Playing) {
            game(gameWidth, cubeWidth, cubeHeight, state);
        } else {
            menu(gameWidth, state, cubeWidth, cubeHeight);
        }
        EndDrawing();
    }

    return 0;

}

void menu(int &gWidth, GameState &state, int &cubeWidth, int &cubeHeight) {
    float width = windowWidth / 3;

    float centerWidth = (windowWidth / 2) - width / 2;
    float height = windowHeight / 7;
    MenuButton buttons[3] = {{"3x3", {centerWidth, 50, width, height}, 3}, {"4x4", {centerWidth, 200, width, height}, 4}, {"5x5", {centerWidth, 350, width, height}, 5}};

    Vector2 mousePosition = GetMousePosition();

    for (MenuButton b : buttons) {
        bool inside = CheckCollisionPointRec(mousePosition, b.rect);
        DrawRectangleV(Vector2(b.rect.x, b.rect.y), Vector2(b.rect.width, b.rect.height), inside ? RED : RAYWHITE);
        DrawText(b.message, b.rect.x + b.rect.width / 3, b.rect.y + b.rect.height / 3, 40, BLUE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inside) {
            gWidth = b.gameWidth;
            state = Playing;
            break;
        }
    }
    if (state == Playing) {
        cubeWidth = windowWidth / gWidth;
        cubeHeight = windowHeight / gWidth;
        setup(gWidth, cubeWidth, cubeHeight);
    }

}

void game(const int gameWidth, const int cubeWidth, const int cubeHeight, GameState &state) {
    for (auto& [key, value] : positions)
    {
        Color color = GRAY;
        if (value.claimed)
        {
            color = value.shift == FirstPlayer ? BLACK : WHITE;
        } else
        {
            if (CheckCollisionPointRec(GetMousePosition(), value.rect))
            {
                color = currentTurn == FirstPlayer ? DARKGRAY : LIGHTGRAY;
            }
        }
        DrawRectangleV(Vector2(value.rect.x, value.rect.y), Vector2(value.rect.width, value.rect.height), color);
    }

    Color winColor = { 0, 255, 136, 255 };
    if (winner == SecondPlayer)
    {
        DrawText("WHITE Won! Press R to Restart", 36, 40, 38, winColor);
    } else if (winner == FirstPlayer)
    {
        DrawText("BLACK Won! Press R to Restart", 36, 40, 38, winColor);
    }

    bool full = isFull(gameWidth);
    if (full) {
        DrawText("Press R to Restart", 36, 40, 38, winColor);
    }

    if (IsKeyPressed(KEY_R))
    {
        state = Menu;
        positions.clear();
    }

    if (!hasWinner()) {
        isInside(gameWidth);
    }
}


bool hasWinner() {
    return winner != None;
}

bool checkWin(const int gameWidth, const Shift currentPlayer)
{
    for (int y = 0; y < gameWidth; y++)
    {
        int xLine = 0;

        for (int x = 0; x < gameWidth; x++)
        {
            const auto rec = positions[{x, y}];
            if (!rec.claimed)
            {
                continue;
            }
            if (rec.shift == currentPlayer)
            {
                xLine++;
            }
            if (xLine >= gameWidth)
            {
                return true;
            }

        }

    }


    for (int x = 0; x < gameWidth; x++)
    {
        int yLine = 0;

        for (int y = 0; y < gameWidth; y++)
        {
            const auto rec = positions[{x, y}];
            if (!rec.claimed)
            {
                continue;
            }
            if (rec.shift == currentPlayer)
            {
                yLine++;
            }
            if (yLine >= gameWidth)
            {
                return true;
            }
        }

    }

    int y = 0;
    int zLine = 0;
    for (int x = 0; x < gameWidth; x++)
    {

        const auto rec = positions[{x, y++}];
        if (!rec.claimed)
        {
            continue;
        }
        if (rec.shift == currentPlayer)
        {
            zLine++;
        }
        if (zLine >= gameWidth)
        {
            return true;
        }

    }

    int yY = 0;
    zLine = 0;
    for (int x = gameWidth-1; x >= 0; x--)
    {
        const auto rec = positions[{x, yY++}];
        if (!rec.claimed)
        {
            continue;
        }
        if (rec.shift == currentPlayer)
        {
            zLine++;
        }
        if (zLine >= gameWidth)
        {
            return true;
        }

    }

    return false;
}

bool isFull(const int gameWidth) {
    int claimedCount = 0;

    for (auto d : positions) {
        if (d.second.claimed) {
            claimedCount++;
        }
    }
    if (claimedCount == (gameWidth*gameWidth)) {
        return true;
    }
    return false;
}

bool isInside(const int gameWidth)
{

    Vector2 mousePosition = GetMousePosition();
    for (auto& [key, value] : positions)
    {
        bool inside = CheckCollisionPointRec(mousePosition, value.rect);
        if (inside && !value.claimed)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {

                value.shift = currentTurn;
                value.claimed = true;
                bool won = checkWin(gameWidth, currentTurn);
                if (won)
                {
                    winner = currentTurn;
                }
                if (currentTurn == FirstPlayer)
                {
                    currentTurn = SecondPlayer;
                }
                else
                {
                    currentTurn = FirstPlayer;
                }
            }
            return true;
        }
    }
    return false;

}

void setup(const int gameWidth, const int cubeWidth, const int cubeHeight)
{
    winner = None;
    int xX = 0;
    int yY = 0;
    for (int y = 0; y < gameWidth; y++)
    {
        for (int x = 0; x < gameWidth; x++)
        {
            positions[{x, y}] = {Rectangle(xX, yY, cubeWidth, cubeHeight), {}};
            xX += cubeWidth;
        }
        xX = 0;
        yY += cubeHeight;


    }
}
