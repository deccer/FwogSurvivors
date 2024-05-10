add_custom_target(copy_data 
    ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data
)

add_executable(FwogSurvivors
    Application.cpp
    Renderer.cpp
    World.cpp
    Main.cpp
)
add_dependencies(FwogSurvivors copy_data)

target_link_libraries(FwogSurvivors
    PRIVATE glfw
    PRIVATE glad
    PRIVATE glm
    PRIVATE spdlog
    PRIVATE fwog
    PRIVATE TracyClient
    PRIVATE debugbreak
    PRIVATE stb
    PRIVATE EnTT
    PRIVATE chipmunk_static
    PRIVATE imgui
)
target_include_directories(FwogSurvivors PRIVATE ${chipmunk_SOURCE_DIR}/include)
