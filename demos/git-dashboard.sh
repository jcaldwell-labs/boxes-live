#!/bin/bash
# Git Dashboard Demo - Live git repository visualization
#
# Shows what's possible: combining connectors + live updates

set -e

REPO_PATH="${1:-.}"
CANVAS="git_dashboard.txt"
INTERVAL=5

echo "========================================="
echo "Git Dashboard Demo"
echo "========================================="
echo ""
echo "Repository: $REPO_PATH"
echo "Canvas: $CANVAS"
echo "Update interval: ${INTERVAL}s"
echo ""

# Create canvas
../connectors/boxes-cli create --width 250 --height 120 "$CANVAS"

# Box 1: Repository Info
../connectors/boxes-cli add "$CANVAS" \
    --x 5 --y 5 --width 50 --height 10 \
    --title "Repository" --color 2 \
    --content "Loading..."

# Box 2: Recent Commits
../connectors/boxes-cli add "$CANVAS" \
    --x 5 --y 18 --width 70 --height 20 \
    --title "Recent Commits" --color 3 \
    --content "Loading..."

# Box 3: Branches
../connectors/boxes-cli add "$CANVAS" \
    --x 80 --y 5 --width 40 --height 25 \
    --title "Branches" --color 4 \
    --content "Loading..."

# Box 4: File Changes
../connectors/boxes-cli add "$CANVAS" \
    --x 5 --y 42 --width 70 --height 15 \
    --title "Uncommitted Changes" --color 1 \
    --content "Loading..."

# Box 5: Stats
../connectors/boxes-cli add "$CANVAS" \
    --x 125 --y 5 --width 35 --height 15 \
    --title "Statistics" --color 5 \
    --content "Loading..."

echo "Canvas created: $CANVAS"
echo "Starting live updates..."
echo ""

cd "$REPO_PATH"

while true; do
    timestamp=$(date "+%H:%M:%S")
    echo "[$timestamp] Updating git dashboard..."

    # Box 1: Repo info
    repo_name=$(basename "$(git rev-parse --show-toplevel)")
    branch=$(git branch --show-current)
    remote=$(git remote get-url origin 2>/dev/null || echo "No remote")
    ../connectors/boxes-cli update "../$CANVAS" 1 \
        --content "Name: $repo_name" \
        --content "Branch: $branch" \
        --content "Remote: $remote" \
        --content "Updated: $timestamp"

    # Box 2: Recent commits
    commits=$(git log --oneline -5 --pretty=format:"%h %s")
    commit_lines=""
    while IFS= read -r line; do
        commit_lines="$commit_lines --content \"$line\""
    done <<< "$commits"
    eval ../connectors/boxes-cli update "../$CANVAS" 2 $commit_lines

    # Box 3: Branches
    branches=$(git branch -a | head -10)
    branch_lines=""
    while IFS= read -r line; do
        branch_lines="$branch_lines --content \"$line\""
    done <<< "$branches"
    eval ../connectors/boxes-cli update "../$CANVAS" 3 $branch_lines

    # Box 4: Uncommitted changes
    status=$(git status --short | head -8)
    if [ -z "$status" ]; then
        status="Working tree clean"
    fi
    status_lines=""
    while IFS= read -r line; do
        status_lines="$status_lines --content \"$line\""
    done <<< "$status"
    eval ../connectors/boxes-cli update "../$CANVAS" 4 $status_lines

    # Box 5: Stats
    total_commits=$(git rev-list --count HEAD)
    contributors=$(git shortlog -s -n | wc -l)
    files=$(git ls-files | wc -l)
    ../connectors/boxes-cli update "../$CANVAS" 5 \
        --content "Commits: $total_commits" \
        --content "Contributors: $contributors" \
        --content "Files: $files"

    echo "  ✓ Dashboard updated"
    sleep $INTERVAL
done
