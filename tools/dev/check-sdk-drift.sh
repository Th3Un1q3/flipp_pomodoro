#!/usr/bin/env bash
set -euo pipefail

# Compare the deployed SDK (firmware version and API) with the recorded parity
# table. On a change, rewrite that row so a pull request carries the change
# through CI and, once merged, the release workflow. This replaces the old
# submodule bump that Dependabot used to open for every firmware release.

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
baseline="$repo_root/docs/environment-baseline.md"
marker="$repo_root/dist/sdk-drift"

mkdir -p "$(dirname "$marker")"
: > "$marker"

for channel in standard unleashed; do
    ufbt_home="$HOME/.ufbt/$channel"
    state="$ufbt_home/current/ufbt_state.json"
    symbols="$ufbt_home/current/sdk_headers/f7_sdk/targets/f7/api_symbols.csv"
    for file in "$state" "$symbols"; do
        if [[ ! -r "$file" ]]; then
            echo "cannot read $file; refresh the $channel SDK first" >&2
            exit 1
        fi
    done

    version="$(jq -r '.version // empty' "$state")"
    api="$(awk -F',' '$1 == "Version" { print $3; exit }' "$symbols")"
    if [[ -z "$version" || -z "$api" ]]; then
        echo "incomplete SDK metadata for $channel (version='$version', API='$api')" >&2
        exit 1
    fi

    # The build script names the artifact after the firmware version, so the
    # file name is a second check that the build used the refreshed SDK.
    fap="$(ls -1t "$repo_root"/dist/"flipp_pomodoro_${channel}_"*.fap 2>/dev/null | head -n1)"
    if [[ -z "$fap" ]]; then
        echo "missing $channel artifact; cannot check SDK drift" >&2
        exit 1
    fi
    built_version="$(basename "$fap" | sed -E "s/^flipp_pomodoro_${channel}_(.*)\.fap$/\1/")"
    if [[ "$built_version" != "$version" ]]; then
        echo "$channel artifact is named '$built_version' but SDK is '$version'" >&2
        exit 1
    fi
    size="$(stat -c %s "$fap")"

    baseline_api="$(awk -F'|' -v c="$channel" '
        /^## Container parity/ { exit }
        { name = $2; gsub(/ /, "", name) }
        name == c { value = $4; gsub(/ /, "", value); print value; exit }
    ' "$baseline")"
    if [[ -z "$baseline_api" ]]; then
        echo "no baseline API for $channel in $baseline" >&2
        exit 1
    fi

    current="$(awk -F'|' -v c="$channel" '
        /^## Container parity/ { parity = 1; next }
        parity {
            name = $2; gsub(/ /, "", name)
            if (name == c) {
                value = $3; revision = $4
                gsub(/ /, "", value); gsub(/ /, "", revision)
                print value "|" revision; exit
            }
        }
    ' "$baseline")"
    if [[ -z "$current" ]]; then
        echo "no parity row for $channel in $baseline" >&2
        exit 1
    fi
    IFS='|' read -r current_version current_api <<< "$current"

    if [[ "$version" == "$current_version" && "$api" == "$current_api" ]]; then
        continue
    fi

    delta="$(awk -v a="$api" -v b="$baseline_api" 'BEGIN { printf "%.2f", a - b }')"
    tmp="$(mktemp)"
    awk -F'|' -v OFS='|' -v c="$channel" -v ver="$version" -v api="$api" \
        -v delta="$delta" -v size="$size" '
        /^## Container parity/ { parity = 1; print; next }
        parity {
            name = $2; gsub(/ /, "", name)
            if (name == c) {
                $3 = " " ver " "
                $4 = " " api " "
                $5 = " " delta " "
                $6 = " `dist/flipp_pomodoro_" c "_" ver ".fap` (" size " bytes) "
            }
        }
        { print }
    ' "$baseline" > "$tmp"
    mv "$tmp" "$baseline"
    echo "$channel: $current_version/$current_api -> $version/$api" >> "$marker"
done
