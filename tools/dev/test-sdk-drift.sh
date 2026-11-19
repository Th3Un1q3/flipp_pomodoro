#!/usr/bin/env bash
set -euo pipefail

# Deterministic checks for tools/dev/check-sdk-drift.sh. Synthetic SDK state is
# used so this runs without downloading a firmware SDK or touching the real
# baseline.

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
checker="$repo_root/tools/dev/check-sdk-drift.sh"
baseline_source="$repo_root/tools/dev/sdk-baseline.json"

fail() {
    echo "$*" >&2
    exit 1
}

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
home="$tmp/home"
dist="$tmp/dist"
baseline="$tmp/sdk-baseline.json"
mkdir -p "$home" "$dist"
cp "$baseline_source" "$baseline"

make_sdk() {
    local channel="$1" version="$2" api="$3"
    local sdk="$home/.ufbt/$channel/current"
    mkdir -p "$sdk/sdk_headers/f7_sdk/targets/f7"
    printf '{"version":"%s"}\n' "$version" > "$sdk/ufbt_state.json"
    printf 'Version,,%s\n' "$api" > "$sdk/sdk_headers/f7_sdk/targets/f7/api_symbols.csv"
    : > "$dist/flipp_pomodoro_${channel}_${version}.fap"
}

run_check() {
    HOME="$home" \
    FLIPP_POMODORO_BASELINE="$baseline" \
    FLIPP_POMODORO_DIST="$dist" \
        bash "$checker" "$@"
}

hash_file() {
    sha256sum "$1" | awk '{ print $1 }'
}

make_sdk standard 1.5.1 88.2
make_sdk unleashed unlshd-095 88.26

before="$(hash_file "$baseline")"
output="$(run_check)"
[[ "$(hash_file "$baseline")" == "$before" ]] || fail "no-drift run changed the baseline"
[[ ! -s "$dist/sdk-drift" ]] || fail "no-drift run wrote a drift marker"
grep -q 'standard: no drift (1.5.1 / API 88.2)' <<< "$output" || fail "missing standard no-drift report"
grep -q 'unleashed: no drift (unlshd-095 / API 88.26)' <<< "$output" || fail "missing unleashed no-drift report"
echo 'PASS no-drift: baseline unchanged and both channels reported'

jq '.channels.standard.firmware_version = "0.0.0" | .channels.standard.api = "1.0"' \
    "$baseline" > "$tmp/drift.json"
mv "$tmp/drift.json" "$baseline"

output="$(run_check)"
jq -e '(.channels.standard.firmware_version == "1.5.1") and (.channels.standard.api == "88.2")' \
    "$baseline" > /dev/null || fail "drift run did not restore the recorded standard pair"
grep -q 'standard: drift 0.0.0/1.0 -> 1.5.1/88.2' <<< "$output" || fail "missing standard drift report"
grep -q 'standard: 0.0.0/1.0 -> 1.5.1/88.2' "$dist/sdk-drift" || fail "drift marker missing expected line"
echo 'PASS drift: baseline rewritten and reported'

jq '.channels.standard.firmware_version = "0.0.0"' "$baseline" > "$tmp/stale.json"
mv "$tmp/stale.json" "$baseline"
stale="$(hash_file "$baseline")"
if run_check --check-only > "$tmp/check-only.log" 2>&1; then
    fail "check-only accepted a stale baseline"
fi
[[ "$(hash_file "$baseline")" == "$stale" ]] || fail "check-only modified the baseline"
grep -q 'update tools/dev/sdk-baseline.json first' "$tmp/check-only.log" || fail "check-only error missing remediation"
echo 'PASS check-only: stale baseline fails without writing'

printf '{"schema": 2}\n' > "$tmp/bad-schema.json"
if HOME="$home" \
    FLIPP_POMODORO_BASELINE="$tmp/bad-schema.json" \
    FLIPP_POMODORO_DIST="$dist" \
    bash "$checker" > "$tmp/schema.log" 2>&1; then
    fail "unsupported schema was accepted"
fi
grep -q 'unsupported baseline schema' "$tmp/schema.log" || fail "schema error missing"
echo 'PASS schema: unsupported version is rejected'
