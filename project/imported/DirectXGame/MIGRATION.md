# DirectXGame Migration

This folder contains a raw import of the source project from:

`C:\Users\k023g\source\repos\就職作品\DirectXGame\DirectXGame`

It is intentionally not part of the current KuboEngine build yet.

## Imported Scope

- `core`: 5 files
- `effects`: 12 files
- `enemy`: 6 files
- `items`: 2 files
- `math`: 1 file
- `notes`: 7 files
- `player`: 11 files
- `scene`: 16 files
- `ui`: 24 files
- `world`: 4 files
- `Resources`: 136 files

## Current Status

The imported game is written against `KamataEngine`.
KuboEngine already has overlapping systems, but the APIs are not compatible.
The code is imported as reference and porting source, not as build-ready code.

## Main Dependency Gaps

The imported game expects these engine-side features that KuboEngine does not currently expose with compatible APIs:

1. `WorldTransform`
2. `LightGroup`
3. `ObjectColor`
4. shared `Model::CreateFromOBJ` style loading
5. sprite factory-style creation and direct texture handles
6. singleton-style `DirectXCommon`, `Input`, `Audio`, `TextureManager`, `WinApp`
7. scene API with `Update(float deltaTime)`
8. richer input abstraction in `InputBindings.h`

## Recommended Port Order

1. Port `core/GameSessionContext.h` and scene result data flow.
2. Port `core/InputBindings.h` onto KuboEngine input APIs.
3. Port `player/core` and `player/weapons` onto `Object3D`, `Camera`, `Audio`, and existing particle systems.
4. Port `enemy` and collision logic.
5. Port `ui/common`, `ui/gauge`, and `ui/hud`.
6. Port `scene/game`.
7. Port `scene/title` and `scene/result`.
8. Port `world` and remaining effects.

## Migration Strategy

Do not try to make the imported source compile unchanged.
The practical path is:

1. Keep `imported/DirectXGame` as a frozen reference.
2. Recreate equivalent runtime classes inside KuboEngine under new modules.
3. Reuse imported assets and external data files.
4. Add thin compatibility helpers only where they reduce rewrite cost.

## Candidate External Libraries

If we expand the imported systems, these are reasonable additions:

- `nlohmann/json`: scene save/load and editor serialization
- `yaml-cpp`: prefab or authoring-oriented data if YAML is preferred
- `entt`: if we want a real ECS instead of the current minimal `GameObject/Component`
- `ImGuizmo`: transform gizmos for editor workflows

## Immediate Next Step

The next practical implementation step is to port `GameSessionContext` and `InputBindings` into KuboEngine-native code, then build a new gameplay scene around those APIs.
