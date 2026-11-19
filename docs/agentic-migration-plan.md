# Retire the legacy Flipper toolchain

This plan runs inside the dev container that `docs/agentic-environment.md` describes. Each task is one commit. Execute them in order; a task is done when every acceptance line has evidence.

## Current state

| Piece | Where | Problem |
|---|---|---|
| Firmware submodules | `.gitmodules`, `.standard-firmware`, `.unleashed-firmware` | Two firmware trees, several GB, cloned by every build |
| Legacy build driver | `tools/build.sh` | Runs `git submodule update --init --recursive`, then `fbt` inside a submodule |
| Editor config | `.vscode/c_cpp_properties.json`, `.vscode/settings.json`, `.vscode/tasks.json` | macOS paths, includePath into a submodule, an unused `chatgpt-helper` setting, a task that flashes hardware |
| Release CI | `.github/workflows/release.yml` | Invokes `tools/build.sh`, so it depends on the submodules |
| Dependency bot | `.github/dependabot.yml` | `gitsubmodule` ecosystem tracks the two firmware trees |
| Host build docs | `README.md` "Developer Environment", `docs/INSTALATION.md` | Manual virtualenv instructions, a duplicated and misspelled install page |

The container build path already exists: `tools/dev/build.sh` drives ufbt and produces `dist/flipp_pomodoro_<channel>_<api>.fap`.

## Non-goals

- Building or flashing firmware. ufbt builds applications only; hardware steps stay on a host.
- Reformatting C sources, adding unit tests, or changing app behaviour.
- Publishing anything. Open a pull request; do not push to `master`.

## Preconditions

```bash
git status --short          # must be empty
just verify                 # must exit 0
git switch -c chore/retire-legacy-toolchain
```

## Task 0 — Capture the baseline before anything is deleted

Record what the legacy path would have produced, so "it still builds" is measurable.

```bash
mkdir -p docs
git ls-tree HEAD .standard-firmware .unleashed-firmware
# API version the legacy driver read from each firmware tree:
git -C .standard-firmware show HEAD:firmware/targets/f7/api_symbols.csv 2>/dev/null | awk -F',' 'NR==2{print $3}'
git -C .standard-firmware show HEAD:targets/f7/api_symbols.csv 2>/dev/null | awk -F',' 'NR==2{print $3}'
```

Write `docs/environment-baseline.md` with one row per channel: submodule commit, API version, released `.fap` name, and `sha256sum` of the latest release artifact if one exists. Note that the numbers come from the submodule commit, not from a build, when the submodules are not checked out.

Acceptance:

- `docs/environment-baseline.md` lists both channels with an API version each.
- The file is committed.

## Task 1 — Standard-channel parity

```bash
just build standard
```

Acceptance:

- `dist/flipp_pomodoro_standard_<api>.fap` exists and is larger than 1 KB.
- `<api>` is equal to or newer than the standard row in `docs/environment-baseline.md`; record the delta. A lower API is a stop condition.

## Task 2 — Unleashed-channel parity

```bash
just sdk-unleashed
just build unleashed
```

Acceptance:

- `dist/flipp_pomodoro_unleashed_<api>.fap` exists and is larger than 1 KB.
- `<api>` is equal to or newer than the unleashed row in `docs/environment-baseline.md`; record the delta. A lower API is a stop condition.
- `UFBT_HOME=~/.ufbt/unleashed ufbt update --index-url=https://up.unleashedflip.com/directory.json --channel=release` leaves the artifact unchanged on a rebuild.

## Task 3 — Guard retirement with CI

Edit `.github/workflows/release.yml`: replace the two `bash tools/build.sh ...` calls with `bash tools/dev/build.sh standard` and `bash tools/dev/build.sh unleashed`, add `actions/setup-python`, install the same ufbt version the Dockerfile pins, and cache `~/.ufbt`. Leave the release triggers and the artifact glob alone.

Add `.github/workflows/ci.yml` with one job that runs in the dev container and builds both channels:

```yaml
- uses: devcontainers/ci@v0.3
  with:
    runCmd: bash .devcontainer/verify.sh && bash tools/dev/build.sh unleashed
```

Trigger it on pull requests touching `.devcontainer/**`, `flipp_pomodoro/**`, `tools/dev/**`, `justfile`, or the workflow file itself.

Acceptance:

- `git diff .github/workflows/release.yml` shows only build-step changes.
- The `ci.yml` job passes on the pull request.
- A deliberately broken C statement in `flipp_pomodoro/` makes the job fail; revert the statement and the job passes again.

## Task 4 — Retire the firmware submodules

```bash
git submodule deinit -f -- .standard-firmware .unleashed-firmware
git rm -f --cached .standard-firmware .unleashed-firmware
rm -rf .standard-firmware .unleashed-firmware .git/modules/.standard-firmware .git/modules/.unleashed-firmware
git rm -f .gitmodules
```

Then remove the two submodule lines from `.gitignore` and the `gitsubmodule` update from `.github/dependabot.yml`.

Acceptance:

- `git ls-files -s | awk '$1 == "160000"'` prints nothing.
- `test ! -e .gitmodules` succeeds.
- `just verify` exits 0 with no submodule present.
- `just build-all` still produces both artifacts with the Task 1 and Task 2 APIs.

## Task 5 — Retire `tools/build.sh`

```bash
git rm tools/build.sh
grep -rn "build.sh" --exclude-dir=.git --exclude-dir=node_modules .
```

Rewrite every remaining reference to `tools/dev/build.sh`; the hardware `-i` flow becomes `ufbt launch` on a host with the device attached.

Acceptance:

- `grep -rn "tools/build.sh" --exclude-dir=.git .` prints nothing.
- `tools/` contains only `files-list.sh` and `dev/`.
- `git ls-files -s | awk '$1 == "160000"'` still prints nothing.

## Task 6 — Replace the editor configuration

Replace `.vscode/c_cpp_properties.json` with a container config that resolves the SDK through the environment instead of a submodule:

```json
{
  "configurations": [
    {
      "name": "Linux",
      "includePath": ["${workspaceFolder}/flipp_pomodoro/**", "${env:UFBT_HOME}/current/**"],
      "cStandard": "c17",
      "cppStandard": "c++17",
      "intelliSenseMode": "${default}",
      "compilerPath": ""
    }
  ],
  "version": 4
}
```

Replace `.vscode/settings.json` with the three keys the project needs (`files.associations` for `*.fam`, `C_Cpp.default.cStandard`, `editor.formatOnSave: false`) and delete the `chatgpt-helper.model` setting and the hundred-entry association list. Rewrite `.vscode/tasks.json` so the build task runs `just build unleashed` and no task flashes hardware.

Acceptance:

- `grep -rn "/Applications\|chatgpt-helper" .vscode` prints nothing.
- `grep -rn "firmware" .vscode` prints nothing.
- Opening `flipp_pomodoro/flipp_pomodoro_app_i.h` in the container resolves `furi.h` without an error squiggle. Record the observation in the commit message.

## Task 7 — Lint the scripts that now carry the build

Add `shellcheck` to `.devcontainer/Dockerfile`, then add a recipe:

```just
# Check the shell scripts that drive the build.
lint:
    shellcheck tools/dev/*.sh .devcontainer/*.sh
```

Acceptance:

- `just lint` exits 0, or every suppression names its rule code and reason inline.
- `docker build` for the devcontainer still succeeds after the Dockerfile edit.

## Task 8 — Documentation

- Rewrite the README "Developer Environment" section around the dev container; keep a short host fallback (`pipx install ufbt`, then `cd flipp_pomodoro && ufbt`).
- Delete `docs/INSTALATION.md` and keep the install text in the README, or fix its spelling and remove the duplication.
- Update `docs/agentic-environment.md` if this retirement changed the commands, volumes, or SDK reset procedure.
- Update `AGENTS.md`: drop the sentence that calls `tools/build.sh` scheduled for removal.

Acceptance:

- `grep -rn "\.standard-firmware\|\.unleashed-firmware\|tools/build.sh" --include="*.md" .` prints nothing.
- Both links in `README.md` to the new docs resolve.

## Task 9 — Clean-room proof

```bash
# Stop the container first; the volume is in use while it runs.
docker volume rm flipp-ufbt
# Rebuild the container from scratch, then inside it:
just verify
just build-all
```

Acceptance:

- A rebuild with an empty SDK volume reaches a passing `just verify` with no manual command beyond the rebuild.
- Both artifacts exist; neither API is lower than `docs/environment-baseline.md`.
- `git status --short` is empty after the build (artifacts stay ignored).

## Retire map

| Removed | Replaced by |
|---|---|
| `.standard-firmware` submodule | `~/.ufbt/standard` SDK, official release channel |
| `.unleashed-firmware` submodule | `~/.ufbt/unleashed` SDK, `up.unleashedflip.com` index |
| `tools/build.sh` | `tools/dev/build.sh`, `just build`, `just build-all` |
| `.vscode/c_cpp_properties.json` macOS config | env-based includePath over `$UFBT_HOME/current` |
| `.vscode/settings.json` association list and `chatgpt-helper.model` | three project settings |
| `.vscode/tasks.json` flashing task | `just build unleashed` |
| `release.yml` submodule build | `tools/dev/build.sh` |
| Dependabot `gitsubmodule` entry | pinned `ufbt` version in the Dockerfile |
| `docs/INSTALATION.md` | README install section |
| README virtualenv workflow | dev container workflow |

## Stop conditions

- An artifact API is lower than `docs/environment-baseline.md` after two rebuild attempts.
- The Unleashed index is unreachable, or its SDK API diverges from the submodule baseline.
- A build fails with `APPCHK` for a symbol the manifest already declares.
- A task would delete or rewrite something outside the retire map.
- Any step seems to require changing application behaviour.

On a stop condition: revert the current task's commit, record the observed output in the pull request, and stop.

## Definition of done

- [ ] No gitlink entries and no `.gitmodules`.
- [ ] `tools/build.sh` gone, no reference anywhere.
- [ ] `release.yml` builds through `tools/dev/build.sh`; `ci.yml` proves the container gate on pull requests.
- [ ] `.vscode` contains no macOS path, no submodule path, and no hardware task.
- [ ] `just lint` and `just verify` exit 0 on a clean rebuild.
- [ ] `just build-all` reproduces both baseline APIs.
- [ ] Documentation describes the container path only, with the host fallback confined to a short section.
