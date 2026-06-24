# Rift

Rift is a third-person online ARPG prototype built around LAN co-op combat.
The current playable path focuses on a multiplayer PvE co-op run: create or join
a LAN room, confirm characters in the lobby, clear the first level objective, then
defeat the boss together.

## Game Pitch

Players enter a corrupted rift zone as twin-sword fighters. The run is built around
readable melee combat, shared pressure, and short-session cooperation:
players survive enemy encounters, activate a teleport under attack, and finish the
run with a boss fight.

## Current Playable Flow

1. Start from `MainMenuMap`.
2. The host creates a LAN room with a room code.
3. Other players join the LAN room using the same code.
4. All connected players enter `LobbyMap`.
5. Each player previews and confirms their character appearance.
6. When every connected player is confirmed, the host starts the run.
7. The party enters `NormalLevel`.
8. Players find and activate the teleport while enemy waves pressure the area.
9. After activation, the teleport sends the party to `BossLevel`.
10. Players defeat the boss and use the victory popup to return to the main menu.

## Multiplayer Co-op Gameplay

Rift is designed as a host-client LAN session. The room host creates the session,
keeps authority over gameplay, and starts the run from the lobby. Joining players
connect by room code and travel with the host through the level flow.

The lobby is the preparation phase. Players can preview their character slot,
adjust appearance, and confirm when ready. The host cannot start the run until all
connected players have finished character creation, which keeps everyone synced
before map travel.

In `NormalLevel`, the core objective is the teleport. Once discovered, the teleport
starts an activation sequence with visible progress. Enemy waves are triggered by
activation progress, so the fight escalates while players decide whether to hold
position together, split target priority, or kite enemies away from the objective.

The best co-op rhythm is:

- One player holds enemy attention near the objective while the other clears weaker
  enemies or protects the flank.
- Players rotate guard, dodge, and combo pressure instead of everyone committing to
  long attacks at the same time.
- When a player is downed, the remaining players buy time until the respawn timer
  completes.
- During boss combat, players share pressure by attacking from different angles
  and responding to the boss phase change at low health.

## Combat

The current player class uses twin swords with fast melee pressure and cancel-driven
defense.

- `WASD`: Move.
- `Mouse`: Look.
- `Left Mouse Button`: Primary twin-sword combo.
- `Right Mouse Button`: RapidSlash finisher when RapidSlash is active.
- `Shift`: Dodge, including a perfect-dodge reward window.
- `Space`: Guard.
- `Q` / `E`: Skill inputs.

Combat features currently implemented:

- Combo graph based twin-sword attacks.
- RapidSlash state and finisher input.
- Guard and dodge action flow.
- Perfect dodge reward window.
- Hit feedback, blocked feedback, damage numbers, and impact cues.
- Ultimate charge resources from combat events.

## Enemy Encounters

Enemy encounters are rule-driven instead of manually fixed. Encounter triggers and
teleport waves use spawn budgets, weighted enemy pools, spawn limits, and configured
spawn points. This lets the same level support different pressure patterns without
rewriting gameplay logic.

Enemies support:

- Health and poise.
- Stagger from poise break.
- Shield block behavior.
- Hit retreat behavior.
- Behavior Tree driven target selection and melee attacks.
- Replicated health bars and combat feedback.

## Boss Fight

The boss encounter is the current run finale in `BossLevel`. The boss inherits the
enemy combat stack and adds a replicated combat phase. At low health, the boss enters
phase 2 and triggers its phase effect for all players. Defeating the boss clears
remaining enemies, broadcasts victory to every player, and opens the victory flow.

## Networking Model

The current multiplayer model is LAN listen-server:

- The host creates the LAN room and also runs the server authority.
- Joining players connect through room-code based LAN session discovery.
- Lobby readiness, selected appearance, map travel, player respawn, enemy state,
  boss phase, and victory are replicated through the gameplay framework.

This is not currently a dedicated-server package. The playable target is a shared
LAN co-op session where one player's machine hosts the run.

## Current Scope

Implemented:

- Main menu room creation and room-code joining.
- LAN lobby with host ownership, player slots, appearance confirmation, and start
  gating.
- Seamless lobby-to-gameplay flow.
- Multiplayer PvE progression through `NormalLevel` and `BossLevel`.
- Teleport activation objective with enemy waves.
- Twin-sword combat, guard, dodge, RapidSlash, damage feedback, and respawn.
- Boss phase change and victory return flow.
- Central audio subsystem for frontend music and UI sounds.

Out of scope for the current prototype:

- Dedicated server deployment.
- Online matchmaking outside LAN discovery.
- Persistent progression.
- Multiple playable classes.
- Full settings menu.

## External Assets and Credits

This prototype uses third-party art, animation, UI, audio, and visual effect assets
for educational and prototype purposes. Asset packs include stylized fantasy
environment assets, character and enemy animation packs, UI elements, sound effects,
and Niagara VFX.

Refer to the Unreal project `Content` folder and asset metadata for detailed asset
origins and licensing information.
