FROM mcr.microsoft.com/devcontainers/base:noble

ARG UFBT_VERSION=0.2.6
ARG OPENCODE_VERSION=

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        curl \
        file \
        jq \
        libusb-1.0-0 \
        pipx \
        python3-pip \
        python3-venv \
        shellcheck \
        unzip \
        xz-utils \
    && rm -rf /var/lib/apt/lists/*

USER vscode

# ufbt downloads its own Python and arm-none-eabi toolchain on first use, so the
# distro Python only has to satisfy the bootstrap (3.8 or newer).
# The OpenCode installer writes the binary to ~/.opencode/bin and leaves PATH
# alone; devcontainer.json adds that directory. An empty OPENCODE_VERSION
# installs the current release.
RUN pipx install "ufbt==${UFBT_VERSION}" \
    && if [ -n "${OPENCODE_VERSION}" ]; then \
        curl -fsSL https://opencode.ai/v2/install | VERSION="${OPENCODE_VERSION}" bash; \
    else \
        curl -fsSL https://opencode.ai/v2/install | bash; \
    fi
