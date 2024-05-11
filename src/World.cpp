#include "World.hpp"
#include "Components.hpp"

#include <random>
#include <ranges>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/vec2.hpp>

SWorld g_world = {};

class Foo : public b2ContactListener {
public:
    auto BeginContact(b2Contact* contact) -> void {
        auto userData1 = contact->GetFixtureA()->GetBody()->GetUserData();
        auto userData2 = contact->GetFixtureB()->GetBody()->GetUserData();


        //auto entityId = static_cast<uint32_t>(g_world.PlayerEntity);
        //bodyDefinition.userData.pointer = reinterpret_cast<uintptr_t>(&entityId);

        if (userData1.pointer != 0 && userData2.pointer != 0) {
            auto entity1 = reinterpret_cast<entt::entity>(static_cast<uint32_t>(userData1.pointer));
            auto entity2 = reinterpret_cast<entt::entity>(static_cast<uint32_t>(userData2.pointer));
        }
    }

    auto EndContact(b2Contact* contact) -> void {

    }
};

Foo g_foo;

b2ContactFilter g_playerVsEnemyContactFilter = {};
b2ContactFilter g_enemyVsEnemyContactFilter = {};

enum EMobileType : uint32_t {
    Player = 1,
    Enemy = 2,
    Wall = 4,
    Bullet = 8
};

struct SMobile {

    b2Body* Body;
    b2PolygonShape* Shape;
    b2Filter ShapeFilter = {};
};

std::vector<SMobile> g_mobiles = {};

auto AddMobile(
    b2Vec2 position,
    EMobileType mobileType,
    uint32_t mobileTypeCollideAgainst,
    float mass,
    float size) -> void {

    b2BodyDef bodyDefinition = {};
    bodyDefinition.position = position;
    bodyDefinition.type = b2BodyType::b2_dynamicBody;

    b2PolygonShape shape;
    shape.SetAsBox(size * 0.5f, size * 0.5f);

    b2FixtureDef fixtureDefinition;
    fixtureDefinition.shape = &shape;
    fixtureDefinition.density = mass;
    fixtureDefinition.friction = 0.0f;
    fixtureDefinition.restitution = 0.0f;
    fixtureDefinition.filter.groupIndex = 0;    
    fixtureDefinition.filter.categoryBits = mobileType;
    fixtureDefinition.filter.maskBits = mobileTypeCollideAgainst;

    auto body = g_world.PhysicsWorld.CreateBody(&bodyDefinition);
    body->CreateFixture(&fixtureDefinition);

    auto mobile = SMobile{
        .Body = body,
    };

    g_mobiles.push_back(mobile);

    if (mobileType == EMobileType::Player) {

        g_world.PlayerEntity = g_world.EntityRegistry.create();
        g_world.EntityRegistry.emplace<SPhysicsComponent>(g_world.PlayerEntity, body);
        g_world.EntityRegistry.emplace<SPlayerComponent>(g_world.PlayerEntity);
        g_world.EntityRegistry.emplace<SPositionComponent>(g_world.PlayerEntity, position);
        g_world.EntityRegistry.emplace<SColorComponent>(g_world.PlayerEntity, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});

        auto entityId = static_cast<uint32_t>(g_world.PlayerEntity);
        bodyDefinition.userData.pointer = reinterpret_cast<uintptr_t>(&entityId);

    } else if (mobileType == EMobileType::Enemy) {

        auto enemy = g_world.EntityRegistry.create();
        auto enemyId = static_cast<uint32_t>(enemy);
        g_world.EntityRegistry.emplace<SPhysicsComponent>(enemy, body);
        g_world.EntityRegistry.emplace<SEnemyComponent>(enemy, 100.0f);
        g_world.EntityRegistry.emplace<SPositionComponent>(enemy, position);
        g_world.EntityRegistry.emplace<SColorComponent>(enemy, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});

        auto entityId = static_cast<uint32_t>(enemy);
        bodyDefinition.userData.pointer = reinterpret_cast<uintptr_t>(&entityId);        
    }
}

auto InitializeLevel() -> void {

    AddMobile({0, 0}, EMobileType::Player, EMobileType::Enemy | EMobileType::Wall, 10000.0f, 32.0f);

    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_real_distribution<float> dist(0, 800);

    const auto enemyIndices = std::ranges::iota_view{0, 400};
    for(auto enemyIndex : enemyIndices) {

        auto enemyPosition = b2Vec2(-800.0f + dist(engine), -800.0f + dist(engine));
        AddMobile(enemyPosition, EMobileType::Enemy, EMobileType::Enemy | EMobileType::Player | EMobileType::Wall, 10.0f, 32.0f);
    }
}

auto InitializeWorld() -> void {

    g_world.PhysicsWorld.SetContactListener(&g_foo);
    
    InitializeLevel();
}

auto ShutdownWorld() -> void {

    for (auto& mobile : g_mobiles) {
        g_world.PhysicsWorld.DestroyBody(mobile.Body);
    }

    g_world.EntityRegistry.clear();
}