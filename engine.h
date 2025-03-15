#include <cglm/cglm.h>

#ifndef ENGINE_H
#define ENGINE_H

extern vec2 GRAVITY;

typedef struct {
    vec2 position;
    vec2 lastPosition;
    vec2 acceleration;
    vec2 velocity;
    float radius;
    float inverseRadius;
    float mass;
    float frictionCoef;
    float elasticityCoef;
    double angle;
    double lastAngle;
    double rotation;
    vec2 angleNormal;
} VerletObject;

VerletObject newVerletObject(vec2 position, float radius, float frictionCoef, float elasticityCoef);

void applyForce(VerletObject *obj, vec2 force);
void applyGravity(VerletObject *obj);
void updateAngle(VerletObject *obj, double dt, double dt0, int subSteps);
void updatePosition(VerletObject *obj, double dt, double dt0, int subSteps);
int applyRoundConstraint(VerletObject *obj, vec2 centerPosition, float radius);
void solveNormalCollision(VerletObject *obj, vec2 wallNormal);
void solveObjectCollision(VerletObject *objA, VerletObject *objB);

#endif