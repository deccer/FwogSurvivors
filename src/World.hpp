#pragma once

#include <entt/entt.hpp>
#include "b2_user_settings.h"
#include <box2d/box2d.h>

struct SWorld {
    entt::registry EntityRegistry = {};
    entt::entity PlayerEntity;
    b2World PhysicsWorld = b2World({0.0f, 0.0f});
};

extern SWorld g_world;

auto InitializeWorld() -> void;
auto ShutdownWorld() -> void;