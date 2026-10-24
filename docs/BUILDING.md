# Build requirements and provenance

The initial source import is the current BattleStar PS5 game, build orchestration, procedural cannon synthesizer and tests. Original PSP platform code/binaries are not needed in this dedicated repository. The imported port code matches the local 01.003.000 build; adding documentation and committed metadata did not rebuild that binary.

## Current limitations

This checkout is not a standalone clean-machine build. It requires a prepared Ubuntu WSL toolchain/platform tree and the separately obtained reference media described in ps5/assets/README.md. No compiled libraries, runtime binaries, ROMs or firmware are committed. See SOURCE-AND-LICENSE-INVENTORY.md for unresolved distribution/source requirements.

The local wrapper uses the Ubuntu distribution and the existing /home/futurepr0n/work staging layout. Adapt the Windows copy-back path in Build-PS5.ps1 for another WSL user. Do not silently update dependency revisions while recreating the validated environment.

| Prepared WSL path | Role / recorded revision |
| --- | --- |
| ~/work/ps5-rpcs3-app | Native platform template and .deps, b794f982df2060eac1b5b10017a7dee3aecb0b90 |
| ~/work/PS5_Vulkan | Native linker/runtime/RADV route, 573d65bbebe2839b8d861fbe219ce1151fd84f7c plus local patches |
| Platform .deps/hui | HUI source, 4451c261ac8631c698291ee765a37456f25bc22c |
| ~/work/battlestar-ps5-app | Generated BattleStar staging/output, not source |

The prepared toolchain uses clang 18.1.3 and /usr/lib/llvm-22/bin/llvm-config 22.1.8, plus CMake, Ninja, Python 3, g++ and FFmpeg. The platform build supplies the target SDK/stubs, CRT, Mesa/RADV archive and matching libc.prx; a host compiler alone cannot build a PS5 title. Exact dependency modifications and a portable bootstrap remain to be published before claiming a complete reproducible source distribution.

From PowerShell, after restoring required inputs:

```powershell
./Build-PS5.ps1
```

Output: dist/PPSA99304/. The build runs simulation and audio tests, native linking/conversion and package checks. Require native-tool SELF inspection to report valid integrity. Keep sce_sys/icon0.png, sce_sys/param.json, runtime, assets and notices with eboot.bin.

Source-controlled ps5/sce_sys/param.json records the current title/version for catalog discovery. prepare_build.py currently generates matching metadata from hardcoded values; update both together until source metadata becomes authoritative. Increase contentVersion for future published releases and never replace bytes under an existing release tag.

## Tests without reference media

```powershell
python ps5/test_weapon_audio.py
python ps5/test_package.py
```

The three standalone C++17 test executables (test_flight.cpp, test_combat.cpp, test_arcade.cpp) can also be compiled/run on a host with g++; they do not require the PS5 SDK or reference media.

## Provenance

Original PSP project: https://github.com/futurepr0n/PSPGameCPP at 6773239be763abc8a54336357933e9e1d2afde3c.
Reference: https://github.com/futurepr0n/BattleStar-OSLib at d627d007e4d521b9b89cf38a13051c4bf9851ba9.
This dedicated repository is https://github.com/futurepr0n/BattlestarPS5.

The current binary has offline validation; prior movement/audio builds were reported working on PS5. This exact cannon update still needs console testing. Source publication does not publish a binary release or submit the catalog record.
