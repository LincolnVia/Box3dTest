set(BOX3D_GAME_SOURCES
    game/enemies/zombie.cpp
    game/enemies/zombie.hpp
    game/graphics.hpp
    game/global.hpp
    game/components/health.hpp
    game/physics/physics.hpp
    game/player/movement.hpp
    game/player/player.cpp
    game/player/player.hpp
    game/systems/zombieManager.cpp
    game/systems/zombieManager.hpp
    game/utils.hpp
    game/weapons.hpp
    game/world/cubeBrush.hpp
    game/world/entities.hpp
    game/world/entityRegistry.cpp
    game/world/entityRegistry.hpp
    game/world/quakeBsp.cpp
    game/world/quakeBsp.hpp
    game/world/quakePalette.hpp
    game/world/quakeUtil.hpp
    game/world/world.cpp
    game/world/world.hpp
)

target_sources(Box3dEngine
    PRIVATE
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
