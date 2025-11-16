# GitHub Setup Instructions

This document provides step-by-step instructions for uploading the boxes-live project to a new GitHub repository.

## Prerequisites

- GitHub account
- Git installed locally
- SSH key configured with GitHub (recommended) or HTTPS credentials

## Steps to Upload to GitHub

### 1. Create a New Repository on GitHub

1. Go to [GitHub](https://github.com)
2. Click the **+** icon in the top right corner
3. Select **New repository**
4. Fill in the repository details:
   - **Repository name**: `boxes-live`
   - **Description**: "Terminal-based interactive canvas application with pan and zoom - like Miro for the terminal"
   - **Visibility**: Choose Public or Private
   - **DO NOT** initialize with README, .gitignore, or license (we already have these)
5. Click **Create repository**

### 2. Link Your Local Repository to GitHub

GitHub will show you commands after creating the repository. Use the "push an existing repository" section:

#### Using SSH (Recommended):
```bash
git remote add origin git@github.com:YOUR_USERNAME/boxes-live.git
git push -u origin main
```

#### Using HTTPS:
```bash
git remote add origin https://github.com/YOUR_USERNAME/boxes-live.git
git push -u origin main
```

Replace `YOUR_USERNAME` with your actual GitHub username.

### 3. Verify the Upload

1. Refresh your GitHub repository page
2. You should see all files uploaded:
   - Source code (src/, include/)
   - Documentation (README.md, USAGE.md, CLAUDE.md)
   - Build files (Makefile)
   - LICENSE file
   - .gitignore

### 4. Configure Repository Settings (Optional but Recommended)

#### Add Topics
Go to your repository page and click the gear icon next to "About" to add topics:
- `c`
- `terminal`
- `ncurses`
- `canvas`
- `pan-zoom`
- `interactive`
- `miro`

#### Add Description
Use the same description from step 1 above.

#### Enable Issues and Discussions (if desired)
- Go to Settings > General
- Enable Issues for bug tracking and feature requests
- Enable Discussions for community engagement

## Current Repository Status

✅ Git repository initialized on main branch
✅ Initial commit created with all project files
✅ Ready to push to GitHub

## Project Structure

```
boxes-live/
├── .gitignore          # Git ignore rules
├── CLAUDE.md           # Architecture documentation
├── LICENSE             # MIT License
├── Makefile            # Build system
├── README.md           # Project overview
├── USAGE.md            # Usage instructions
├── include/            # Header files (5 files)
│   ├── input.h
│   ├── render.h
│   ├── terminal.h
│   ├── types.h
│   └── viewport.h
└── src/                # Source files (5 files)
    ├── input.c
    ├── main.c
    ├── render.c
    ├── terminal.c
    └── viewport.c
```

## Commit Information

- **Commit Hash**: cf6d362
- **Files**: 16
- **Lines Added**: 987
- **Author**: BE Dev Agent
- **Co-Author**: Claude

## Quick Reference Commands

```bash
# View current status
git status

# View commit history
git log --oneline

# View remote repositories
git remote -v

# View files in repository
git ls-files

# Create a new branch (if needed)
git checkout -b feature/new-feature

# Push changes to GitHub
git push origin main
```

## Next Steps After Upload

1. **Update repository description** on GitHub with project details
2. **Add topics** to make the project discoverable
3. **Create a release** (optional): Tag v1.0.0 for the initial version
4. **Enable GitHub Pages** (optional): For hosting documentation
5. **Set up GitHub Actions** (optional): For CI/CD
6. **Star your own repository** to bookmark it!

## Troubleshooting

### Permission Denied (SSH)
If you get "Permission denied (publickey)":
1. Check if you have an SSH key: `ls -la ~/.ssh`
2. Generate one if needed: `ssh-keygen -t ed25519 -C "your_email@example.com"`
3. Add it to GitHub: Settings > SSH and GPG keys > New SSH key
4. Test connection: `ssh -T git@github.com`

### Remote Already Exists
If you get "remote origin already exists":
```bash
git remote remove origin
git remote add origin git@github.com:YOUR_USERNAME/boxes-live.git
```

### Authentication Failed (HTTPS)
If using HTTPS and authentication fails:
1. Use a Personal Access Token instead of password
2. Generate token: GitHub Settings > Developer settings > Personal access tokens
3. Use token as password when prompted

## Support

For issues or questions:
- Check the [README.md](README.md) for project overview
- Review [CLAUDE.md](CLAUDE.md) for architecture details
- Consult [USAGE.md](USAGE.md) for usage instructions
- Open an issue on GitHub after uploading
