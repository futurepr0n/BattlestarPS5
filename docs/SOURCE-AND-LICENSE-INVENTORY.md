# Source, dependency and asset inventory

Catalog license: **Unlicensed**, requested by the owner; GitHub currently reports no repository license. This is metadata, not a new license grant. Do not remove existing notices or infer asset rights from the repository being public.

## Published source scope

This repository publishes the PS5 game code, tests, build scripts, procedural sound generator and launcher metadata/icon. Reference music/art, generated outputs and external platform/dependency trees are not committed. This is not yet a complete corresponding-source distribution. See BUILDING.md and ../ps5/assets/README.md for required external inputs and remaining work.

Original PSPGameCPP revision: 6773239be763abc8a54336357933e9e1d2afde3c.
BattleStar-OSLib reference: d627d007e4d521b9b89cf38a13051c4bf9851ba9.
Platform: b794f982df2060eac1b5b10017a7dee3aecb0b90.
PS5_Vulkan: 573d65bbebe2839b8d861fbe219ce1151fd84f7c (local patches also matter).
HUI: 4451c261ac8631c698291ee765a37456f25bc22c.

## Dependencies

- HUI renderer and retained platform portions: GPL-3.0-or-later according to source/notices. Resolve applicable combined-work licensing and provide the exact corresponding source, modifications and build scripts when distributing; merely including GPL text or marking the repository Unlicensed does not discharge these obligations.
- Platform template, Vulkan, PayloadSDK, Mesa/RADV and fonts retain their own notices. Current package includes licenses/ and assets/LICENSES/; those texts must stay with the release.
- Prepared build tree is `~/work/battlestar-ps5-app`; platform baseline is `~/work/ps5-rpcs3-app`; Vulkan/runtime tree is `~/work/PS5_Vulkan`. Capture exact used sources and local patches before claiming reproducibility on a different machine. The build wrapper alone does not install these dependencies.

## Game assets

- `menu.ogg`: converted from theme_battlestar_galactica.mp3; redistribution grant not documented.
- `combat.ogg`: converted from mrdeath_-_12th_moon_rising.xm (MrDeath); redistribution grant not documented.
- `vipersm.png`, `bsg_title.png`: original PSPGameCPP artwork; blanket redistribution grant not documented.
- Raider/capital-ship/projectile/starfield artwork: BattleStar-OSLib reference; blanket redistribution grant not documented.
- `fire.f32`, `fire_alt1.f32`, `fire_alt2.f32`: original procedural sound design from synthesize_viper.py; no sampled television/game weapon audio. Other generated combat effects are synthesized in code.
- `sce_sys/icon0.png`: generated from local vector primitives by Build-Icon.ps1.

Obtain permission for uncertain material or replace it with original/appropriately licensed alternatives. Replacing assets requires rebuilding, revalidating and regenerating this submission and its ZIP digest. The current owner choice of Unlicensed does not confirm permission for upstream music or art.

## Exact packaged game-asset hashes

- `assets/game/battlestar.png`: `5f67d9bba0eb64570c4f6377b7e80764e54036cb25a7ce6da16d7793b6a38c6a`
- `assets/game/bsg_title.png`: `a2f7081bdda213516cfe0c734657c508184061b802115b96db074cde4fe83e44`
- `assets/game/chain.png`: `ed11f009e563b225a40a4ad3f67cbb84814a0367a47fcf4bf2726dbfcc8cfb3d`
- `assets/game/combat.ogg`: `ed0475feaa19abab762eea3ec9b29181e770b4e4deefbc21bcbd8d1283322314`
- `assets/game/enemychain.PNG`: `a05eec80b3bf432b071dc90d06eef7719583d9eab62a9116cace92f58d0635ac`
- `assets/game/fire.f32`: `f25db92a3bad4b16d0bc8b5d41f480d73a47734f8cac975fc26215f9a255babc`
- `assets/game/fire_alt1.f32`: `369aa740d823e2dd7f3b215c6d8a8995e7860b3bcd79123a5fb48c4b458a0b88`
- `assets/game/fire_alt2.f32`: `94d1118d3ebee20819f68e4f95cdfcf204a139e9b452d0fc85857f605fd9a4e9`
- `assets/game/menu.ogg`: `d5ab415e6f1290fdf99121becc03659ee42af6c5df10f5be8ff55233b82ce59a`
- `assets/game/raidersm.png`: `c30d7127f683fb7e0181d167fa17d6a36f084775a22c51f9548df4793387ab81`
- `assets/game/starfield_one.png`: `4bb2fe004b066a2a300ae75a2d8cd6587ef7f74552d9e411c7da307ff45cdaa6`
- `assets/game/vipersm.png`: `cc70128084789336cc6bc8a5cf7e5419a964113a663f74be79042c50dddd866e`
