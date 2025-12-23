# jcaldwell-labs Organization Guidelines

Standards for project organization, documentation, and discoverability across the jcaldwell-labs organization.

## Repository Structure

### Root Directory (Essential Files Only)

Keep the root directory minimal:

```
project-root/
├── README.md           # Project overview
├── LICENSE             # License file
├── .gitignore          # Git ignore patterns
├── Makefile            # Build configuration (or equivalent)
├── llms.txt            # AI discoverability
├── CLAUDE.md           # AI agent guidance
├── CONTRIBUTING.md     # Contribution guidelines
├── CHANGELOG.md        # Version history
├── src/                # Source code
├── include/            # Header files
├── tests/              # Test files
├── docs/               # User documentation
└── .github/            # GitHub configuration
```

### Documentation Hierarchy

**Rule**: If a user would read it → `docs/`. If only maintainers need it → `.github/planning/`.

```
docs/
├── README.md           # Documentation index
├── guides/             # How-to guides
├── tutorials/          # Step-by-step learning
├── examples/           # Example code/configs
└── reference/          # API/CLI reference

.github/
├── planning/           # Internal planning docs
│   ├── ROADMAP.md      # Feature roadmap
│   └── backlog/        # Feature ideas, RFC docs
├── workflows/          # CI/CD workflows
├── ISSUE_TEMPLATE/     # Issue templates
└── PULL_REQUEST_TEMPLATE.md
```

## README Standards

Every README must include:

1. **Badges** - License, language, PRs welcome
2. **One-line description** - What the project does
3. **Why section** - Value proposition and differentiators
4. **Quick Start** - Usable in under 5 minutes
5. **Features** - Core capabilities
6. **Documentation links** - Easy navigation
7. **Contributing** - How to help
8. **License** - Clear licensing

**Quality bar**: Can a stranger understand what this does in 30 seconds?

## Discoverability

### GitHub Configuration

1. **Description**: "[What] - [Key Feature/Benefit]" (70-120 characters)
2. **Topics**: 5-10 relevant tags (language, category, features, use cases)
3. **Website**: Documentation URL if available

### Required Files

| File | Purpose |
|------|---------|
| `README.md` | Project overview and quick start |
| `llms.txt` | AI agent discoverability |
| `CLAUDE.md` | Claude Code agent guidance |
| `CONTRIBUTING.md` | How to contribute |
| `LICENSE` | Legal terms |

### llms.txt Format

```
# Project Name - One Line Description

> Brief paragraph explaining what this is for AI context

## Capabilities
- Key capability 1
- Key capability 2

## Quick Start
```code examples```

## Common Patterns
```usage examples for AI agents```

## Repository
https://github.com/jcaldwell-labs/project
```

## Code Quality

### C Projects (like boxes-live)

- Compile with `-Wall -Wextra -Werror`
- Zero memory leaks (verified with valgrind)
- Modular architecture (separate .h and .c files)
- Comprehensive test coverage

### All Projects

- Automated tests in CI
- Clear build instructions
- Documented dependencies

## Pull Request Standards

Every PR must:

1. Link to an issue (`Fixes #N`)
2. Have a clear summary
3. List actual changes (no empty bullets)
4. Show test results
5. Use the project's PR template

## Maturity Levels

| Level | Description | Requirements |
|-------|-------------|--------------|
| Minimal | Basic functionality | README, LICENSE, builds |
| Standard | Documented | + docs/, CONTRIBUTING, tests |
| Polished | Discoverable | + llms.txt, topics, badges, examples |
| Showcase | Portfolio-ready | + demos, blog posts, social preview |

## Checklist for New Projects

- [ ] README with badges and quick start
- [ ] LICENSE file
- [ ] .gitignore appropriate for language
- [ ] llms.txt for AI discoverability
- [ ] CLAUDE.md for Claude Code
- [ ] GitHub description and topics
- [ ] Issue and PR templates
- [ ] CI workflow for tests
- [ ] docs/ with at least README.md

---

*Based on patterns from jcaldwell-labs/my-grid and applied across organization projects.*
