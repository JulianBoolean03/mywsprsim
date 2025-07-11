# Quick Start Guide

## For Fresh Clones (VS Code/Neovim Users)

This repository is designed to work immediately after cloning. Here's how to get started:

### 1. Clone the Repository
```bash
git clone <your-repository-url> jtencode-sim
cd jtencode-sim
```

### 2. Run the Installation Script
```bash
./install.sh
```

**Choose Option 1** for full setup (recommended for first-time users)
- Installs all dependencies automatically
- Builds all project components
- Creates VS Code development setup
- Runs verification tests

### 3. Development Workflow

#### VS Code Users:
```bash
code .
```
- **Build**: `Ctrl+Shift+P` → "Tasks: Run Build Task"
- **Generate Signals**: `Ctrl+Shift+P` → "Tasks: Run Task" → "Generate WSPR Signals"
- **Debug**: F5 (debug WSPR generator)

#### Neovim Users:
```bash
nvim .
```
- **Build**: `:!./install.sh` (option 3)
- **Generate Signals**: `:!./wsprsim TEST FM04 20`
- **Run Tests**: `:!./wspr_project_menu.sh`

### 4. Quick Commands

```bash
# Generate WSPR signals
./wsprsim VK3ABC FM04 20

# Interactive menu
./wspr_project_menu.sh

# Test decoder matrix
./decode_norm_norm.sh  # Should work
./decode_alt_alt.sh    # Should work  
./decode_norm_alt.sh   # Should fail
./decode_alt_norm.sh   # Should fail
```

### 5. What Gets Created

After installation, you'll have:
- **Built executables**: `wsprsim`, decoders in `wspr-cui/`
- **Generated files**: `*.wav`, `*.bits`, `*.hex`
- **VS Code setup**: `.vscode/` with tasks and IntelliSense
- **All scripts**: Made executable and ready to use

### 6. Platform Support

The `install.sh` script automatically detects and supports:
- ✅ **macOS** (via Homebrew)
- ✅ **Ubuntu/Debian** (via apt)
- ✅ **RHEL/CentOS** (via yum)
- ✅ **Fedora** (via dnf)
- ✅ **Windows WSL** (via apt)
- ✅ **Windows MSYS2** (via pacman)

### 7. Troubleshooting

**Dependencies missing?**
```bash
./install.sh  # Choose option 2 to install dependencies only
```

**Build failed?**
```bash
./install.sh  # Choose option 3 to rebuild only
```

**Want to reset everything?**
```bash
git clean -fdx  # Remove all generated files
./install.sh    # Choose option 1 to start fresh
```

### 8. Project Structure

```
jtencode-sim/
├── src/                    # JTEncode library source
├── wspr-cui/               # WSPR decoders
├── wsprsim.cpp            # Signal generator source
├── install.sh             # One-command setup
├── wspr_project_menu.sh   # Interactive menu
├── decode_*.sh            # Test scripts
└── .vscode/               # VS Code development setup
```

## That's It!

After running `./install.sh`, your project is ready to use. The script handles everything automatically, and you can immediately start generating WSPR signals and testing the altered sync vector functionality.

**Need help?** Check `README.md` and `SETUP_GUIDE.md` for detailed information.
