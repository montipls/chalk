#include <cglm/cglm.h>
#include <SDL3/SDL.h>

#include "engine.h"

#ifndef RENDERER_H
#define RENDERER_H

void drawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius);
void renderBall(SDL_Renderer *renderer, uint8_t color[4], vec2 ballPosition, float ballRadius, vec2 *angleNormal);

#endif