# DirectXGame Port Status

## Working Ports

- `GameSessionContext`
- `InputBindings` for keyboard, mouse, and XInput pad

## Stored But Not Yet Ported

These remain in `imported/DirectXGame` as source references:

- `scene/game/GameScene`
- `player/core`
- `enemy`
- `ui`
- `world`
- `effects`

## Known Missing Runtime Features

The original project expects features that KuboEngine does not currently expose:

1. DirectInput joystick enumeration and previous-state access
2. `WorldTransform`
3. `LightGroup`
4. `ObjectColor`
5. factory-style shared model loading
6. singleton engine services
7. delta-time driven scene contract

## Policy

If a source file cannot run on current KuboEngine APIs, keep it in `imported/DirectXGame` and port only the reusable logic first.
