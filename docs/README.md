# Boxes-Live Documentation

Welcome to the boxes-live documentation. This directory contains user guides, tutorials, and reference documentation.

## Documentation Structure

```
docs/
├── guides/          # How-to guides for specific tasks
├── tutorials/       # Step-by-step learning materials
├── examples/        # Example configurations and use cases
└── reference/       # API and CLI reference (coming soon)
```

## Quick Links

### Getting Started

| Document | Description |
|----------|-------------|
| [Quick Start](../README.md#quick-start) | Install and run in 5 minutes |
| [Controls Reference](../README.md#controls-reference) | Keyboard and mouse controls |
| [USAGE.md](../USAGE.md) | Complete usage instructions |

### Guides

| Guide | Description |
|-------|-------------|
| [Connector Guide](CONNECTOR-GUIDE.md) | Using data connectors |
| [Workflow Examples](WORKFLOW-EXAMPLES.md) | Real-world workflow patterns |
| [Joystick Guide](../JOYSTICK-GUIDE.md) | Gamepad controls |
| [UAT Testing](UAT-TESTING.md) | User acceptance testing |

### Tutorials

| Tutorial | Description |
|----------|-------------|
| [First Canvas](tutorials/first-canvas.md) | Create your first canvas |

### Examples

| Example | Description |
|---------|-------------|
| [Project Dashboard](examples/project-dashboard.md) | Task tracking canvas |

### Reference

| Reference | Description |
|-----------|-------------|
| [Architecture](../ARCHITECTURE.md) | System design |
| [Protocol](../PROTOCOL.md) | Multi-user protocol spec |
| [Security Review](../SECURITY_REVIEW.md) | Security best practices |

## By Use Case

### "I want to visualize my Docker containers"

```bash
docker ps | ./connectors/docker2canvas > containers.txt
./boxes-live  # Press F3, load containers.txt
```

See [Workflow Examples](WORKFLOW-EXAMPLES.md) for more.

### "I want to use a gamepad"

See [Joystick Guide](../JOYSTICK-GUIDE.md) for controller setup.

### "I want to connect boxes-live to my app"

See [Connector Guide](CONNECTOR-GUIDE.md) for building custom connectors.

### "I want to contribute"

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## Additional Documentation

Root-level documentation files:

- [README.md](../README.md) - Project overview
- [ARCHITECTURE.md](../ARCHITECTURE.md) - System architecture
- [CONTRIBUTING.md](../CONTRIBUTING.md) - Contribution guidelines
- [CHANGELOG.md](../CHANGELOG.md) - Version history
- [CAMPAIGN_ORCHESTRATION.md](../CAMPAIGN_ORCHESTRATION.md) - RPG integration

## Planning & Roadmap

See [.github/planning/](../.github/planning/) for development roadmap and planning documents.

---

**Last Updated**: 2024-12-23
