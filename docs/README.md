# Boxes-Live Documentation

Welcome to the boxes-live documentation hub. This directory contains comprehensive guides for using and developing boxes-live.

## 📚 Documentation Index

### Getting Started
- **[Quick Start Guide](../QUICK_START.md)** - 6-minute introduction for new users
- **[README](../README.md)** - Project overview and features
- **[Usage Guide](../USAGE.md)** - Complete usage instructions
- **[Examples](../EXAMPLES.md)** - Usage examples and demonstrations

### User Guides
- **[Joystick Guide](../JOYSTICK-GUIDE.md)** - Gamepad/joystick usage
- **[Advanced Usage](../ADVANCED_USAGE.md)** - Advanced features and techniques
- **[Demo Guide](../DEMO-GUIDE.md)** - Feature showcase and demos
- **[Quick Demo](../QUICK_DEMO.md)** - Quick demonstration script

### Testing & Quality
- **[UAT Testing Guide](UAT-TESTING.md)** - User acceptance testing documentation ⭐ NEW
- **[Testing Guide](../TESTING.md)** - Automated testing documentation
- **[Security Review](../SECURITY_REVIEW.md)** - Security analysis and best practices
- **[Performance](../PERFORMANCE.md)** - Performance benchmarks and optimization

### Development
- **[Architecture](../ARCHITECTURE.md)** - System design and architecture
- **[Contributing](../CONTRIBUTING.md)** - Contribution guidelines
- **[Claude AI Guide](../CLAUDE.md)** - AI agent guidance for development

### Connectors & Integration
- **[Connector Guide](CONNECTOR-GUIDE.md)** - Complete connector documentation
- **[Workflow Examples](WORKFLOW-EXAMPLES.md)** - Real-world connector workflows
- **[Connectors Overview](../CONNECTORS.md)** - Connector ecosystem
- **[MCP Server](../MCP_SERVER.md)** - AI agent integration via MCP
- **[Plugin Architecture](../PLUGIN_ARCHITECTURE.md)** - Live-updating plugins

### Advanced Topics
- **[Campaign Orchestration](../CAMPAIGN_ORCHESTRATION.md)** - Multi-pane tmux campaigns
- **[Protocol Specification](../PROTOCOL.md)** - Multi-user collaboration protocol
- **[Realm Engine Integration](../REALM_ENGINE_INTEGRATION.md)** - Adventure game integration

### Phase Documentation
- **[Phase 2: Parameter Editor](../PHASE2-PARAMETER-EDITOR.md)**
- **[Phase 3: Text Editing](../PHASE3-TEXT-EDITING.md)**
- **[Phase 4: Grid System](../PHASE4-GRID-SYSTEM.md)**
- **[Phase 5A: Config System](../PHASE5A-CONFIG-SYSTEM.md)**
- **[Phase 5B: Focus Mode](../PHASE5B-FOCUS-MODE.md)**

### Project Management
- **[Feature Roadmap](../FEATURE_ROADMAP.md)** - Planned features and timeline
- **[Changelog](../CHANGELOG.md)** - Version history and changes
- **[Project Summary](../PROJECT_SUMMARY.md)** - High-level project overview
- **[Release Readiness](../RELEASE_READINESS.md)** - Release checklist

---

## 🎯 By Use Case

### I want to get started quickly
1. [Quick Start Guide](../QUICK_START.md) - Start here!
2. [README](../README.md) - Understand what boxes-live does
3. [test-canvases/](../test-canvases/) - Try example canvases

### I want to test the application
1. **[UAT Testing Guide](UAT-TESTING.md)** - User acceptance testing ⭐
2. [Testing Guide](../TESTING.md) - Automated tests
3. [test-uat.sh](../test-uat.sh) - Run automated UAT scenarios

### I want to use a joystick/gamepad
1. [Joystick Guide](../JOYSTICK-GUIDE.md) - Complete joystick documentation
2. [Joystick UX Redesign](../JOYSTICK-UX-REDESIGN.md) - Design rationale
3. [config.ini.example](../config.ini.example) - Button mapping customization

### I want to use connectors
1. [Connector Guide](CONNECTOR-GUIDE.md) - Complete guide
2. [Workflow Examples](WORKFLOW-EXAMPLES.md) - Real-world examples
3. [Connectors Overview](../CONNECTORS.md) - Ecosystem overview

### I want to contribute
1. [Contributing](../CONTRIBUTING.md) - Guidelines
2. [Architecture](../ARCHITECTURE.md) - System design
3. [Testing Guide](../TESTING.md) - How to test

### I want to understand the architecture
1. [Architecture](../ARCHITECTURE.md) - Complete architecture
2. [Protocol](../PROTOCOL.md) - Network protocol spec
3. [Plugin Architecture](../PLUGIN_ARCHITECTURE.md) - Plugin system

---

## 📋 Quick Reference

### Installation
```bash
# Build
make clean && make

# Run
./boxes-live

# Test
make test           # Run automated tests
./test-uat.sh all   # Run UAT scenarios
```

### Common Tasks
- **Create box**: Press `n`
- **Toggle grid**: Press `G`
- **Save canvas**: Press `F2`
- **Load canvas**: Press `F3`
- **Quit**: Press `Q` or `ESC`

See [Quick Start Guide](../QUICK_START.md) for more.

---

## 🔗 External Resources

### GitHub
- **Repository**: https://github.com/jcaldwell-labs/boxes-live
- **Issues**: https://github.com/jcaldwell-labs/boxes-live/issues
- **Wiki**: _(Coming soon - see UAT-TESTING.md for latest testing documentation)_

### Community
- Discussions: GitHub Discussions (when enabled)
- Bug Reports: GitHub Issues
- Feature Requests: GitHub Issues with `enhancement` label

---

## 📝 Recent Updates

### November 2024
- ✅ **UAT Infrastructure Complete** - Comprehensive testing framework added
- ✅ **Grid Persistence Fixed** - Grid settings now save/load correctly
- ✅ **Documentation Expansion** - 5 new guides added
- ✅ **Test Coverage Increase** - 442 test assertions (was 245)

See [UAT Testing Guide](UAT-TESTING.md) for details.

---

## 🤝 Contributing to Documentation

Documentation improvements are welcome! Please:
1. Check existing docs before creating new ones
2. Follow the markdown formatting in existing files
3. Update this index when adding new documentation
4. Submit via pull request

---

## 📞 Getting Help

1. **Quick questions**: Check [Quick Start Guide](../QUICK_START.md)
2. **Bugs**: Open an issue on GitHub
3. **Feature requests**: Open an issue with `enhancement` label
4. **General help**: Check relevant guide above or [README](../README.md)

---

**Last Updated**: 2024-11-26  
**Documentation Version**: 1.2
