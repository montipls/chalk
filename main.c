#include <stdio.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cglm/cglm.h>

#include "engine.h"
#include "renderer.h"

ivec2 WINDOW_SIZE = {1920, 1080};

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    int result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (result < 0) {
        SDL_Log("SDL_Init error: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("CHALK", WINDOW_SIZE[0], WINDOW_SIZE[1], SDL_WINDOW_FULLSCREEN);
    if (!window) {
        SDL_Log("SDL_CreateWindow error: %s", SDL_GetError());
        return -2;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer error: %s", SDL_GetError());
        return -3;
    }

    long long frequency = SDL_GetPerformanceFrequency();
    long long currentTime = SDL_GetPerformanceCounter();
    double dt = 1.0 / 60.0; // default 60 fps for the first frame
    double dt0 = dt;
    int subSteps = 6;

    VerletObject *balls = NULL;
    int ballCount = 0;

    uint8_t ballColor[4] = {0xff, 0xff, 0xff, 0xff};

    vec2 constraintPosition = {WINDOW_SIZE[0] / 2.0f, WINDOW_SIZE[1] / 2.0f};
    float constraintRadius = 540.0f;
    uint8_t constraintColor[4] = {0x88, 0x88, 0x88, 0xff};

    SDL_Event event;
    float xMouse, yMouse;
    int attracted = 0;
    int gravityOn = 0;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            SDL_GetMouseState(&xMouse, &yMouse);

            switch(event.type) {
            case SDL_EVENT_QUIT:
                quit = 1;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    quit = 1;
                    break;
                }
                if (event.key.key == SDLK_G) {
                    gravityOn = !gravityOn;
                }
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == 1) {
                    int radius = 20;
                    int size = 3;

                    int oldCount = ballCount;
                    ballCount += size * size;
                    balls = realloc(balls, ballCount * sizeof(VerletObject));

                    VerletObject *newBalls = realloc(balls, ballCount * sizeof(VerletObject));
                    if (!newBalls) {
                        printf("Memory allocation failed!\n");
                        exit(1);
                    }
                    balls = newBalls;
                    
                    for (int i = 0; i < size; ++i) {
                        for (int j = 0; j < size; ++j) {
                            int index = oldCount + (i * size + j); // Correct indexing
                            balls[index] = newVerletObject((vec2) {xMouse + ((-1 + i) * (3 * radius)), yMouse + ((-1 + j) * (3 * radius))}, radius, 0.999f, 0.5f);
                        }
                    }
                }
                if (event.button.button == 3) {
                    ballCount++;
                    balls = realloc(balls, ballCount * sizeof(VerletObject));
                    vec2 newBallPosition = {xMouse, yMouse};
                    balls[ballCount - 1] = newVerletObject(newBallPosition, 40.0f, 0.999f, 0.5f);
                }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == 2) {
                    attracted = !attracted;
                }
            }
        }

        long long newTime = SDL_GetPerformanceCounter();
        dt0 = dt;
        dt = (double)(newTime - currentTime) / frequency;
        currentTime = newTime;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
        SDL_RenderClear(renderer);
        renderBall(renderer, constraintColor, constraintPosition, constraintRadius, NULL); // rendering constraint
        
        double subDt = dt / subSteps;
        double subDt0 = dt0 / subSteps;
        for (int s = 0; s < subSteps; s++) {
            for (int i = 0; i < ballCount; i++) {
                VerletObject *ball = &balls[i];
                if (gravityOn) applyGravity(ball);
                
                vec2 forceDir;
                glm_vec2_sub((vec2){xMouse, yMouse}, ball->position, forceDir);
                
                if (attracted) {
                    float dist = glm_vec2_norm(forceDir);
                    if (dist > 0.0f) {
                        glm_vec2_scale(forceDir, 1.0f / dist, forceDir); // Normalize
                        float forceStrength = fmax(0.0f, 1500.0f - dist);
                        glm_vec2_scale(forceDir, forceStrength, forceDir);
                        applyForce(ball, forceDir);
                    }
                }
                for (int j = 0; j < ballCount; j++) {
                    if (i == j) continue;
                    VerletObject *other = &balls[j];
                    solveObjectCollision(ball, other);
                }
                applyRoundConstraint(ball, constraintPosition, constraintRadius);
                updatePosition(ball, subDt, subDt0, subSteps);
            }
        }
        
        for (int i = 0; i < ballCount; i++) {
            VerletObject *ball = &balls[i];
            renderBall(renderer, ballColor, ball->position, ball->radius, &ball->angleNormal);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}