Publication note: This document records the original workspace build. Reference media and local rollback archives mentioned below are not included in this public repository. See [current build requirements](../docs/BUILDING.md) and [asset inputs](assets/README.md).

# BattleStar - Arcade Assault (01.003.000)

Build output: `../dist/PPSA99304/`. Copy the complete PPSA99304 folder to the USB location your loader already uses, replacing the previous BattleStar folder. Keep its assets and sce_sys folders alongside eboot.bin. For ShadowMountPlus, use /data/homebrew/PPSA99304/ internally or homebrew/PPSA99304/ at the USB drive root. Avoid an extra dist folder. The package includes the required 512 x 512 sce_sys/icon0.png; registration fails if this is missing. Title ID remains PPSA99304; KB-RPCS3 uses a separate ID.

The game keeps the original Viper/title artwork, uses a 1920 x 1080 logical playfield on a 3840 x 2160 render surface, and adds the Raider, capital ship, projectile and starfield assets from BattleStar-OSLib. This iteration is a complete short arcade run: three sectors, twelve authored encounters, and capital ships on waves 4, 8 and 12. Clear the command ship to reach the victory/results screen. Each cleared wave repairs 10 hull and awards 250 points; victory adds 2,500 points.

| Control | Action |
| --- | --- |
| Cross | Launch / hold to fire / retry from results |
| R1 | Hold to fire |
| Left stick / D-pad | Move |
| R2 | Boost |
| Options | Pause / resume |
| Circle | Return to title |
| Triangle | Retry from results |
| Hold Options 1.25 seconds | Exit |

Music crossfades between the reference title theme and MrDeath's combat module. The player now uses an original Viper-inspired twin-cannon sound: a sharp energy sweep with a low cannon body, brief mechanical/electrical crack and a 7 ms separation between barrels. Three 0.212-second variations replace the old 1.248-second firing sample. Only one audio voice is posted per volley, including spread fire. Normal and rapid shots keep their existing gameplay timing; short tails and moderate stereo placement reduce overlapping noise. Synthesized effects distinguish enemy fire, impacts, destruction, pickups and victory. Muzzle flashes, hit flashes, expanding explosion rings/sparks, shield rings and restrained camera shake improve combat feedback. The HUD remains stable during shake. Pause and shell interception lower the music. All audio is converted to 48 kHz stereo during staging; music streams through bounded rings instead of loading whole tracks as decoded PCM. The audio output is stopped before its buffers are freed.

Combat updates at a fixed 120 Hz with interpolated positions and banking. The Viper banks up to 24 degrees while moving vertically and eases back to level when released. Its twin cannons now fire from the rotated nose, along that banking angle. Rounds keep their launch heading after the player turns; their speed stays constant. Temporary rapid-fire and spread-fire upgrades alter the volley pattern or cooldown. Directional tracers make the angled paths visible, and boost exhaust follows the tail. Scouts cross quickly with weaving paths; fighters assemble and then dive toward a locked player position; bombers hold formation and launch slower, heavy rounds; escorts orbit capital ships and shield them until destroyed. Enemies use line, wedge and pincer formations. Target markers warn for approximately 0.55 seconds before attacks: aim locks at the warning, allowing the player to dodge. Enemies do not begin attacks close to or behind the player. Capital ships have three hull-based phases, escalating from spread volleys to faster spread and heavy aimed shots. Their HUD shows phase, health and escort shielding. Player rounds and enemy rounds use separate collision targets. Swept relative-motion collision prevents fast shots passing through ships. Hull damage has a 1.2-second grace period. Ship, projectile and effect pools are bounded; textures and clips are loaded once.

## Supplies and scores

Fly through diamond-shaped supplies to collect them:

| Mark | Effect |
| --- | --- |
| H | Repair 25 hull, capped at 100 |
| S | Add 40 shield, capped at 60; expires after 15 seconds |
| R | Rapid twin cannons for 14 seconds |
| W | Three twin-shot directions for 14 seconds |

Weapon supplies replace the current temporary weapon and refresh its timer. Shields absorb damage before hull; damage protection still applies after a hit. Bombers and every fourth kill can drop supplies, and wave breaks provide a guaranteed supply. Low hull biases combat drops toward repairs. Supplies expire, so collecting them requires moving through the playfield.

The results screen reports score, best score, sector/wave reached, time, enemies destroyed/escaped, projectile accuracy and supplies collected. A kill scores 100 for a scout/fighter, 150 for an escort, 200 for a bomber and 1,500 for a capital ship. Supplies add 50 points. Enemies that escape still allow a wave to end; defeating each capital ship is required to advance.

Best scores load at startup and save on defeat or victory, using a temporary file followed by rename. The preferred file is `/data/battlestar-ps5/highscore.txt`; `/app0/battlestar-highscore.txt` is a fallback. Actual write access depends on the console loader/sandbox and must be verified on hardware. If both fail, the game continues with a session best and displays a notice. Only best scores persist; a new run starts from wave 1.

## Weapon sound preview

Listen to `audio-preview/viper-cannons-preview.wav` in the source tree: an isolated volley, normal held fire, then rapid fire. `audio-preview/viper-cannon-single.wav` contains a single unscaled volley. The repeat preview uses the actual 0.42 mixer gain and gameplay firing intervals. These are original synthesized effects, not an extracted or exact replica of a Battlestar Galactica recording. Generated PCM, WAV previews and sound-design metadata are recreated at build time by `synthesize_viper.py`.

## Build and checks

Run `Build-PS5.ps1` from PowerShell. Requires this machine's previously prepared Ubuntu WSL PS5 toolchain and FFmpeg (`sudo apt-get install ffmpeg`). The script stages the port in `~/work/battlestar-ps5-app`, runs movement, combat, arcade and score-persistence tests, compiles the native title and validates the SELF container and folder-title metadata, required launcher icon, runtime and gameplay assets. Original PSP source/build files remain unchanged. Reference assets are snapshotted unchanged in `ps5/assets`; build-provenance.json in the package records their SHA256 hashes and reference revision.

`test_flight.cpp` checks movement timing, diagonal speed, boost, pause and bounds. `test_combat.cpp` checks 30/144 Hz frame pacing equivalence (including banked fire), muzzle placement, aim direction, constant round speed, persistent trajectories, atomic twin volleys at pool capacity, swept/team collisions, damage grace, pause, boss scheduling and a three-minute simulated endurance run. `test_arcade.cpp` verifies authored role counts, wave transitions, completion through the final boss, enemy behavior, escort shielding, boss phases and locked targeting, supplies, temporary weapons, pool capacity reserved for player fire, collision ordering and high-score reload/corruption/fallback behavior. Combat and arcade tests also pass with AddressSanitizer and UndefinedBehaviorSanitizer. `test_weapon_audio.py` checks deterministic variants, finite PCM, peak/headroom, short duration/tails, click-free boundaries and sustained normal/rapid-fire overlap. `test_package.py` covers the missing-icon registration regression, malformed PNGs, wrong folder identity and renamed raw ELFs. Every build regenerates package validation hashes and refreshes packaged documentation; provenance records the actual port source hashes.

The user confirmed the previous movement/audio build working on PS5 on 2026-10-23. This cannon-sound update is an offline rebuild and still needs a console listening test, especially sustained rapid fire against music and combat effects. The previous arcade package is preserved as `../../build-support/releases/BattleStar-pre-laser-01.002.000.zip`. The earlier banked-cannon package is also preserved as `../../build-support/releases/BattleStar-banked-cannons-01.001.000.zip`. The last working package is archived in `../../build-support/releases/BattleStar-console-confirmed-2026-10-23.zip`. Shared build lessons and future-work instructions live in `../../PS5-BUILD-GUIDE.md` and `../../AGENTS.md`.

Optional console smoke test: place `smoke-test.txt` containing a frame count beside eboot.bin. The app starts in combat, writes smoke-test.ppm and exits after that many frames. Use at least 600 frames to exercise enemy spawning. Remove the file for normal play. Capturing the screenshot requires a writable title folder.

See `REFERENCE-COMPARISON.md` for the reference review. Package licences/notices cover the platform and renderer; original artwork/audio retain their upstream rights and attributions.
