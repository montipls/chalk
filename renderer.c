#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "engine.h"

void drawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius) {
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        //  each one of these renders an octant of the circle
        SDL_RenderPoint(renderer, centreX + x, centreY - y);
        SDL_RenderPoint(renderer, centreX + x, centreY + y);
        SDL_RenderPoint(renderer, centreX - x, centreY - y);
        SDL_RenderPoint(renderer, centreX - x, centreY + y);
        SDL_RenderPoint(renderer, centreX + y, centreY - x);
        SDL_RenderPoint(renderer, centreX + y, centreY + x);
        SDL_RenderPoint(renderer, centreX - y, centreY - x);
        SDL_RenderPoint(renderer, centreX - y, centreY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void renderBall(SDL_Renderer *renderer, uint8_t color[4], vec2 ballPosition, float ballRadius, vec2 angleNormal) {
    float x = ballPosition[0];
    float y = ballPosition[1];
    SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], color[3]);
    drawCircle(renderer, x, y, ballRadius);

    if (!angleNormal) return;
    SDL_RenderLine(renderer, x, y, x + angleNormal[0], y + angleNormal[1]);
}