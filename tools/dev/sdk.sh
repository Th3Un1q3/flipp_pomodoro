#!/usr/bin/env bash
set -euo pipefail

# Single source of truth for ufbt SDK setup. Build, dev-container startup, just
# recipes and agent docs all call this script so the per-channel index rules
# cannot drift apart.

channel="${1:-standard}"
force=0

if [[ $# -gt 1 ]]; then
    if [[ "$2" == "--force" ]]; then
        force=1
    else
        echo "usage: $0 [standard|unleashed] [--force]" >&2
        exit 2
    fi
fi

case "$channel" in
    standard|unleashed) ;;
    *)
        echo "usage: $0 [standard|unleashed] [--force]" >&2
        exit 2
        ;;
esac

export UFBT_HOME="$HOME/.ufbt/$channel"

if [[ $force -eq 0 && -d "$UFBT_HOME/current" ]]; then
    exit 0
fi

if [[ "$channel" == "unleashed" ]]; then
    ufbt update \
        --index-url=https://up.unleashedflip.com/directory.json \
        --channel=release
else
    ufbt update --channel=release
fi
