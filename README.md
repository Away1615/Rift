# Rift

Rift is a prototype third-person online ARPG developed with Unreal Engine 5.6.
The current progress includes a LAN lobby, character appearance confirmation, a teleport activation level with procedural enemy encounters, and a boss level with victory flow.

## Project Overview

Players create or join a LAN room from the main menu, confirm their character
appearance in the lobby, then enter the first gameplay level. In Level 1, players
find and activate a teleportation point while surviving enemy waves generated from
a budget-based weighted enemy pool. After activation, players travel to Level 2 and
fight the boss. Defeating the boss shows a victory popup and allows players to
return to the main menu.

## Main Features

- LAN room creation and joining through room codes.
- Lobby character appearance preview and confirmation.
- Dual-sword player combat with combo attacks, RapidSlash, dodge, and guard.
- Enemy AI using Behavior Trees and Blackboard.
- Enemy health, poise, stagger, death, and damage number feedback.
- Rule-based procedural enemy spawning using spawn budget, weighted enemy pool,
  and NavMesh-valid spawn positions.
- Teleport activation objective with progress feedback and enemy waves.
- Boss encounter with phase-based behaviour and victory flow.
- Player death screen, 10-second respawn countdown, and in-place revive.

## Requirements

- Unreal Engine 5.6
- Windows PC
- Visual Studio 2022 with C++ toolchain

Required Unreal plugins:

- GameplayAbilities
- OnlineSubsystemNull
- MotionWarping
- ModelingToolsEditorMode

## Setup

1. Clone or download the project.
2. Open `Rift.uproject` with Unreal Engine 5.6.
3. If prompted, rebuild project modules.
4. Open the main menu map:
   - `/Game/0_/Map/MainMenuMap`
5. Press Play in Editor.

## Build

Use UnrealBuildTool directly. Do not use `Build.bat`.

```powershell
& "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" RiftEditor Win64 Development -Project="C:\Dev\Projects\Rift\Rift.uproject" -WaitMutex
```

If Live Coding is active, close the editor or press `Ctrl+Alt+F11` before compiling.

## Playtesting Flow

Recommended test flow:

1. Start from `MainMenuMap`.
2. Create a LAN room.
3. Enter the Lobby.
4. Confirm character appearance.
5. Start the game as host.
6. Complete the Level 1 teleport activation objective.
7. Travel to Level 2.
8. Defeat the boss.
9. Use the victory popup to return to the main menu.

For multiplayer testing, use Play In Editor with multiple players and listen-server
network mode.

For PIE multiplayer map travel testing, enable seamless travel support for PIE:

```text
net.AllowPIESeamlessTravel 1
```

This can be entered in the Unreal console before testing Lobby-to-gameplay travel.

## Source Code Structure

- `Source/Rift/Public/Character`
  - Player, enemy, and boss character classes.
- `Source/Rift/Public/AbilitySystem`
  - Gameplay abilities, gameplay tags, attributes, and animation notify states.
- `Source/Rift/Public/AI`
  - Behavior Tree services, tasks, decorators, and enemy AI controller.
- `Source/Rift/Public/World`
  - Teleport objective and encounter trigger actors.
- `Source/Rift/Public/UI`
  - Menu, lobby, HUD, enemy health bar, and player HUD widget bases.
- `Source/Rift/Public/Data`
  - Ability configuration, enemy configuration, input configuration, and appearance data.

## External Assets and Credits

This prototype uses third-party art, animation, UI, audio, and visual effect assets
for educational purposes. Asset packs include stylized fantasy environment assets,
character and enemy animation packs, UI elements, sound effects, and Niagara VFX.

Please refer to the Unreal project `Content` folder and asset metadata for detailed
asset origins and licensing information.