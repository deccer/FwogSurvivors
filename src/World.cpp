#include "World.hpp"
#include "Components.hpp"

#include <random>
#include <ranges>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/vec2.hpp>

SWorld g_world = {};

enum EMobileType : uint32_t {
    Player = 1,
    Enemy = 2,
    Wall = 4,
    Bullet = 8
};

struct SMobile {

    cpBody* Body = nullptr;
    cpShape* Shape = nullptr;
    cpShapeFilter ShapeFilter = {};
};

std::vector<SMobile> g_mobiles = {};

auto AddMobile(
    cpVect position,
    EMobileType mobileType,
    uint32_t mobileTypeCollideAgainst,
    float mass,
    float size) -> void {

    auto body = cpBodyNew(mass, cpMomentForBox(mass, size, size));
    auto shape = cpBoxShapeNew(body, size, size, 0.0f);
    auto shapeFilter = cpShapeFilterNew(CP_NO_GROUP, mobileType, mobileTypeCollideAgainst);

    cpBodySetPosition(body, position);
    cpShapeSetCollisionType(shape, mobileType);
    cpShapeSetFilter(shape, shapeFilter);

    body = cpSpaceAddBody(g_world.PhysicsWorld, body);
    cpSpaceAddShape(g_world.PhysicsWorld, shape);

    auto mobile = SMobile{
        .Body = body,
        .Shape = shape,
        .ShapeFilter = shapeFilter,
    };

    g_mobiles.push_back(mobile);

    if (mobileType == EMobileType::Player) {

        g_world.PlayerEntity = g_world.EntityRegistry.create();
        g_world.EntityRegistry.emplace<SPhysicsComponent>(g_world.PlayerEntity, body);
        g_world.EntityRegistry.emplace<SPlayerComponent>(g_world.PlayerEntity);
        g_world.EntityRegistry.emplace<SPositionComponent>(g_world.PlayerEntity, position);
        g_world.EntityRegistry.emplace<SColorComponent>(g_world.PlayerEntity, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});

        auto playerId = static_cast<uint32_t>(g_world.PlayerEntity);
        cpBodySetUserData(body, &playerId);

    } else if (mobileType == EMobileType::Enemy) {

        auto enemy = g_world.EntityRegistry.create();
        auto enemyId = static_cast<uint32_t>(enemy);
        g_world.EntityRegistry.emplace<SPhysicsComponent>(enemy, body);
        g_world.EntityRegistry.emplace<SEnemyComponent>(enemy, 100.0f);
        g_world.EntityRegistry.emplace<SPositionComponent>(enemy, position);
        g_world.EntityRegistry.emplace<SColorComponent>(enemy, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});

        cpBodySetUserData(body, &enemyId);
    }
}

auto InitializeLevel() -> void {

    constexpr float mass = 1.0f;

    AddMobile({0, 0}, EMobileType::Player, EMobileType::Enemy | EMobileType::Wall, 1.0f, 32.0f);

    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_real_distribution<float> dist(0, 800);

    const auto enemyIndices = std::ranges::iota_view{0, 30};
    for(auto enemyIndex : enemyIndices) {

        auto enemyPosition = cpv(-400.0f + dist(engine), -400.0f + dist(engine));
        AddMobile(enemyPosition, EMobileType::Enemy, EMobileType::Enemy | EMobileType::Player | EMobileType::Wall, 1.0f, 32.0f);
    }
}

auto OnHandlePlayerVsEnemyCollisionBegin(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> cpBool {

    if (userData != nullptr) {
        auto entity = *reinterpret_cast<uint32_t*>(userData);

        spdlog::info("{} PvsE Begin", entity);
    }

    return cpTrue;
}

auto OnHandlePlayerVsEnemyCollisionEnd(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> void {

    if (userData != nullptr) {
        auto entity = *reinterpret_cast<uint32_t*>(userData);

        spdlog::info("{} PvsE End", entity);
    }
}

auto OnHandleEnemyVsEnemyCollisionBegin(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> cpBool {

    if (userData != nullptr) {
        auto entity = *reinterpret_cast<uint32_t*>(userData);

        spdlog::info("{} EvsE Begin", entity);
    }

    return cpTrue;
}

auto OnHandleEnemyVsEnemyCollisionEnd(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> void {
    
    if (userData != nullptr) {
        auto entity = *reinterpret_cast<uint32_t*>(userData);

        spdlog::info("{} EvsE End", entity);
    }
}

auto InitializeWorld() -> void {

    g_world.PhysicsWorld = cpSpaceNew();
    cpSpaceSetDamping(g_world.PhysicsWorld, 0.2f);
    cpSpaceSetCollisionSlop(g_world.PhysicsWorld, 0.2f);

    g_world.CollisionHandlerPlayerVsEnemy = cpSpaceAddCollisionHandler(g_world.PhysicsWorld, EMobileType::Player, EMobileType::Enemy);
    g_world.CollisionHandlerPlayerVsEnemy->beginFunc = &OnHandlePlayerVsEnemyCollisionBegin;
    g_world.CollisionHandlerPlayerVsEnemy->postSolveFunc = &OnHandlePlayerVsEnemyCollisionEnd;

    g_world.CollisionHandlerEnemyVsEnemy = cpSpaceAddCollisionHandler(g_world.PhysicsWorld, EMobileType::Enemy, EMobileType::Enemy);
    g_world.CollisionHandlerEnemyVsEnemy->beginFunc = &OnHandleEnemyVsEnemyCollisionBegin;
    g_world.CollisionHandlerEnemyVsEnemy->postSolveFunc = &OnHandleEnemyVsEnemyCollisionEnd;

    InitializeLevel();
}

auto ShutdownWorld() -> void {

    for (auto& mobile : g_mobiles) {
        cpBodyDestroy(mobile.Body);
        cpShapeDestroy(mobile.Shape);
    }

    g_world.EntityRegistry.clear();
    cpSpaceFree(g_world.PhysicsWorld);
    g_world.PhysicsWorld = nullptr;
}