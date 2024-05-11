#include "World.hpp"
#include "Components.hpp"

#include <random>
#include <ranges>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

SWorld g_world = {};

enum CollisionType : uint32_t {
    Player = 1,
    Enemy = 2,
    Wall = 4,
    Bullet = 8
};

auto InitializeLevel() -> void {

    g_world.PlayerEntity = g_world.EntityRegistry.create();
    auto playerBody = cpSpaceAddBody(g_world.PhysicsWorld, cpBodyNew(1.0, cpMomentForBox(1.0, 32.0, 32.0)));
    cpBodySetType(playerBody, cpBodyType::CP_BODY_TYPE_DYNAMIC);
    auto playerShape = cpBoxShapeNew(playerBody, 32.0f, 32.0f, 0.0f);
    cpShapeSetCollisionType(playerShape, CollisionType::Player);
    cpShapeSetFilter(playerShape, cpShapeFilterNew(CP_NO_GROUP, CollisionType::Player, CollisionType::Enemy | CollisionType::Wall));
    cpSpaceAddShape(g_world.PhysicsWorld, playerShape);
    cpBodySetPosition(playerBody, cpv(0, 0));
    g_world.EntityRegistry.emplace<SPhysicsComponent>(g_world.PlayerEntity, playerBody);
    g_world.EntityRegistry.emplace<SPlayerComponent>(g_world.PlayerEntity);
    g_world.EntityRegistry.emplace<SPositionComponent>(g_world.PlayerEntity, cpVect{0, 0});
    g_world.EntityRegistry.emplace<SColorComponent>(g_world.PlayerEntity, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});

    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_real_distribution<float> dist(0, 800);

    const auto enemyIndices = std::ranges::iota_view{0, 30};
    for(auto enemyIndex : enemyIndices) {

        auto position = cpv(-400.0f + dist(engine), -400.0f + dist(engine));
        auto enemy = g_world.EntityRegistry.create();
        auto enemyBody = cpSpaceAddBody(g_world.PhysicsWorld, cpBodyNew(1.0, cpMomentForBox(1.0, 32.0, 32.0)));
        cpBodySetType(enemyBody, cpBodyType::CP_BODY_TYPE_DYNAMIC);
        auto enemyShape = cpBoxShapeNew(enemyBody, 32.0f, 32.0f, 0.0f);
        cpShapeSetCollisionType(enemyShape, CollisionType::Enemy);
        cpShapeSetFilter(enemyShape, cpShapeFilterNew(CP_NO_GROUP, CollisionType::Enemy, CollisionType::Enemy | CollisionType::Player | CollisionType::Wall));
        cpSpaceAddShape(g_world.PhysicsWorld, enemyShape);
        cpBodySetPosition(enemyBody, position);
        g_world.EntityRegistry.emplace<SPhysicsComponent>(enemy, enemyBody);
        g_world.EntityRegistry.emplace<SEnemyComponent>(enemy, 100.0f);
        g_world.EntityRegistry.emplace<SPositionComponent>(enemy, position);
        g_world.EntityRegistry.emplace<SColorComponent>(enemy, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    }

}

auto OnHandlePlayerVsEnemyCollisionBegin(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> cpBool {

    return cpTrue;
}

auto OnHandlePlayerVsEnemyCollisionEnd(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> void {
    
}

auto OnHandleEnemyVsEnemyCollisionBegin(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> cpBool {

    return cpTrue;
}

auto OnHandleEnemyVsEnemyCollisionEnd(
    cpArbiter* arbiter,
    cpSpace* space,
    cpDataPointer userData) -> void {
    
}

auto InitializeWorld() -> void {

    g_world.PhysicsWorld = cpSpaceNew();

    auto collisionHandlerPlayerVsEnemy = cpSpaceAddCollisionHandler(g_world.PhysicsWorld, CollisionType::Player, CollisionType::Enemy);
    collisionHandlerPlayerVsEnemy->beginFunc = &OnHandlePlayerVsEnemyCollisionBegin;
    collisionHandlerPlayerVsEnemy->postSolveFunc = &OnHandlePlayerVsEnemyCollisionEnd;

    auto collisionHandlerEnemyVsEnemy = cpSpaceAddCollisionHandler(g_world.PhysicsWorld, CollisionType::Enemy, CollisionType::Enemy);
    collisionHandlerEnemyVsEnemy->beginFunc = &OnHandleEnemyVsEnemyCollisionBegin;
    collisionHandlerEnemyVsEnemy->postSolveFunc = &OnHandleEnemyVsEnemyCollisionEnd;

    InitializeLevel();
}

auto ShutdownWorld() -> void {

    g_world.EntityRegistry.clear();
    cpSpaceFree(g_world.PhysicsWorld);
    g_world.PhysicsWorld = nullptr;
}