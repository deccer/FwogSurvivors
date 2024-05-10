#pragma once

#include <chipmunk/chipmunk.h>
#include <glm/vec4.hpp>

struct SPlayerComponent {
    bool OnlyHereBecauseEnttDoesntLikeEmptyStructs;
};

struct SEnemyComponent {
    float Speed;
};

struct SPhysicsComponent {
    cpBody* Body;
    cpVect PreviousPosition;
    cpVect CurrentPosition;
};

struct SPositionComponent {
    cpVect Position;
};

struct SColorComponent {
    glm::vec4 Color;
};