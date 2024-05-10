include(FetchContent)

#- chipmunk -----------------------------------------------------------------------------------------------------------

option(BUILD_DEMOS "Build the demo applications" OFF)
option(INSTALL_DEMOS "Install the demo applications" OFF)
option(BUILD_SHARED "Build and install the shared library" OFF)
option(BUILD_STATIC "Build as static library" ON)
option(INSTALL_STATIC "Install the static library" OFF)
FetchContent_Declare(
    chipmunk
    GIT_REPOSITORY https://github.com/slembcke/Chipmunk2D.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE    
)
message("Fetching chipmunk")
FetchContent_MakeAvailable(chipmunk)

#- fwog ---------------------------------------------------------------------------------------------------------------

option(FWOG_BUILD_EXAMPLES FALSE)
FetchContent_Declare(
    fwog
    GIT_REPOSITORY https://github.com/JuanDiegoMontoya/Fwog.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE    
)
message("Fetching fwog")
FetchContent_MakeAvailable(fwog)

#- spdlog -------------------------------------------------------------------------------------------------------------

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.10.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

message("Fetching spdlog")
FetchContent_MakeAvailable(spdlog)

#- tracy --------------------------------------------------------------------------------------------------------------

FetchContent_Declare(
    tracy
    GIT_REPOSITORY  https://github.com/wolfpld/tracy.git
    GIT_TAG         master
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
)

set(TRACY_ENABLE ON CACHE BOOL "Enable profiling")
#set(TRACY_NO_SYSTEM_TRACING ON CACHE BOOL "Disable System Tracing")
set(TRACY_ONLY_IPV4 ON CACHE BOOL "IPv4 only")
message("Fetching tracy")
FetchContent_MakeAvailable(tracy)

#- imgui --------------------------------------------------------------------------------------------------------------

if(TARGET imgui)
message("already fetched imgui")
else()
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        v1.88
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    message("Fetching imgui")
    FetchContent_Populate(imgui)

    add_library(imgui
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)

    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${glfw_SOURCE_DIR}/include)

    target_link_libraries(imgui PRIVATE glfw)
endif()
endif()

#- glm ----------------------------------------------------------------------------------------------------------------

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

message("Fetching glm")
FetchContent_MakeAvailable(glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_SILENT_WARNINGS)

#- stb ---------------------------------------------------------------------------------------------------------------

FetchContent_Declare(
    stb
    GIT_REPOSITORY  https://github.com/nothings/stb.git
    GIT_TAG         master
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
)
FetchContent_GetProperties(stb)
if(NOT stb_image_POPULATED)
    FetchContent_Populate(stb)
    message("Fetching stb")

    add_library(stb INTERFACE ${stb_SOURCE_DIR}/stb_image.h)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endif()

#- glad --------------------------------------------------------------------------------------------------------------

if(TARGET glad)
message("already fetched glad")
else()
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    message("Fetching glad")
    FetchContent_Populate(glad)
    set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile")
    set(GLAD_API "gl=4.6" CACHE STRING "API type/version pairs, like \"gl=4.6\", no version means latest")
    set(GLAD_GENERATOR "c" CACHE STRING "Language to generate the binding for")
    set(GLAD_EXTENSIONS "GL_ARB_bindless_texture" CACHE STRING "Extensions to take into consideration when generating the bindings")
    add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR})
endif()
endif()

#- debugbreak --------------------------------------------------------------------------------------------------------

FetchContent_Declare(
    debugbreak
    GIT_REPOSITORY https://github.com/scottt/debugbreak
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(debugbreak)
if(NOT debugbreak_POPULATED)
    message("Fetching debugbreak")
    FetchContent_Populate(debugbreak)

    add_library(debugbreak INTERFACE ${debugbreak_SOURCE_DIR}/debugbreak.h)
    target_include_directories(debugbreak INTERFACE ${debugbreak_SOURCE_DIR})
endif()

#- entt ---------------------------------------------------------------------------------------------------------------

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.13.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

message("Fetching entt")
FetchContent_MakeAvailable(entt)

