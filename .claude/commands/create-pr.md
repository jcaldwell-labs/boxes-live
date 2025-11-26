# Create Pull Request

Create a pull request with a properly formatted description.

## Instructions

When creating a PR, you MUST:

1. **Link the related issue** - Use "Fixes #N" or "Closes #N" in the PR body
2. **Fill in ALL template sections** - Do not leave placeholder text or empty sections
3. **Mark applicable checkboxes** - Use [x] for completed items
4. **Remove non-applicable sections** - Delete sections that don't apply rather than leaving them empty

## PR Body Format

Use this streamlined format (NOT the verbose template):

```markdown
## Summary

[2-3 sentences describing what this PR does and why]

Fixes #[issue-number]

## Changes

- [Key change 1]
- [Key change 2]
- [Key change 3]

## Testing

- [x] All existing tests pass (`make test`)
- [x] No memory leaks (`make valgrind`)
- [New test details if applicable]

## Type

- [x] New feature | Bug fix | Refactor | Docs | CI (pick one)
```

## Example

```markdown
## Summary

Add proportional box sizing feature that automatically sizes new boxes based on nearby existing boxes, improving visual consistency.

Fixes #18

## Changes

- Add `canvas_calc_proportional_size()` to find and average neighbor dimensions
- Add `[proportional]` config section with `enabled`, `proximity_radius`, `use_nearest_neighbor`, `min_neighbors_required`
- Integrate proportional sizing into `ACTION_CREATE_BOX` handler
- Update config.ini.example with documentation

## Testing

- [x] All 289 tests pass
- [x] No memory leaks (valgrind clean)
- [x] Manual testing with multiple box layouts

## Type

- [x] New feature
```

## Creating the PR

After preparing the description:

```bash
gh pr create --title "Brief descriptive title" --body "$(cat <<'EOF'
[Your formatted PR body here]
EOF
)"
```
