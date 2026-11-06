set(BOX3D_GAME_SOURCES
    game/enemies/zombie.cpp
    game/enemies/zombie.hpp
    game/global.hpp
    game/components/health.hpp
    game/player/movement.hpp
    game/player/player.cpp
    game/player/player.hpp
    game/systems/zombieManager.cpp
    game/systems/zombieManager.hpp
    game/utils.hpp
    game/weapons.hpp
    game/world/entities.hpp
    game/world/world.cpp
    game/world/world.hpp
)

# Make project headers importable from the src root (for example,
# #include <engine/engine.hpp> from game code).
target_include_directories(Box3dEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(Box3dEngine
    PRIVATE
        engine/physics/physics.hpp
        engine/graphics/graphics.hpp
        engine/world/cubeBrush.hpp
        engine/world/entity.hpp
        engine/world/entityRegistry.cpp
        engine/world/entityRegistry.hpp
        engine/world/quakeBsp.cpp
        engine/world/quakeBsp.hpp
        engine/world/quakePalette.hpp
        engine/world/quakeUtil.hpp
        engine/assetManager.hpp
        engine/engine.cpp
        engine/engine.hpp
        engine/eventListener.hpp
        engine/global.hpp
        engine/shaderManager.hpp
)

target_sources(Box3dGame PRIVATE game/main.cpp ${BOX3D_GAME_SOURCES})
target_sources(Box3dEditor PRIVATE ${BOX3D_GAME_SOURCES}
    Debug/debugger.cpp
    Debug/debugger.hpp
    Editor/editor.cpp
    Editor/editor.hpp
    Editor/main.cpp
)
