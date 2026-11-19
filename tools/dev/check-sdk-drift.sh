#!/usr/bin/env bash
set -euo pipefail

# Compare the deployed SDK (firmware version and API) with the tracked JSON
# baseline. On a change, rewrite only that file so a pull request carries the
# change through CI and, once merged, the release workflow. This replaces the
# old submodule bump that Dependabot used to open for every firmware release.

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
baseline="$repo_root/tools/dev/sdk-baseline.json"
marker="$repo_root/dist/sdk-drift"

api_re='^[0-9]+([.][0-9]+)?$'
version_re='^[A-Za-z0-9._-]+$'

fail() {
    echo "$*" >&2
    exit 1
}

check_only=0
if [[ "${1:-}" == "--check-only" ]]; then
    check_only=1
    shift
fi
[[ $# -eq 0 ]] || fail "usage: $0 [--check-only]"

[[ -r "$baseline" ]] || fail "cannot read baseline: $baseline"
[[ "$(jq -r '.schema // empty' "$baseline")" == "1" ]] || fail "unsupported baseline schema in $baseline"

mkdir -p "$(dirname "$marker")"
: > "$marker"

for channel in standard unleashed; do
    ufbt_home="$HOME/.ufbt/$channel"
    state="$ufbt_home/current/ufbt_state.json"
    symbols="$ufbt_home/current/sdk_headers/f7_sdk/targets/f7/api_symbols.csv"
    for file in "$state" "$symbols"; do
        [[ -r "$file" ]] || fail "cannot read $file; refresh the $channel SDK first"
    done

    version="$(jq -r '.version // empty' "$state")"
    api="$(awk -F',' '$1 == "Version" { print $3; exit }' "$symbols")"
    [[ "$version" =~ $version_re ]] || fail "unexpected firmware version for $channel: '$version'"
    [[ "$api" =~ $api_re ]] || fail "unexpected API for $channel: '$api'"

    # The build script names the artifact after the firmware version, so the
    # file name is a second check that the build used the refreshed SDK.
    fap="$(ls -1t "$repo_root"/dist/"flipp_pomodoro_${channel}_"*.fap 2>/dev/null | head -n1)"
    [[ -n "$fap" ]] || fail "missing $channel artifact; cannot check SDK drift"
    built_version="$(basename "$fap" | sed -E "s/^flipp_pomodoro_${channel}_(.*)\.fap$/\1/")"
    [[ "$built_version" == "$version" ]] || fail "$channel artifact is named '$built_version' but SDK is '$version'"
    size="$(stat -c %s "$fap")"

    jq -e --arg c "$channel" '.channels[$c] | type == "object"' "$baseline" >/dev/null ||
        fail "no baseline channel object for $channel in $baseline"
    read -r baseline_api current_version current_api < <(
        jq -r --arg c "$channel" '.channels[$c] | [.baseline_api, .firmware_version, .api] | @tsv' "$baseline"
    )
    [[ -n "$baseline_api" && -n "$current_version" && -n "$current_api" ]] ||
        fail "incomplete baseline for $channel in $baseline"
    [[ "$baseline_api" =~ $api_re ]] || fail "unexpected baseline API for $channel: '$baseline_api'"
    [[ "$current_api" =~ $api_re ]] || fail "unexpected recorded API for $channel: '$current_api'"
    [[ "$current_version" =~ $version_re ]] || fail "unexpected recorded version for $channel: '$current_version'"

    if [[ "$version" == "$current_version" && "$api" == "$current_api" ]]; then
        continue
    fi

    if [[ "$check_only" -eq 1 ]]; then
        fail "$channel drift: $current_version/$current_api -> $version/$api; update tools/dev/sdk-baseline.json first"
    fi

    delta="$(awk -v a="$api" -v b="$baseline_api" 'BEGIN { printf "%.2f", a - b }')"
    tmp="$(mktemp "${baseline}.XXXXXX")"
    trap 'rm -f "$tmp"' EXIT
    jq --indent 4 \
        --arg c "$channel" --arg ver "$version" --arg api "$api" \
        '.channels[$c].firmware_version = $ver | .channels[$c].api = $api' \
        "$baseline" > "$tmp"
    mv "$tmp" "$baseline"
    trap - EXIT

    {
        echo "$channel: $current_version/$current_api -> $version/$api"
        echo "artifact: $fap ($size bytes)"
    } >> "$marker"
done
