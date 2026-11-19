#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

channel="${1:-standard}"

echo "== tool versions =="
git --version
python3 --version
opencode --version
just --version
printf 'ufbt: %s\n' "$(pipx list --short 2>/dev/null | awk '$1 == "ufbt" {print $2}')"

echo "== build: $channel =="
bash tools/dev/build.sh "$channel"

fap="$(ls -1t "$repo_root"/dist/flipp_pomodoro_"$channel"_*.fap | head -n1)"
if [[ ! -s "$fap" ]]; then
    echo "build produced no usable artifact for channel $channel" >&2
    exit 1
fi
printf 'PASS %s (%s bytes)\n' "$fap" "$(stat -c %s "$fap")"
