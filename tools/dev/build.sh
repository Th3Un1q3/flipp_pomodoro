#!/usr/bin/env bash
set -euo pipefail

channel="${1:-standard}"
case "$channel" in
    standard|unleashed) ;;
    *)
        echo "usage: $0 [standard|unleashed]" >&2
        exit 2
        ;;
esac

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export UFBT_HOME="$HOME/.ufbt/$channel"

bash "$repo_root/tools/dev/sdk.sh" "$channel"

log="$(mktemp)"
trap 'rm -f "$log"' EXIT

(cd "$repo_root/flipp_pomodoro" && ufbt) 2>&1 | tee "$log"

# A missing API line still fails the build: the release notes report the API
# the FAP was linked against, even though the file name uses the firmware
# version from the deployed SDK.
api="$(sed -e 's/\x1b\[[0-9;]*m//g' "$log" | sed -n 's/.*API: *\([0-9][0-9.]*\).*/\1/p' | tail -n1)"
if [[ -z "$api" ]]; then
    echo "could not read the API version from ufbt output" >&2
    exit 1
fi

state="$UFBT_HOME/current/ufbt_state.json"
if [[ ! -r "$state" ]]; then
    echo "SDK state file missing: $state" >&2
    exit 1
fi
firmware_version="$(jq -r '.version // empty' "$state")"
if [[ -z "$firmware_version" || ! "$firmware_version" =~ ^[A-Za-z0-9._-]+$ ]]; then
    echo "unexpected firmware version in $state: $firmware_version" >&2
    exit 1
fi

mkdir -p "$repo_root/dist"
out="$repo_root/dist/flipp_pomodoro_${channel}_${firmware_version}.fap"
cp "$repo_root/flipp_pomodoro/dist/flipp_pomodoro.fap" "$out"
echo "built $out (firmware $firmware_version, API $api)"
