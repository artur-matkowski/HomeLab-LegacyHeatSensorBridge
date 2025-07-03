#!/usr/bin/env bash
#
# ──────────────────────────────────────────────────────────────
#  version.sh ― derive a SemVer-style version from git history
# ──────────────────────────────────────────────────────────────
#
# Tags recognised in *commit subject lines* (keep the trailing colon):
#
#   api-break:   → backward-incompatible change          (bumps MAJOR)
#   release:     → backward-compatible feature addition  (bumps MINOR)
#
# Components
# ──────────
#   MAJOR  = total count of  api-break:  commits.
#   MINOR  = count of       release:  commits *since the last api-break*.
#   PATCH  = commit count   **since the last release:** commit.
#
#   Example:  3.7.15
#             └─┬─┘└─┬┘└──┬── commit count since the
#              │    │      │   most-recent release:
#              │    │      └─ MINOR bump occurrences
#              │    └─ api-break occurrences
#              └─ total api-breaks overall
#
# Dirty work-trees append  “wip on: <branch>”.
#
# Usage
# ─────
#   ./version.sh --short   → MAJOR.MINOR.PATCH
#   ./version.sh           → MAJOR.MINOR build:PATCH [wip …]
# -----------------------------------------------------------------


# ─── helper: SHA of the latest commit whose subject matches $1 ─────
latest_tag_sha() {
  git log --oneline | grep -w "$1" | head -n1 | cut -d' ' -f1
}

# ─── compute MAJOR & MINOR ────────────────────────────────────────
major=$(git log --oneline | grep -ow 'api-break:' | wc -l)

last_api_break_sha=$(latest_tag_sha 'api-break:')
[[ -z $last_api_break_sha ]] && last_api_break_sha=$(git rev-list --max-parents=0 HEAD)

minor=$(git log --oneline "${last_api_break_sha}..HEAD" | grep -ow 'release:' | wc -l)

# ─── PATCH: commits since last release: ───────────────────────────
last_release_sha=$(latest_tag_sha 'release:')
[[ -z $last_release_sha ]] && last_release_sha=$last_api_break_sha   # fallback

patch=$(git rev-list --count "${last_release_sha}..HEAD")

if [[ "${1:-}" == "--short" ]]; then
  echo "${major}.${minor}.${patch}"
  exit 0
fi

# ─── full label (with optional WIP marker) ────────────────────────
patch_label="build:${patch}"

if [[ -n $(git status --porcelain) ]]; then
  patch_label+=" wip on: $(git rev-parse --abbrev-ref HEAD)"
fi

echo "${major}.${minor} ${patch_label}"