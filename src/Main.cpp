#include "Application.hpp"
#include "Renderer.hpp"
#include "Components.hpp"
#include "World.hpp"

#include <spdlog/spdlog.h>

constexpr std::string_view g_gameTitle = "FwogSurvivors";

auto Initialize() -> bool {

    if (!InitializeRenderer(g_application.Configuration.IsDebug)) {

        return false;
    }

    InitializeWorld();
    
    return true;
}

auto Shutdown() -> void {

    ShutdownWorld();
    ShutdownRenderer();
    ShutdownApplication();
}

int32_t main(
    [[maybe_unused]] int32_t argc,
    [[maybe_unused]] char* argv[]) {

    if (!InitializeApplication({
        .Width = 1920,
        .Height = 1080,
        .Title = g_gameTitle,
        .ResolutionScale = 1.0f,
        .WindowStyle = EWindowStyle::Windowed,
        .IsDebug = true,
        .IsVSyncEnabled = true
    })) {
        spdlog::error("{} Unable to initialize", g_gameTitle);
        Shutdown();
        return -1;
    }
    if (!Initialize()) {
        spdlog::error("{} Unable to initialize game", g_gameTitle);
    }
    spdlog::info("{} Initialized", g_gameTitle);

    RunApplication();

    spdlog::info("{} shutting down", g_gameTitle);

    Shutdown();

    return 0;
}
