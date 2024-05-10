#include "World.hpp"
#include "Components.hpp"

#include <random>
#include <ranges>

SWorld g_world = {};

auto InitializeLevel() -> void {

    g_world.PlayerEntity = g_world.EntityRegistry.create();
    auto playerBody = cpSpaceAddBody(g_world.PhysicsWorld, cpBodyNew(1.0, cpMomentForBox(1.0, 32.0, 32.0)));
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
        cpBodySetPosition(enemyBody, position);
        g_world.EntityRegistry.emplace<SPhysicsComponent>(enemy, enemyBody);
        g_world.EntityRegistry.emplace<SEnemyComponent>(enemy, 100.0f);
        g_world.EntityRegistry.emplace<SPositionComponent>(enemy, position);
        g_world.EntityRegistry.emplace<SColorComponent>(enemy, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    }

}

auto InitializeWorld() -> void {

    g_world.PhysicsWorld = cpSpaceNew();

    InitializeLevel();
}

auto ShutdownWorld() -> void {

    g_world.EntityRegistry.clear();
    if (g_world.PhysicsWorld != nullptr) {
        cpSpaceDestroy(g_world.PhysicsWorld);
        g_world.PhysicsWorld = nullptr;
    }    
}