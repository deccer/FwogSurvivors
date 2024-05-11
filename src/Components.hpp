#pragma once

#include <box2d/box2d.h>
#include <glm/vec4.hpp>

struct SPlayerComponent {
    bool OnlyHereBecauseEnttDoesntLikeEmptyStructs;
};

struct SEnemyComponent {
    float Speed;
};

struct SPhysicsComponent {
    b2Body* Body;
    b2Vec2 PreviousPosition;
    b2Vec2 CurrentPosition;
};

struct SPositionComponent {
    b2Vec2 Position;
};

struct SColorComponent {
    glm::vec4 Color;
};