#include <SDL2/SDL.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// Simple pixelated Pong — clear code and explicit names

constexpr int LOGICAL_WIDTH = 160;   // logical pixels width
constexpr int LOGICAL_HEIGHT = 120;  // logical pixels height
constexpr int PIXEL_SCALE = 4;       // how many screen pixels per logical pixel
constexpr int WINDOW_WIDTH = LOGICAL_WIDTH * PIXEL_SCALE;
constexpr int WINDOW_HEIGHT = LOGICAL_HEIGHT * PIXEL_SCALE;

struct Paddle {
    float y;       // Y position in logical pixels (top corner)
    float speed;   // speed in logical pixels per second
    int width;     // width in logical pixels
    int height;    // height in logical pixels
};

struct Ball {
    float x, y;    // position in logical pixels (top-left)
    float vx, vy;  // velocity in logical pixels per second
    int size;      // size in logical pixels (square)
};

enum class Mode { MENU, VS_AI, ONE_VS_ONE, QUIT };

Mode showModeDialog(SDL_Window* window) {
    // Use native message box with buttons for a quick menu.
    const SDL_MessageBoxButtonData buttons[] = {
        { 0, 1, "Play vs AI" },
        { 0, 2, "Play 1 vs 1" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 3, "Quit" }
    };
    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION,
        window,
        "Choose mode",
        "Select a game mode:\nPlay vs AI or Play 1 vs 1?",
        SDL_arraysize(buttons),
        buttons,
        nullptr
    };

    int buttonid = 0;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
        // fallback to VS AI
        return Mode::VS_AI;
    }
    if (buttonid == 1) return Mode::VS_AI;
    if (buttonid == 2) return Mode::ONE_VS_ONE;
    return Mode::QUIT;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("My Pixelated Pong",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Game parameters
    constexpr float PADDLE_MOVE_SPEED = 120.0f; // px/s
    constexpr float AI_MOVE_SPEED = 80.0f;      // px/s
    constexpr float BALL_START_SPEED_X = 60.0f; // px/s
    constexpr float BALL_START_SPEED_Y = 30.0f; // px/s
    constexpr int WIN_SCORE = 3;                // points required to win

    Paddle leftPaddle{ (LOGICAL_HEIGHT - 24) / 2.0f, 0.0f, 3, 24 };
    Paddle rightPaddle{ (LOGICAL_HEIGHT - 24) / 2.0f, 0.0f, 3, 24 };

    Ball ball{ LOGICAL_WIDTH / 2.0f, LOGICAL_HEIGHT / 2.0f, BALL_START_SPEED_X, BALL_START_SPEED_Y, 2 };

    int scoreLeft = 0;
    int scoreRight = 0;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (std::rand() % 2) ball.vx = -ball.vx;

    // Show start menu
    Mode mode = showModeDialog(window);
    if (mode == Mode::QUIT) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    bool useAI = (mode == Mode::VS_AI);

    bool running = true;
    Uint32 lastTicks = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float deltaSeconds = (now - lastTicks) / 1000.0f;
        if (deltaSeconds > 0.05f) deltaSeconds = 0.05f; // avoid large jumps
        lastTicks = now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // show menu again
                    Mode newMode = showModeDialog(window);
                    if (newMode == Mode::QUIT) {
                        running = false;
                    } else {
                        useAI = (newMode == Mode::VS_AI);
                        // reset scores and ball when changing mode
                        scoreLeft = 0;
                        scoreRight = 0;
                        ball.x = LOGICAL_WIDTH / 2.0f;
                        ball.y = LOGICAL_HEIGHT / 2.0f;
                        ball.vx = BALL_START_SPEED_X * (std::rand() % 2 ? 1 : -1);
                        ball.vy = BALL_START_SPEED_Y * (std::rand() % 2 ? 1 : -1);
                    }
                }
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(nullptr);

        // Left player controls (W / S)
        leftPaddle.speed = 0.0f;
        if (keys[SDL_SCANCODE_W]) leftPaddle.speed = -PADDLE_MOVE_SPEED;
        if (keys[SDL_SCANCODE_S]) leftPaddle.speed = +PADDLE_MOVE_SPEED;

        // Right player: either player input (Up/Down) or AI depending on mode
        rightPaddle.speed = 0.0f;
        if (!useAI) {
            // 1v1: both players control
            if (keys[SDL_SCANCODE_UP]) rightPaddle.speed = -PADDLE_MOVE_SPEED;
            if (keys[SDL_SCANCODE_DOWN]) rightPaddle.speed = +PADDLE_MOVE_SPEED;
        } else {
            // VS AI: ignore player 2 keys and run simple AI
            float paddleCenter = rightPaddle.y + rightPaddle.height / 2.0f;
            if (paddleCenter < ball.y - 2.0f) rightPaddle.speed = AI_MOVE_SPEED;
            else if (paddleCenter > ball.y + 2.0f) rightPaddle.speed = -AI_MOVE_SPEED;
            else rightPaddle.speed = 0.0f;
        }

        // Update positions
        leftPaddle.y += leftPaddle.speed * deltaSeconds;
        rightPaddle.y += rightPaddle.speed * deltaSeconds;

        leftPaddle.y = std::clamp(leftPaddle.y, 0.0f, static_cast<float>(LOGICAL_HEIGHT - leftPaddle.height));
        rightPaddle.y = std::clamp(rightPaddle.y, 0.0f, static_cast<float>(LOGICAL_HEIGHT - rightPaddle.height));

        ball.x += ball.vx * deltaSeconds;
        ball.y += ball.vy * deltaSeconds;

        // Bounce top/bottom
        if (ball.y < 0.0f) { ball.y = 0.0f; ball.vy = -ball.vy; }
        if (ball.y > LOGICAL_HEIGHT - ball.size) { ball.y = LOGICAL_HEIGHT - ball.size; ball.vy = -ball.vy; }

        // Collision with left paddle
        if (ball.x <= leftPaddle.width + 0.5f) {
            if (ball.y + ball.size >= leftPaddle.y && ball.y <= leftPaddle.y + leftPaddle.height) {
                ball.x = leftPaddle.width + 0.5f;
                ball.vx = -ball.vx;
                float relative = (ball.y + ball.size / 2.0f) - (leftPaddle.y + leftPaddle.height / 2.0f);
                ball.vy += (relative / (leftPaddle.height / 2.0f)) * 80.0f;
            }
        }

        // Collision with right paddle
        if (ball.x + ball.size >= LOGICAL_WIDTH - rightPaddle.width - 0.5f) {
            if (ball.y + ball.size >= rightPaddle.y && ball.y <= rightPaddle.y + rightPaddle.height) {
                ball.x = LOGICAL_WIDTH - rightPaddle.width - ball.size - 0.5f;
                ball.vx = -ball.vx;
                float relative = (ball.y + ball.size / 2.0f) - (rightPaddle.y + rightPaddle.height / 2.0f);
                ball.vy += (relative / (rightPaddle.height / 2.0f)) * 80.0f;
            }
        }

        // Goal
        if (ball.x < -10.0f) {
            ++scoreRight;

            // Check for victory
            if (scoreRight >= WIN_SCORE) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Victory", "Right player wins!", window);
                // After announcement, show mode dialog again
                Mode newMode = showModeDialog(window);
                if (newMode == Mode::QUIT) { running = false; }
                else { useAI = (newMode == Mode::VS_AI); }
                scoreLeft = 0;
                scoreRight = 0;
            }

            ball.x = LOGICAL_WIDTH / 2.0f;
            ball.y = LOGICAL_HEIGHT / 2.0f;
            ball.vx = BALL_START_SPEED_X * (std::rand() % 2 ? 1 : -1);
            ball.vy = BALL_START_SPEED_Y * (std::rand() % 2 ? 1 : -1);
        }
        if (ball.x > LOGICAL_WIDTH + 10.0f) {
            ++scoreLeft;

            // Check for victory
            if (scoreLeft >= WIN_SCORE) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Victory", "Left player wins!", window);
                Mode newMode = showModeDialog(window);
                if (newMode == Mode::QUIT) { running = false; }
                else { useAI = (newMode == Mode::VS_AI); }
                scoreLeft = 0;
                scoreRight = 0;
            }

            ball.x = LOGICAL_WIDTH / 2.0f;
            ball.y = LOGICAL_HEIGHT / 2.0f;
            ball.vx = BALL_START_SPEED_X * (std::rand() % 2 ? 1 : -1);
            ball.vy = BALL_START_SPEED_Y * (std::rand() % 2 ? 1 : -1);
        }

        // Render: all enlarged rectangles for the pixel effect
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);

        // Center dashed line
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        for (int y = 0; y < LOGICAL_HEIGHT; y += 4) {
            SDL_Rect r{ (LOGICAL_WIDTH / 2 - 1) * PIXEL_SCALE, y * PIXEL_SCALE, 2 * PIXEL_SCALE, 2 * PIXEL_SCALE };
            SDL_RenderFillRect(renderer, &r);
        }

        // Paddles
        SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
        SDL_Rect leftRect{ 0, static_cast<int>(leftPaddle.y) * PIXEL_SCALE, leftPaddle.width * PIXEL_SCALE, leftPaddle.height * PIXEL_SCALE };
        SDL_Rect rightRect{ (LOGICAL_WIDTH - rightPaddle.width) * PIXEL_SCALE, static_cast<int>(rightPaddle.y) * PIXEL_SCALE, rightPaddle.width * PIXEL_SCALE, rightPaddle.height * PIXEL_SCALE };
        SDL_RenderFillRect(renderer, &leftRect);
        SDL_RenderFillRect(renderer, &rightRect);

        // Ball
        SDL_SetRenderDrawColor(renderer, 240, 120, 60, 255);
        SDL_Rect ballRect{ static_cast<int>(ball.x) * PIXEL_SCALE, static_cast<int>(ball.y) * PIXEL_SCALE, ball.size * PIXEL_SCALE, ball.size * PIXEL_SCALE };
        SDL_RenderFillRect(renderer, &ballRect);

        // Scores (small blocks)
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        for (int i = 0; i < std::min(scoreLeft, 10); ++i) {
            SDL_Rect s{ 2 * PIXEL_SCALE + i * (3 * PIXEL_SCALE), 2 * PIXEL_SCALE, 2 * PIXEL_SCALE, 4 * PIXEL_SCALE };
            SDL_RenderFillRect(renderer, &s);
        }
        for (int i = 0; i < std::min(scoreRight, 10); ++i) {
            SDL_Rect s{ WINDOW_WIDTH - (2 * PIXEL_SCALE + (i + 1) * (3 * PIXEL_SCALE)), 2 * PIXEL_SCALE, 2 * PIXEL_SCALE, 4 * PIXEL_SCALE };
            SDL_RenderFillRect(renderer, &s);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(1);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
