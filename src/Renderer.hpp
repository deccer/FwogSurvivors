#pragma once

#include <entt/entt.hpp>
#include <glm/vec2.hpp>

auto InitializeRenderer(bool isDebug) -> bool;
auto ShutdownRenderer() -> void;

auto UpdateGpuResources(const entt::registry& registry) -> void;
auto RenderWorld(glm::ivec2 framebufferSize) -> void;