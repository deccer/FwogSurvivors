#pragma once

#include <entt/entt.hpp>
#include <chipmunk/chipmunk.h>

struct SWorld {
    entt::registry EntityRegistry = {};
    entt::entity PlayerEntity;
    cpSpace* PhysicsWorld = nullptr;
    cpCollisionHandler* CollisionHandlerPlayerVsEnemy = nullptr;
    cpCollisionHandler* CollisionHandlerEnemyVsEnemy = nullptr;
};

extern SWorld g_world;

auto InitializeWorld() -> void;
auto ShutdownWorld() -> void;