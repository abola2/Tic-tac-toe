#include <map>
#include "raylib.h"

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

static const Color BACKGROUND_COLOR   = {15, 23, 42, 255};

static const Color TEXT_PRIMARY       = {226, 232, 240, 255};

static const Color BUTTON_BASE        = {30, 41, 59, 255};
static const Color BUTTON_HOVER       = {51, 65, 85, 255};

static const Color HIGHLIGHT_COLOR    = {34, 211, 238, 255};

static const Color GAME_BACKGROUND    = {2, 6, 23, 255};
static const Color GRID_COLOR         = {71, 85, 105, 255};

static const Color CELL_BACKGROUND    = {15, 23, 42, 255};

static const Color PLAYER_RED_COLOR     = {244, 63, 94, 255};
static const Color PLAYER_GREEN_COLOR     = {34, 197, 94, 255};

static const Color GAME_TEXT          = {226, 232, 240, 255};

static const Color HOVER_RED_COLOR = {244, 63, 94, 120};
static const Color HOVER_GREEN_COLOR = {34, 197, 94, 120};

enum Shift
{
    RedPlayer,
    GreenPlayer,
    None,
};

enum GameState
{
    Menu,
    Playing
};

struct VectorI2 {
    int x;
    int y;

        bool operator<(const VectorI2 &o) const {
            return x < o.x || (x == o.x && y < o.y);
        }

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

void setup(int gameWidth, int cubeWidth, int cubeHeight, std::pmr::map<VectorI2, PositionData> &positions, Shift &winner);
bool isInside(int gameWidth, std::pmr::map<VectorI2, PositionData> &positions, Shift &currentTurn, Shift &winner);
bool checkWin(VectorI2 clickedPos, int gameWidth, Shift currentPlayer, std::pmr::map<VectorI2, PositionData> &positions);
bool isFull(int gameWidth, std::pmr::map<VectorI2, PositionData> &positions);
void game(int gameWidth, int cubeWidth, int cubeHeight, GameState &state, std::pmr::map<VectorI2, PositionData> &positions, Shift &currentTurn, Shift &winner);
void menu(int &gWidth, GameState &state, int &cubeWidth, int &cubeHeight, std::pmr::map<VectorI2, PositionData> &positions, Shift &winner);
bool hasWinner(Shift currentTurn);
int claimedCounter(VectorI2 clickedPos, Shift current, std::pmr::map<VectorI2, PositionData> &positions, int xAdd, int yAdd);

int main()
{
    Shift currentTurn = RedPlayer;
    std::pmr::map<VectorI2, PositionData> positions;
    Shift winner = None;

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
        if (state == Playing) {
            game(gameWidth, cubeWidth, cubeHeight, state, positions, currentTurn, winner);
        } else {
            menu(gameWidth, state, cubeWidth, cubeHeight, positions, winner);
        }
        EndDrawing();
    }

    return 0;

}

void menu(int &gWidth, GameState &state, int &cubeWidth, int &cubeHeight, std::pmr::map<VectorI2, PositionData> &positions, Shift &winner) {
    float width = windowWidth / 3;

    float centerWidth = (windowWidth / 2) - width / 2;
    float height = windowHeight / 7;
    MenuButton buttons[3] = {{"3x3", {centerWidth, 50, width, height}, 3}, {"4x4", {centerWidth, 200, width, height}, 4}, {"5x5", {centerWidth, 350, width, height}, 5}};

    Vector2 mousePosition = GetMousePosition();

    ClearBackground(BACKGROUND_COLOR);


    for (MenuButton b : buttons) {
        bool inside = CheckCollisionPointRec(mousePosition, b.rect);
        DrawRectangleV(Vector2(b.rect.x, b.rect.y), Vector2(b.rect.width, b.rect.height), inside ? BUTTON_HOVER : BUTTON_BASE);
        DrawText(b.message, b.rect.x + b.rect.width / 3, b.rect.y + b.rect.height / 3, 40, TEXT_PRIMARY);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inside) {
            gWidth = b.gameWidth;
            state = Playing;
            break;
        }
    }
    if (state == Playing) {
        cubeWidth = windowWidth / gWidth;
        cubeHeight = windowHeight / gWidth;
        setup(gWidth, cubeWidth, cubeHeight, positions, winner);
    }

}

void game(const int gameWidth, const int cubeWidth, const int cubeHeight, GameState &state, std::pmr::map<VectorI2, PositionData> &positions, Shift &currentTurn, Shift &winner) {

    ClearBackground(GAME_BACKGROUND);


    for (auto& [key, value] : positions)
    {
        Color color = CELL_BACKGROUND;
        if (value.claimed)
        {
            color = value.shift == RedPlayer ? PLAYER_RED_COLOR : PLAYER_GREEN_COLOR;
        } else
        {
            if (CheckCollisionPointRec(GetMousePosition(), value.rect))
            {
                color = currentTurn == RedPlayer ? HOVER_RED_COLOR : HOVER_GREEN_COLOR;
            }
        }
        DrawRectangleV(Vector2(value.rect.x, value.rect.y), Vector2(value.rect.width, value.rect.height), color);
        DrawRectangleLinesEx(value.rect, 2, GRID_COLOR);
    }

    int center = windowWidth/ 2;

    if (winner == GreenPlayer)
    {
        DrawText("GREEN Won!", center- center/3, 200, 58, GAME_TEXT);
        DrawText("Press R to Restart", center- center/2, 280, 48, GAME_TEXT);
    } else if (winner == RedPlayer)
    {
        DrawText("RED Won!", center- center/3, 220, 58, GAME_TEXT);
        DrawText("Press R to Restart", center- center/2, 280, 48, GAME_TEXT);
    }

    bool full = isFull(gameWidth, positions);
    if (full) {
        DrawText("GG", center, 220, 58, GAME_TEXT);
        DrawText("Press R to Restart", center- center/2, 280, 48, GAME_TEXT);
    }

    if (IsKeyPressed(KEY_R))
    {
        state = Menu;
        positions.clear();
    }

    if (!hasWinner(winner)) {
        isInside(gameWidth, positions, currentTurn, winner);
    }
}


bool hasWinner(Shift winner) {
    return winner != None;
}

bool checkWin(VectorI2 clickedPos, const int gameWidth, const Shift currentPlayer, std::pmr::map<VectorI2, PositionData> &positions)
{
    int xCheck = 1;
    int yCheck = 1;
    int zCheck = 1;

    int x = clickedPos.x;
    int y = clickedPos.y;

    auto clicked = positions[clickedPos];
    xCheck += claimedCounter(clickedPos, currentPlayer, positions, 1, 0);
    xCheck += claimedCounter(clickedPos, currentPlayer, positions, -1, 0);

    if (xCheck >= gameWidth)
    {
        return true;
    }

    yCheck += claimedCounter(clickedPos, currentPlayer, positions, 0, 1);
    yCheck += claimedCounter(clickedPos, currentPlayer, positions, 0, -1);

    if (yCheck >= gameWidth)
    {
        return true;
    }

    zCheck += claimedCounter(clickedPos, currentPlayer, positions, 1, 1);
    zCheck += claimedCounter(clickedPos, currentPlayer, positions, -1, -1);

    if (zCheck >= gameWidth)
    {
        return true;
    }

    zCheck = 1;

    zCheck += claimedCounter(clickedPos, currentPlayer, positions, -1, 1);
    zCheck += claimedCounter(clickedPos, currentPlayer, positions, 1, -1);

    if (zCheck >= gameWidth)
    {
        return true;
    }

    return false;
}

int claimedCounter(VectorI2 clickedPos, Shift current, std::pmr::map<VectorI2, PositionData> &positions, int xAdd, int yAdd) {
    int counter = 0;

    int x = clickedPos.x;
    int y = clickedPos.y;
    bool continueChecks = true;

    auto clicked = positions[clickedPos];
    while (continueChecks) {
        auto pos = positions[{x += xAdd, y += yAdd}];

        if (!pos.claimed || pos.shift != current) {
            continueChecks = false;
            break;
        }
        counter++;
    }
    return counter;

}

bool isFull(const int gameWidth, std::pmr::map<VectorI2, PositionData> &positions) {
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

bool isInside(const int gameWidth, std::pmr::map<VectorI2, PositionData> &positions, Shift &currentTurn, Shift &winner)
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
                bool won = checkWin(key, gameWidth, currentTurn, positions);
                if (won)
                {
                    winner = currentTurn;
                }
                if (currentTurn == RedPlayer)
                {
                    currentTurn = GreenPlayer;
                }
                else
                {
                    currentTurn = RedPlayer;
                }
            }
            return true;
        }
    }
    return false;

}

void setup(const int gameWidth, const int cubeWidth, const int cubeHeight, std::pmr::map<VectorI2, PositionData> &positions, Shift &winner)
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
