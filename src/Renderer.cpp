#include "Renderer.hpp"
#include "Components.hpp"

#include <Fwog/Buffer.h>
#include <Fwog/Context.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Rendering.h>
#include <Fwog/Shader.h>
#include <Fwog/Texture.h>

#include <glad/glad.h>
#include <debugbreak.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <iterator>

struct SGpuCameraInformation {
    glm::mat4x4 ProjectionMatrix;
    glm::mat4x4 ViewMatrix;
};

struct alignas(16) SGpuSprite {
    glm::vec4 PositionAndRotation;
    glm::vec4 Color;
};

std::optional<Fwog::GraphicsPipeline> g_graphicsPipeline = {};
std::optional<Fwog::Buffer> g_gpuCameraInformationBuffer = {};
std::optional<Fwog::TypedBuffer<SGpuSprite>> g_gpuSpriteBuffer = {};
std::optional<Fwog::Buffer> g_gpuSpriteTextureHandleBuffer = {};
std::optional<Fwog::Texture> g_frogTexture = {};
std::optional<Fwog::Sampler> g_defaultSampler = {};

SGpuCameraInformation g_gpuCameraInformation = {};

uint64_t g_frogTextureHandle = 0;
int32_t g_spriteCount = 0;

auto static OnDebugMessageCallback(
    [[maybe_unused]] uint32_t source,
    uint32_t type,
    [[maybe_unused]] uint32_t id,
    [[maybe_unused]] uint32_t severity, 
    [[maybe_unused]] int32_t length,
    const char* message,
    [[maybe_unused]] const void* userParam) -> void {

    if (type == GL_DEBUG_TYPE_ERROR) {
        spdlog::error(message);
        debug_break();
    }
}

auto LoadTextFromFile(std::string_view filePath) -> std::string {

    std::ifstream fileStream{ filePath.data() };
    std::string text { std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>() };
    return text;
};

auto LoadTextureFromFile(std::string_view filePath) -> std::optional<Fwog::Texture> {

    int32_t width = 0;
    int32_t height = 0;

    const auto pixelData = stbi_load(filePath.data(), &width, &height, nullptr, 4);
    if (!pixelData) {
        return {};
    }

    auto texture = Fwog::CreateTexture2D({
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)},
        Fwog::Format::R8G8B8A8_UNORM,
        filePath);
    
    Fwog::TextureUpdateInfo tui = {};
    tui.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    tui.format = Fwog::UploadFormat::RGBA;
    tui.type = Fwog::UploadType::UBYTE;
    tui.pixels = pixelData;
    texture.UpdateImage(tui);
    stbi_image_free(pixelData);

    return texture;
}

auto CreateGraphicsPipeline() -> std::optional<Fwog::GraphicsPipeline> {

    auto vertexShaderSource = LoadTextFromFile("data/shaders/sprite.vs.glsl");
    if (vertexShaderSource.empty()) {
        return {};
    }

    auto fragmentShaderSource = LoadTextFromFile("data/shaders/sprite.fs.glsl");
    if (fragmentShaderSource.empty()) {
        return {};
    }

    auto vertexShader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, vertexShaderSource, "VS:sprite.vs.glsl");
    auto fragmentShader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, fragmentShaderSource, "FS:sprite.fs.glsl");

    return Fwog::GraphicsPipeline
    {{
        .name = "Unlit Graphics Pipeline",
        .vertexShader = &vertexShader,
        .fragmentShader = &fragmentShader,
        .inputAssemblyState =
        {
            .topology = Fwog::PrimitiveTopology::TRIANGLE_STRIP,
        },
        .rasterizationState =
        {
            .polygonMode = Fwog::PolygonMode::FILL,
            .cullMode = Fwog::CullMode::BACK,
            .frontFace = Fwog::FrontFace::COUNTERCLOCKWISE,
        },
        .depthState =
        {
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthCompareOp = Fwog::CompareOp::LESS,
            
        },
        .colorBlendState =
        {
            .attachments = {{ 
                {
                    .blendEnable = true,
                    .dstColorBlendFactor = Fwog::BlendFactor::ONE_MINUS_SRC_ALPHA
                }
            }},
        }        
    }};
}

auto CreateBuffers() -> void {

    auto viewportWidth = 1920 / 2.0f;//g_application.Context.FramebufferSize.x / 2.0f;
    auto viewportHeight = 1080 / 2.0f;//g_application.Context.FramebufferSize.y / 2.0f;
    g_gpuCameraInformation.ProjectionMatrix = glm::orthoRH(-viewportWidth, viewportWidth, viewportHeight, -viewportHeight, -20.0f, 20.0f);

    auto cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    auto cameraDirection = glm::vec3(0, 0, -1);
    auto cameraUp = glm::vec3(0, 1, 0);
    g_gpuCameraInformation.ViewMatrix = glm::lookAtRH(cameraPosition, cameraPosition + cameraDirection, cameraUp);
    g_gpuCameraInformationBuffer = Fwog::Buffer(g_gpuCameraInformation, Fwog::BufferStorageFlag::DYNAMIC_STORAGE, "GpuCameraInformation");
    g_gpuCameraInformationBuffer->UpdateData(g_gpuCameraInformation, 0);

    g_defaultSampler = Fwog::Sampler(Fwog::SamplerState{
        .minFilter = Fwog::Filter::NEAREST,
        .magFilter = Fwog::Filter::NEAREST,
        .addressModeU = Fwog::AddressMode::REPEAT,
        .addressModeV = Fwog::AddressMode::REPEAT,
    });    

    std::array<SGpuSprite, 3> sprites = {{
        { glm::vec4(-200, 100, -10, 0), glm::vec4(1, 0, 0, 1) },
        { glm::vec4(100, 150, -10, 0), glm::vec4(1, 1, 0, 1) },
        { glm::vec4(200, -100, -9, 0), glm::vec4(0, 0, 1, 1) },
    }};

    g_gpuSpriteBuffer = Fwog::TypedBuffer<SGpuSprite>(1024, Fwog::BufferStorageFlag::DYNAMIC_STORAGE, "GpuSprites");

    std::vector<uint64_t> spriteTextureHandles;

    g_frogTexture = LoadTextureFromFile("data/sprites/frog.png");


    if (g_frogTexture.has_value()) {
        g_frogTextureHandle = g_frogTexture.value().GetBindlessHandle(g_defaultSampler.value());
        /*
        auto frogTextureHandle = g_frogTexture.value().GetBindlessHandle(g_defaultSampler.value());
        spriteTextureHandles.push_back(frogTextureHandle);
        spriteTextureHandles.push_back(frogTextureHandle);
        spriteTextureHandles.push_back(frogTextureHandle);
        */
    }

    g_gpuSpriteTextureHandleBuffer = Fwog::Buffer(1024, Fwog::BufferStorageFlag::DYNAMIC_STORAGE, "SGpuSpriteTextureHandles");
}

auto InitializeRenderer(bool isDebug) -> bool {

    if (gladLoadGL() == GL_FALSE) {
        spdlog::error("{} Unable to load OpenGL", "GameHost");
        return false;
    }

    auto fwogCallback = nullptr;
    Fwog::Initialize({
        .verboseMessageCallback = fwogCallback,
    });

    if (isDebug) {
        glDebugMessageCallback(OnDebugMessageCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    g_graphicsPipeline = CreateGraphicsPipeline();
    if (!g_graphicsPipeline) {
        spdlog::error("{} Unable to compile graphics pipeline", "FwogSurvivors");
        return false;
    }

    CreateBuffers();

    return true;
}

auto ShutdownRenderer() -> void {

    g_graphicsPipeline.reset();
    g_gpuCameraInformationBuffer.reset();
    g_gpuSpriteBuffer.reset();
    g_gpuSpriteTextureHandleBuffer.reset();
    g_frogTexture.reset();
    g_defaultSampler.reset();

    Fwog::Terminate();
}

auto UpdateGpuResources(const entt::registry& registry) -> void {

    auto positionView = registry.view<SPositionComponent, SColorComponent>();
    auto spriteIndex = 0;
    g_spriteCount = 0;
    positionView.each([&](auto& positionComponent, auto& colorComponent) {

        SGpuSprite gpuSprite = {
            .PositionAndRotation = glm::vec4(positionComponent.Position.x, positionComponent.Position.y, 0.0f, 1.0f),
            .Color = colorComponent.Color,
        };
        g_gpuSpriteBuffer->UpdateData(gpuSprite, spriteIndex);
        g_gpuSpriteTextureHandleBuffer->UpdateData(g_frogTextureHandle, sizeof(uint64_t) * spriteIndex);

        spriteIndex++;
        g_spriteCount++;
    });
}

auto RenderWorld(glm::ivec2 framebufferSize) -> void {

    Fwog::RenderToSwapchain(
        Fwog::SwapchainRenderInfo {
            .name = "RenderScene",
            .viewport = Fwog::Viewport {
                .drawRect {
                    .offset = {0, 0},
                    .extent = {
                        static_cast<uint32_t>(framebufferSize.x),
                        static_cast<uint32_t>(framebufferSize.y)
                    }
                },
                .depthRange = Fwog::ClipDepthRange::NEGATIVE_ONE_TO_ONE,
            },
            .colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
            .clearColorValue = {.2f, .4f, .1f, 1.0f},
            .depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
            .clearDepthValue = { 1.0f }
        },
    [&] {
        Fwog::Cmd::BindGraphicsPipeline(g_graphicsPipeline.value());
        Fwog::Cmd::BindUniformBuffer("SGpuCameraInformationBuffer", g_gpuCameraInformationBuffer.value(), 0, sizeof(SGpuCameraInformation));
        Fwog::Cmd::BindStorageBuffer("SGpuSpriteBuffer", g_gpuSpriteBuffer.value(), 0, Fwog::WHOLE_BUFFER);
        Fwog::Cmd::BindStorageBuffer("SGpuSpriteTextureHandleBuffer", g_gpuSpriteTextureHandleBuffer.value(), 0, Fwog::WHOLE_BUFFER);

        Fwog::Cmd::Draw(4, g_spriteCount, 0, 0);
    });
}