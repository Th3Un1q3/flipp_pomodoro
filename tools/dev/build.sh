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

# The release notes and the artifact name both key off the API version, so a
# missing API line is a build failure rather than an "unknown" artifact.
api="$(sed -e 's/\x1b\[[0-9;]*m//g' "$log" | sed -n 's/.*API: *\([0-9][0-9.]*\).*/\1/p' | tail -n1)"
if [[ -z "$api" ]]; then
    echo "could not read the API version from ufbt output" >&2
    exit 1
fi

mkdir -p "$repo_root/dist"
out="$repo_root/dist/flipp_pomodoro_${channel}_${api}.fap"
cp "$repo_root/flipp_pomodoro/dist/flipp_pomodoro.fap" "$out"
echo "built $out"
