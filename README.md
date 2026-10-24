# BattleStar for PS5

Native PS5 homebrew arcade shooter by futurepr0n, developed from the original PSPGameCPP and BattleStar-OSLib projects. Three sectors, twelve waves, banking twin cannons, enemy formations, capital-ship bosses, supplies and best scores.

See [PS5 gameplay and controls](ps5/README.md) and [build requirements](docs/BUILDING.md). This source repository currently has no binary release. The reference music/art are intentionally omitted pending redistribution permission; required inputs are documented in [ps5/assets/README.md](ps5/assets/README.md).

The build produces a folder title (PPSA99304), not a PKG or PSP EBOOT. For a separately built package, copy the complete PPSA99304 folder into the loader's homebrew directory. Preserve existing saved data and keep one installed copy of this title ID.

Build with `./Build-PS5.ps1` using the prepared Ubuntu WSL PayloadSDK/Vulkan environment described in the port sources. The current scripts depend on a prepared external platform/toolchain tree; they are not a clean-machine installer.

## License and assets

No blanket license is granted for this repository (catalog status: Unlicensed). Existing third-party licenses remain applicable, including GPL-3.0-or-later renderer/platform components. See packaged THIRD_PARTY_NOTICES.txt and licenses/. Original reference music and art retain their respective owners' rights. This statement does not grant redistribution permission for those assets or override dependency licenses.

Version 01.003.000 passed offline package checks. This exact cannon-sound update still needs a console listening/gameplay test; prior movement/audio builds were reported working on PS5. No specific firmware compatibility range or performance guarantee is claimed.
