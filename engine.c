#include "engine.h"

vec2 GRAVITY = {0.0f, 250.0f}; // y is inverted of course

VerletObject newVerletObject(vec2 position, float radius, float frictionCoef, float elasticityCoef) {
    VerletObject obj;
    glm_vec2_copy(position, obj.position);
    glm_vec2_copy(position, obj.lastPosition);
    glm_vec2_zero(obj.acceleration);
    glm_vec2_zero(obj.velocity);

    obj.angle = 0.0;
    obj.lastAngle = 0.0;
    obj.rotation = 0.0;
    obj.angleNormal[0] = 1.0;
    obj.angleNormal[1] = 0.0;

    obj.radius = radius;
    obj.inverseRadius = 1.0f / radius;
    obj.mass = radius * radius * radius; // mass is R^3
    obj.frictionCoef = frictionCoef;
    obj.elasticityCoef = elasticityCoef;

    return obj;
}

void applyForce(VerletObject *obj, vec2 force) {
    glm_vec2_add(obj->acceleration, force, obj->acceleration);
}

// gravity is not a force
void applyGravity(VerletObject *obj) {
    glm_vec2_add(obj->acceleration, GRAVITY, obj->acceleration);
}

void updateAngle(VerletObject *obj, double dt, double dt0, int subSteps) {
    obj->lastAngle = obj->angle;
    obj->rotation *= (dt + subSteps) / (dt0 + subSteps);
    obj->angle += obj->rotation;

    if (obj->angle > M_PI) {
        obj->angle -= 2.0 * M_PI;
    } else if (obj->angle < -M_PI) {
        obj->angle += 2.0 * M_PI;
    }

    obj->angleNormal[0] = cos(2.0 * obj->angle) * (obj->radius - 1);
    obj->angleNormal[1] = sin(-2.0 * obj->angle) * (obj->radius - 1);
}

void updatePosition(VerletObject *obj, double dt, double dt0, int subSteps) {
    glm_vec2_copy(obj->position, obj->lastPosition);

    vec2 scaledVelocity, scaledAcceleration;
    float dtAverage = (dt + dt0) * 0.5f;
    float dtRatio = (dt + subSteps - dtAverage) / (dt0 + subSteps - dtAverage);
    glm_vec2_scale(obj->velocity, dtRatio, scaledVelocity);
    glm_vec2_scale(obj->acceleration, dt * dtAverage, scaledAcceleration); // averaging last 2 time steps instead of dt^2

    glm_vec2_add(obj->position, scaledVelocity, obj->position);
    glm_vec2_add(obj->position, scaledAcceleration, obj->position);

    glm_vec2_sub(obj->position, obj->lastPosition, obj->velocity); // at the end for accessibility
    glm_vec2_zero(obj->acceleration);

    if (obj->radius > 0.0f) updateAngle(obj, dt, dt0, subSteps);
}

int applyRoundConstraint(VerletObject *obj, vec2 centerPosition, float radius) {
    vec2 relativePosition;
    glm_vec2_sub(centerPosition, obj->position, relativePosition);

    float targetDistance = radius - obj->radius;
    float distanceSquared = relativePosition[0] * relativePosition[0] + relativePosition[1] * relativePosition[1];

    if (distanceSquared > targetDistance * targetDistance) {
        float distance = sqrtf(distanceSquared);  // Compute only once
        vec2 wallNormal = {relativePosition[0] / distance, relativePosition[1] / distance};  // Normalized once

        // Position correction using normalized normal
        float correctionAmount = distance - targetDistance;
        obj->position[0] += wallNormal[0] * correctionAmount;
        obj->position[1] += wallNormal[1] * correctionAmount;

        solveNormalCollision(obj, wallNormal);
    }
}

void solveNormalCollision(VerletObject *obj, vec2 wallNormal) {
    float dotProduct = glm_vec2_dot(obj->velocity, wallNormal);
    vec2 normalVelocity, tangentVelocity;
    normalVelocity[0] = wallNormal[0] * dotProduct;
    normalVelocity[1] = wallNormal[1] * dotProduct;
    tangentVelocity[0] = (obj->velocity[0] - normalVelocity[0]) * obj->frictionCoef;
    tangentVelocity[1] = (obj->velocity[1] - normalVelocity[1]) * obj->frictionCoef;

    vec2 scaledNormal;
    glm_vec2_scale(normalVelocity, -obj->elasticityCoef, scaledNormal); // important to invert

    vec2 exitVelocity;
    glm_vec2_add(scaledNormal, tangentVelocity, exitVelocity);
    vec2 initialVelocity;
    glm_vec2_copy(obj->velocity, initialVelocity);
    glm_vec2_copy(exitVelocity, obj->velocity);

    // skipping for single points
    if (obj->radius == 0.0f) return;

    vec2 initialDirection;
    glm_vec2_normalize_to(initialVelocity, initialDirection);
    float perpendicularFactor = (glm_vec2_dot(wallNormal, initialDirection) + 1) * 0.5f;
    float rotationDirection = initialVelocity[0] * exitVelocity[1] - initialVelocity[1] * exitVelocity[0];
    float speedSquared = initialVelocity[0] * initialVelocity[0] + initialVelocity[1] * initialVelocity[1];
    obj->rotation = copysign(sqrtf(speedSquared) * 0.5f * obj->inverseRadius * perpendicularFactor * obj->frictionCoef, rotationDirection);
}

void solveObjectCollision(VerletObject *objA, VerletObject *objB) {
    vec2 difference;
    glm_vec2_sub(objB->position, objA->position, difference);
    float distanceSquared = difference[0] * difference[0] + difference[1] * difference[1];
    float targetDistance = (objA->radius + objB->radius);
    if (distanceSquared >= targetDistance * targetDistance) return;

    float distance;
    if (distanceSquared <= 0.1f) {
        difference[0] = 0.1f;
        difference[1] = 0.0f;
        distance = 0.1f;
    } else {
        distance = sqrtf(distanceSquared);
    }
    float correctionFactor = (targetDistance - distance) / distance;
    vec2 massRatio = {objA->mass / (objA->mass + objB->mass), objB->mass / (objA->mass + objB->mass)};
    float averageElasticity = (objA->elasticityCoef + objB->elasticityCoef) / 2.0f;

    // update position (doesnt affect velocity)
    objA->position[0] -= difference[0] * correctionFactor * massRatio[1];
    objA->position[1] -= difference[1] * correctionFactor * massRatio[1];
    objB->position[0] += difference[0] * correctionFactor * massRatio[0];
    objB->position[1] += difference[1] * correctionFactor * massRatio[0];

    // make copy of velocity
    vec2 initialVelocityA, initialVelocityB;
    glm_vec2_copy(objA->velocity, initialVelocityA);
    glm_vec2_copy(objB->velocity, initialVelocityB);

    // update velocity
    objA->velocity[0] -= difference[0] * correctionFactor * massRatio[1] * averageElasticity;
    objA->velocity[1] -= difference[1] * correctionFactor * massRatio[1] * averageElasticity;
    objB->velocity[0] += difference[0] * correctionFactor * massRatio[0] * averageElasticity;
    objB->velocity[1] += difference[1] * correctionFactor * massRatio[0] * averageElasticity;

    vec2 invertedDifference;
    glm_vec2_negate_to(difference, invertedDifference);

    glm_vec2_normalize(difference);
    glm_vec2_normalize(invertedDifference);

    vec2 initialDirectionA, initialDirectionB;
    glm_vec2_normalize_to(initialVelocityA, initialDirectionA);
    glm_vec2_normalize_to(initialVelocityB, initialDirectionB);
    float perpendicularFactorA = (glm_vec2_dot(invertedDifference, initialDirectionA) + 1) * 0.5f;
    float perpendicularFactorB = (glm_vec2_dot(difference, initialDirectionB) + 1) * 0.5f;
    float rotationDirectionA = initialVelocityA[0] * objA->velocity[1] - initialVelocityA[1] * objA->velocity[0];
    float rotationDirectionB = initialVelocityB[0] * objB->velocity[1] - initialVelocityB[1] * objB->velocity[0];
    float speedSquaredA = initialVelocityA[0] * initialVelocityA[0] + initialVelocityA[1] * initialVelocityA[1];
    float speedSquaredB = initialVelocityB[0] * initialVelocityB[0] + initialVelocityB[1] * initialVelocityB[1];
    objA->rotation = copysign(sqrtf(speedSquaredA) * 0.5f * objA->inverseRadius * perpendicularFactorA * objA->frictionCoef, rotationDirectionA);
    objB->rotation = copysign(sqrtf(speedSquaredB) * 0.5f * objB->inverseRadius * perpendicularFactorB * objB->frictionCoef, rotationDirectionB);
}