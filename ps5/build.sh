#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
export PS5_CLANG=/usr/bin/clang LLVM_CONFIG=/usr/lib/llvm-22/bin/llvm-config
export PS5_VULKAN_DIR=$HOME/work/PS5_Vulkan
python3 "$repo/ps5/test_weapon_audio.py"
python3 "$repo/ps5/prepare_build.py" "$repo"
app=$HOME/work/battlestar-ps5-app
g++ -std=c++17 -O2 -Wall -Wextra -pedantic "$repo/ps5/test_flight.cpp" -o "$app/test-flight"
"$app/test-flight"
g++ -std=c++17 -O2 -Wall -Wextra -pedantic "$repo/ps5/test_combat.cpp" -o "$app/test-combat"
"$app/test-combat"
g++ -std=c++17 -O2 -Wall -Wextra -pedantic "$repo/ps5/test_arcade.cpp" -o "$app/test-arcade"
"$app/test-arcade"
bash "$app/ps5/tools/build.sh"
python3 "$repo/ps5/test_package.py"
python3 "$repo/ps5/validate_package.py" "$app/dist/PPSA99304" --write-report
echo "Built $app/dist/PPSA99304"
