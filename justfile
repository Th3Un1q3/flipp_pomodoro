set shell := ["bash", "-euo", "pipefail", "-c"]

# Build the .fap for one firmware channel; the artifact lands in dist/.
build $channel="standard":
    bash tools/dev/build.sh {{channel}}

# Build both firmware channels in sequence.
build-all:
    bash tools/dev/build.sh standard
    bash tools/dev/build.sh unleashed

# Refresh the Unleashed SDK from its third-party index.
sdk-unleashed:
    bash tools/dev/sdk.sh unleashed --force

# Environment gate: tools present, SDK present, clean build of one channel.
verify:
    bash .devcontainer/verify.sh

# Check the shell scripts that drive the build and the dev container.
lint:
    shellcheck tools/dev/*.sh .devcontainer/*.sh

# Delete generated build output.
clean:
    rm -rf dist flipp_pomodoro/dist
