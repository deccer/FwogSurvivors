#include "Application.hpp"
#include "Renderer.hpp"
#include "Components.hpp"
#include "World.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "b2_user_settings.h"
#include <box2d/box2d.h>

SApplication g_application = {};

int32_t g_velocityIterations = 6;
int32_t g_positionIterations = 2;

auto static OnCursorPositionCallback(
    [[maybe_unused]] GLFWwindow* window,
    double currentCursorX,
    double currentCursorY) -> void {

    g_application.CursorPosition = {static_cast<float>(currentCursorX), static_cast<float>(currentCursorY)};
}

auto static OnCursorEnterCallback(
    [[maybe_unused]] GLFWwindow* window,
    int32_t entered) -> void {

    g_application.IsWindowFocused = entered == 1;
}

auto static OnFramebufferResizeCallback(
    [[maybe_unused]] GLFWwindow* window,
    int32_t newWidth,
    int32_t newHeight) -> void {

    g_application.Context.IsFramebufferResized = true;
    g_application.Context.FramebufferSize = glm::ivec2{newWidth, newHeight};
}

auto InitializeApplication(const SApplicationConfiguration& applicationConfiguration) -> bool {

    g_application.Configuration = std::move(applicationConfiguration);

    if (glfwInit() == GLFW_FALSE) {
        spdlog::error("{} Unable to Initialize", "GLFW");
        return false;
    }

    const auto isWindowWindowed = g_application.Configuration.WindowStyle == EWindowStyle::Windowed;
    glfwWindowHint(GLFW_DECORATED, isWindowWindowed ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, isWindowWindowed ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    if (g_application.Configuration.IsDebug) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    auto primaryMonitor = glfwGetPrimaryMonitor();
    auto primaryMonitorResolution = glfwGetVideoMode(primaryMonitor);

    auto monitor = g_application.Configuration.WindowStyle == EWindowStyle::FullscreenExclusive
        ? primaryMonitor
        : nullptr;

    g_application.Context.ScreenSize = {
        primaryMonitorResolution->width,
        primaryMonitorResolution->height
    };
    if (g_application.Configuration.WindowStyle != EWindowStyle::Windowed) {
        g_application.Context.WindowSize = g_application.Context.ScreenSize;
    } else {
        g_application.Context.WindowSize = {
            g_application.Configuration.Width,
            g_application.Configuration.Height
        };
    }

    g_application.Window = glfwCreateWindow(
        g_application.Context.WindowSize.x,
        g_application.Context.WindowSize.y,
        g_application.Configuration.Title.data(),
        monitor,
        nullptr);
    if (g_application.Window == nullptr) {
        spdlog::error("{} Unable to create a window", "GameHost");
        return false;
    }

    glfwMakeContextCurrent(g_application.Window);
    glfwSwapInterval(g_application.Configuration.IsVSyncEnabled ? 1 : 0);

    int32_t monitorLeft = 0;
    int32_t monitorTop = 0;
    glfwGetMonitorPos(primaryMonitor, &monitorLeft, &monitorTop);
    if (isWindowWindowed) {
        glfwSetWindowPos(
            g_application.Window,
            g_application.Context.ScreenSize.x / 2 - g_application.Context.WindowSize.x / 2 + monitorLeft, 
            g_application.Context.ScreenSize.y / 2 - g_application.Context.WindowSize.y / 2 + monitorTop);
    } else {
        glfwSetWindowPos(
            g_application.Window, 
            monitorLeft,
            monitorTop);
        glfwSetWindowSize(
            g_application.Window,
            g_application.Context.WindowSize.x,
            g_application.Context.WindowSize.y
        );
    }

    glfwGetFramebufferSize(
        g_application.Window,
        &g_application.Context.FramebufferSize.x,
        &g_application.Context.FramebufferSize.y);

    glfwSetCursorPosCallback(g_application.Window, OnCursorPositionCallback);
    glfwSetCursorEnterCallback(g_application.Window, OnCursorEnterCallback);
    glfwSetFramebufferSizeCallback(g_application.Window, OnFramebufferResizeCallback);

    return true;
}

auto ShutdownApplication() -> void {

    if (g_application.Window != nullptr) {
        glfwDestroyWindow(g_application.Window);
    }
    glfwTerminate();
    g_application.Window = nullptr;
}

auto HandleResize() -> void {

    if (g_application.Context.IsWindowResized) {
        
    }

    if (g_application.Context.IsFramebufferResized) {

    }
}

auto HandleInput(b2Body* playerBody) -> void {

    const float playerSpeed = 5.0f;

    b2Vec2 velocity = b2Vec2_zero;

    auto keyW = glfwGetKey(g_application.Window, GLFW_KEY_W);
    auto keyS = glfwGetKey(g_application.Window, GLFW_KEY_S);
    auto keyA = glfwGetKey(g_application.Window, GLFW_KEY_A);
    auto keyD = glfwGetKey(g_application.Window, GLFW_KEY_D);
    
    if (keyW == GLFW_PRESS || keyW == GLFW_REPEAT) {
        velocity.y -= playerSpeed;
    }
    if (keyS == GLFW_PRESS || keyS == GLFW_REPEAT) {
        velocity.y += playerSpeed;
    }
    if (keyA == GLFW_PRESS || keyA == GLFW_REPEAT) {
        velocity.x -= playerSpeed;
    }
    if (keyD == GLFW_PRESS || keyD == GLFW_REPEAT) {
        velocity.x += playerSpeed;
    }

    if (velocity.x != 0 || velocity.y != 0) {
        auto position = playerBody->GetPosition();
        velocity *= playerSpeed;
        playerBody->SetTransform(position + velocity, 0.0f);
    }
}

auto UpdateWorld(entt::registry& registry, b2World& physicsWorld, float physicsDeltaTime) -> void {

    physicsWorld.Step(physicsDeltaTime, g_velocityIterations, g_positionIterations);

    auto playerView = registry.view<SPhysicsComponent, SPlayerComponent, SPositionComponent>();
    const auto [playerPhysicsComponent, playerComponent, playerPositionComponent] = playerView.get(playerView.front());
    playerPositionComponent.Position = playerPhysicsComponent.Body->GetPosition();

    auto enemyView = registry.view<SPhysicsComponent, SEnemyComponent, SPositionComponent>();
    enemyView.each([&](auto& enemyPhysicsComponent, auto& enemyComponent, auto& enemyPositionComponent) {

        b2Vec2 playerPosition = playerPhysicsComponent.Body->GetPosition();
        b2Vec2 enemyPosition = enemyPhysicsComponent.Body->GetPosition();
        b2Vec2 playerEnemyDirection = playerPosition - enemyPosition;
        playerEnemyDirection.Normalize();

        b2Vec2 newEnemyVelocity = playerEnemyDirection;
        newEnemyVelocity *= enemyComponent.Speed;
        enemyPhysicsComponent.Body->SetLinearVelocity(newEnemyVelocity);

        enemyPhysicsComponent.PreviousPosition = enemyPhysicsComponent.CurrentPosition;
        enemyPhysicsComponent.CurrentPosition = enemyPosition;

/*
        cpVect interpolatedPosition = cpvlerp(
            enemyPhysicsComponent.PreviousPosition,
            enemyPhysicsComponent.CurrentPosition,
            physicsDeltaTime);
        enemyPositionComponent.Position = interpolatedPosition;
*/
        enemyPositionComponent.Position = enemyPosition;
    });
}

auto RunApplication() -> void {

    auto lastPhysicsUpdateTime = 0.0f;
    auto currentTime = glfwGetTime();
    auto accumulator = 0.0f;
    auto physicsDeltaTime = 1.0f / 60.0f;

    while (!glfwWindowShouldClose(g_application.Window)) {

        HandleResize();

        auto newTime = glfwGetTime();
        auto frameTime = newTime - currentTime;
        currentTime = newTime;
        accumulator += frameTime;

        HandleInput(g_world.EntityRegistry.get<SPhysicsComponent>(g_world.PlayerEntity).Body);
        
        while (accumulator >= physicsDeltaTime) {
            UpdateWorld(g_world.EntityRegistry, g_world.PhysicsWorld, physicsDeltaTime);

            accumulator -= physicsDeltaTime;
            lastPhysicsUpdateTime = newTime;
        }

        UpdateGpuResources(g_world.EntityRegistry);
        RenderWorld(g_application.Context.FramebufferSize);

        glfwSwapBuffers(g_application.Window);
        glfwPollEvents();

        if (!g_application.IsWindowFocused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

}