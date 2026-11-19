#!/usr/bin/env bash
set -euo pipefail

export PATH="$HOME/.local/bin:$HOME/.opencode/bin:$PATH"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Docker creates a new named volume root-owned, and both tools write into theirs.
for dir in "$HOME/.ufbt" "$HOME/.local/share/opencode"; do
    sudo mkdir -p "$dir"
    sudo chown -R "$(id -u):$(id -g)" "$dir"
done

mkdir -p "$HOME/.ufbt/standard" "$HOME/.ufbt/unleashed"

for tool in git python3 ufbt opencode; do
    if ! command -v "$tool" >/dev/null; then
        echo "missing tool: $tool" >&2
        exit 1
    fi
done

if [[ "${SKIP_SDK_UPDATE:-0}" != "1" ]]; then
    echo "Checking the official Flipper SDK (about 1 GB on a fresh volume)..."
    bash "$repo_root/tools/dev/sdk.sh" standard
fi

if [[ "${BOOTSTRAP_UNLEASHED:-0}" == "1" ]]; then
    echo "Checking the Unleashed SDK..."
    bash "$repo_root/tools/dev/sdk.sh" unleashed
fi

echo "ufbt:     $(pipx list --short 2>/dev/null | awk '$1 == "ufbt" {print $2}')"
echo "opencode: $(opencode --version 2>/dev/null | head -n1)"
echo "python:   $(python3 --version)"
