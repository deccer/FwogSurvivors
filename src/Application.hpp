#pragma once

#include <glm/vec2.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

struct GLFWwindow;

enum class EWindowStyle {
    Windowed,
    Fullscreen,
    FullscreenExclusive
};

struct SApplicationConfiguration {
    uint32_t Width;
    uint32_t Height;
    std::string_view Title;
    float ResolutionScale;
    EWindowStyle WindowStyle;
    bool IsDebug;
    bool IsVSyncEnabled;
};

struct SApplicationContext {
    glm::ivec2 ScreenSize;
    glm::ivec2 WindowSize;
    glm::ivec2 FramebufferSize;
    bool IsFramebufferResized;
    bool IsWindowResized;
};

struct SApplication {
    GLFWwindow* Window = nullptr;
    SApplicationConfiguration Configuration = {};
    SApplicationContext Context = {};
    glm::vec2 CursorPosition = {};
    bool IsWindowFocused = true;
};

extern SApplication g_application;

auto InitializeApplication(const SApplicationConfiguration& applicationConfiguration) -> bool;
auto ShutdownApplication() -> void;
auto RunApplication() -> void;