#!/usr/bin/env bash
set -euo pipefail

# Compare the APIs produced by a fresh SDK build with the recorded parity table.
# On drift, update that table so a pull request can carry the change through CI
# and, once merged, the release workflow. This replaces the old submodule bump
# that Dependabot used to open for every firmware release.

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
baseline="$repo_root/docs/environment-baseline.md"
marker="$repo_root/dist/sdk-drift"

mkdir -p "$(dirname "$marker")"
: > "$marker"

for channel in standard unleashed; do
    # The freshly built artifact is the most recently written one; using ls -t
    # keeps this correct when an older API artifact is still in dist/.
    fap="$(ls -1t "$repo_root"/dist/"flipp_pomodoro_${channel}_"*.fap 2>/dev/null | head -n1)"
    if [[ -z "$fap" ]]; then
        echo "missing $channel artifact; cannot check API drift" >&2
        exit 1
    fi

    api="$(basename "$fap" | sed -E "s/^flipp_pomodoro_${channel}_(.*)\.fap$/\1/")"
    size="$(stat -c %s "$fap")"

    current="$(awk -F'|' -v c="$channel" '
        /^## Container parity/ { parity = 1; next }
        parity && $2 ~ c { gsub(/ /, "", $4); print $4; exit }
    ' "$baseline")"

    if [[ -z "$current" ]]; then
        echo "no parity row for $channel in $baseline" >&2
        exit 1
    fi

    if [[ "$api" == "$current" ]]; then
        continue
    fi

    tmp="$(mktemp)"
    awk -F'|' -v OFS='|' -v c="$channel" -v api="$api" -v size="$size" '
        /^## Container parity/ { parity = 1; print; next }
        parity && $2 ~ c {
            base = $3
            gsub(/ /, "", base)
            delta = sprintf("%.2f", api - base)
            $4 = " " api " "
            $5 = " " delta " "
            $6 = " `dist/flipp_pomodoro_" c "_" api ".fap` (" size " bytes) "
        }
        { print }
    ' "$baseline" > "$tmp"
    mv "$tmp" "$baseline"
    echo "$channel: $current -> $api" >> "$marker"
done
