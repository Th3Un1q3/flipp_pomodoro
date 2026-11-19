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

**Motivation that must survive the retirement:** the `gitsubmodule` watcher was not housekeeping. A daily Dependabot bump of either gitlink opened a PR; that PR ran CI, and merging it to `master` ran the release workflow against the newly published firmware API. The replacement must keep the loop *upstream API change → PR → build → release*. Removing the submodules without replacing that trigger silently loses functionality.

## Non-goals

- Building or flashing firmware. ufbt builds applications only; hardware steps stay on a host.
- Reformatting C sources, adding unit tests, or changing app behaviour.
- Publishing anything. Open a pull request; do not push to `master`.

## Preconditions

```bash
git status --short          # record the starting state
just verify                 # must exit 0
```

**Execution note:** the plan originally required a clean tree and a new `chore/retire-legacy-toolchain` branch. Neither held here: the tree already carried untracked setup work, and the maintainer directed that everything stay on `feat/agentic-setup`. No branch was created or checked out. Do not reintroduce the branch step when re-reading this plan.

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

Edit `.github/workflows/release.yml`: replace the two legacy build calls with one `devcontainers/ci@v0.3` step whose `runCmd` calls `bash tools/dev/build.sh standard && bash tools/dev/build.sh unleashed` inside the prebuilt image. Do **not** add `actions/setup-python`, `pipx install`, or `actions/cache` — that is a second toolchain and duplicates the Dockerfile. Leave the release triggers and the artifact glob alone.

Add `.github/workflows/ci.yml` with one job that runs in the dev container and builds both channels:

```yaml
- uses: devcontainers/ci@v0.3
  with:
    runCmd: bash .devcontainer/verify.sh && bash tools/dev/build.sh unleashed
```

Trigger it on pull requests touching `.devcontainer/**`, `flipp_pomodoro/**`, `tools/dev/**`, `docs/**`, `justfile`, or the workflow file itself. `docs/**` is required so the SDK-drift PR below is actually gated.

Acceptance:

- `git diff .github/workflows/release.yml` shows only build-step changes.
- The `ci.yml` job passes on the pull request.
- A deliberately broken C statement in `flipp_pomodoro/` makes the job fail; revert the statement and the job passes again.

**Execution notes / prevention:**
- `.devcontainer/verify.sh` first resolved `repo_root` with `../..`, which from `.devcontainer/` is `/workspaces`, so `just verify` failed with `tools/dev/build.sh: No such file or directory`. The script directory is one level below the repo root. Prevent recurrence by running `just verify` locally after any change to `.devcontainer/*.sh`; CI alone would not have caught a broken local gate.
- SDK setup had been duplicated in four places (`tools/dev/build.sh`, `.devcontainer/post-create.sh`, `justfile`, build skill). Only `tools/dev/sdk.sh` may contain `ufbt update`. Prevent drift with `grep -rn "ufbt update" --exclude-dir=.git .`; prose in docs is fine, executable duplicates are not.
- I initially added `actions/setup-python`, `pipx install` and `actions/cache` to `release.yml`, creating a second toolchain. The dev container image already owns those. Prevent recurrence with `grep -rn "setup-python\|actions/cache\|pipx install" .github/workflows/`, which must print nothing.

## Task 3a — Preserve upstream API change detection (added during execution)

The original plan missed this. The submodule bumps were the trigger for rebuilds; deleting them without a replacement means a new firmware API is only noticed when a human happens to push app code. Add `.github/workflows/sdk-drift.yml`:

- `schedule` (nightly) plus `workflow_dispatch`; no parallel toolchain — use `devcontainers/ci@v0.3`.
- `runCmd` refreshes both SDKs through `tools/dev/sdk.sh --force`, rebuilds both channels, then runs `tools/dev/check-sdk-drift.sh`.
- `check-sdk-drift.sh` compares the `API:` encoded in the fresh artifacts with the parity table in `docs/environment-baseline.md`; on drift it rewrites that row.
- `peter-evans/create-pull-request` opens a PR for the changed parity row; that PR runs `ci.yml`, and merging it runs `release.yml`.

Acceptance:

- A nightly run with no API change leaves the working tree clean.
- A simulated API change updates the parity row and opens a PR; no release is created until the PR is merged.
- The drift workflow uses only the dev container action and the same build/SDK scripts as local builds.

**Execution notes / prevention:**
- `ci.yml` had to gain `docs/**` in `pull_request.paths`. Without it, a drift PR that only edits the parity table would not run CI, so merging it would not exercise the build gate.
- Both the drift PR and the release step rely on `devcontainers/ci@v0.3` mounting the workspace back to the runner so `create-pull-request` sees the rewritten table and `ncipollo/release-action` sees `dist/**`. That behaviour cannot be exercised locally; it is documented here rather than claimed as verified.
- `check-sdk-drift.sh` compares against the parity table in `docs/environment-baseline.md`, not against the release assets. Those assets predate the current APIs, so they must not be used as the "current" value.

## Task 4 — Retire the firmware submodules

```bash
git submodule deinit -f -- .standard-firmware .unleashed-firmware
git rm -f --cached .standard-firmware .unleashed-firmware
rm -rf .standard-firmware .unleashed-firmware .git/modules/.standard-firmware .git/modules/.unleashed-firmware
git rm -f .gitmodules
```

Then remove the two submodule lines from `.gitignore` and the `gitsubmodule` update from `.github/dependabot.yml`.

**Execution notes:** the checked-out submodules were stale (`8622f1a`, `7ccce3e`, API 87.1) relative to the recorded gitlinks (`9e810dc` API 88.2, `9c63277` API 88.26). Task 0 must read the API from the gitlink commit, never from the submodule working tree's `HEAD`, or the baseline is understated and every later parity check is wrong. Before deleting the multi-GB trees, move them to a backup location so the retirement can be reverted if a build fails. The plan's `rm -rf` list also misses the stale `.git/modules/.official-firmware` directory left by an older submodule name; it is harmless but should be recorded rather than silently rediscovered.

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

**Execution note:** this document itself contains `tools/build.sh` in its historical sections, so the literal grep can never print nothing while the plan is tracked. Treat the criterion as "no live reference outside this plan"; the same applies to Task 8's markdown grep. Do not rewrite or delete the plan just to make a grep pass.

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

**Execution note:** `flipp_pomodoro/.vscode/compile_commands.json` is generated by ufbt's `CDB` step on every build, not by the editor config. It must be ignored via `.gitignore` (`flipp_pomodoro/.vscode/`), not deleted as a stray file. Also, `.devcontainer/devcontainer.json` carries its own `customizations.vscode.settings` block duplicating `files.associations` / `C_Cpp.default.cStandard` / `editor.formatOnSave`; keep one owner (the tracked `.vscode/settings.json`) to avoid a second configuration source.

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

**Execution note:** `just verify` cannot prove the Dockerfile change; a real `docker build` needs the host daemon, which is outside this container. Record that limitation instead of claiming the image was rebuilt. Shellcheck must cover the new `tools/dev/sdk.sh` and `tools/dev/check-sdk-drift.sh` too, not just the two scripts named in the original plan.

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

**Execution note:** `docker volume rm flipp-ufbt` is a host-side command; the dev container cannot remove the volume it is running from. Do not fake this step — leave it to the maintainer and report it as unverified locally.

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
| Dependabot `gitsubmodule` entry | nightly `sdk-drift.yml` + `tools/dev/check-sdk-drift.sh` PR (the pinned ufbt version only pin the tool, not the firmware API) |
| `docs/INSTALATION.md` | README install section |
| README virtualenv workflow | dev container workflow |

## Failure record and prevention measures

Observed during execution; each measure is a check to run before calling the migration done.

| Failure / gap observed | Measure that prevents it |
|---|---|
| Two files named `build.sh` (`tools/build.sh`, `tools/dev/build.sh`) | Task 5 deletes the legacy one; `find tools -name build.sh` must print only `tools/dev/build.sh` |
| SDK bootstrap duplicated in four places | single `tools/dev/sdk.sh`; grep for `ufbt update` outside it must be empty |
| `repo_root` off by one in `verify.sh` broke `just verify` | compute from `$(dirname "${BASH_SOURCE[0]}")/..` and run `just verify` locally after editing dev-container scripts |
| CI grew a parallel Python/pipx/cache toolchain | workflows use only `devcontainers/ci@v0.3`; grep for `setup-python`, `pipx install`, `actions/cache` in `.github/workflows/` must be empty |
| ufbt `CDB` wrote `flipp_pomodoro/.vscode/compile_commands.json` | ignored in `.gitignore`; do not delete it as a stray file |
| stale submodule working trees reported API 87.1 while gitlinks were 88.2/88.26 | baseline read from gitlink commits, never from submodule `HEAD` |
| multi-GB submodule deletion was nearly done with `rm -rf` | move to a backup first; verify both builds before committing the retirement |
| submodule bump was the upstream-API trigger | Task 3a nightly SDK drift PR, with `docs/**` in `ci.yml` paths |
| plan's literal grep criteria contradicted the tracked plan itself | interpret those greps as "no live reference outside this document" |
| `shellcheck` is absent in the current container | add it to the Dockerfile and record that `just lint` cannot prove it until the image is rebuilt |

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
