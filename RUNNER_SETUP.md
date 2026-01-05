# Self-Hosted GitHub Actions Runner Setup Guide

This document provides step-by-step instructions for provisioning GitHub Actions self-hosted runners for the DaktLib-GUI project across Windows (x86/x64), Linux, and macOS platforms.

## Overview

DaktLib-GUI requires **platform-specific** and **architecture-specific** builds:
- **Windows**: x86, x64
- **Linux**: x64
- **macOS**: x64 (Intel), ARM64 (Apple Silicon)

Each runner must have:
1. Git
2. CMake 4.21+ with Ninja generator
3. C++ compiler (MSVC for Windows, Clang/GCC for others)
4. Vulkan SDK (for Vulkan backend validation)
5. Platform-specific SDKs (DirectX SDK for Windows, etc.)

---

## Prerequisites

### GitHub Setup
1. Go to your repository → **Settings → Actions → Runners**
2. Click **New self-hosted runner**
3. Note the registration token and runner group URL

### Common to All Platforms
- Dedicated machine or VM for each runner (do not share across architectures)
- Minimum 4 GB RAM, 20 GB free disk space
- Network connectivity to GitHub

---

## Windows x86 Runner Setup

### 1. Environment Preparation
```powershell
# Run as Administrator
$runnerUser = "actions-runner"
$runnerDir = "C:\runners\win-x86"

# Create directory
New-Item -ItemType Directory -Path $runnerDir -Force

# Download actions-runner
$downloadUrl = "https://github.com/actions/runner/releases/download/v2.311.0/actions-runner-win-x86-2.311.0.zip"
$zipPath = "$runnerDir\runner.zip"
Invoke-WebRequest -Uri $downloadUrl -OutFile $zipPath
Expand-Archive -Path $zipPath -DestinationPath $runnerDir
Remove-Item $zipPath
```

### 2. Install Build Tools
```powershell
# Install Visual Studio 2022 (Community or Enterprise)
# Include: MSVC, Windows SDK, C++ build tools
# Or use winget:
winget install Microsoft.VisualStudio.2022.Community

# Install CMake
winget install Kitware.CMake

# Install Ninja
winget install Ninja-build.Ninja

# Verify installations
cmake --version
ninja --version
```

### 3. Install Vulkan SDK
```powershell
# Download from https://vulkan.lunarg.com/sdk/home
# Run installer, add to PATH
# Verify:
vkcube  # Or: vulkaninfo
```

### 4. Register Runner
```powershell
cd $runnerDir

# Run configuration script (replace TOKEN and RUNNER_GROUP_URL)
.\config.cmd `
  --url https://github.com/YOUR_ORG/DaktLib-GUI `
  --token <REGISTRATION_TOKEN> `
  --runnergroup default `
  --name "win-x86-runner-1" `
  --labels "windows,x86" `
  --unattended

# Install service (runs as SYSTEM)
.\config.cmd --service install

# Start service
Start-Service "GitHub Actions Runner (win-x86-runner-1)"

# Verify
Get-Service "GitHub Actions Runner*"
```

### 5. Verify Build Environment
```powershell
# Check MSVC
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.3*\bin\Hostx86\x86\cl.exe"

# Test build
cd C:\workspace\DaktLib-GUI
cmake -S . -B build-x86 -G "Ninja" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER="cl.exe" `
  -DCMAKE_TOOLCHAIN_FILE="toolchains/windows-x86.cmake"
ninja -C build-x86
```

---

## Windows x64 Runner Setup

### 1. Environment Preparation
```powershell
$runnerDir = "C:\runners\win-x64"
New-Item -ItemType Directory -Path $runnerDir -Force

$downloadUrl = "https://github.com/actions/runner/releases/download/v2.311.0/actions-runner-win-x64-2.311.0.zip"
$zipPath = "$runnerDir\runner.zip"
Invoke-WebRequest -Uri $downloadUrl -OutFile $zipPath
Expand-Archive -Path $zipPath -DestinationPath $runnerDir
Remove-Item $zipPath
```

### 2. Install Build Tools
Same as x86 (Visual Studio, CMake, Ninja, Vulkan SDK already installed)

### 3. Register Runner
```powershell
cd $runnerDir

.\config.cmd `
  --url https://github.com/YOUR_ORG/DaktLib-GUI `
  --token <REGISTRATION_TOKEN> `
  --name "win-x64-runner-1" `
  --labels "windows,x64" `
  --unattended

.\config.cmd --service install
Start-Service "GitHub Actions Runner (win-x64-runner-1)"
```

### 4. Verify Build Environment
```powershell
cd C:\workspace\DaktLib-GUI
cmake -S . -B build-x64 -G "Ninja" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER="cl.exe" `
  -DCMAKE_TOOLCHAIN_FILE="toolchains/windows-x64.cmake"
ninja -C build-x64
```

---

## Linux x64 Runner Setup

### 1. Install Dependencies (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y \
  git \
  curl \
  build-essential \
  cmake \
  ninja-build \
  clang \
  libvulkan-dev \
  vulkan-tools \
  libwayland-dev \
  libxcb-render0-dev \
  libxcb-shape0-dev

# Verify
cmake --version
ninja --version
g++ --version
```

### 2. Environment Preparation
```bash
RUNNER_DIR="/opt/runners/linux-x64"
sudo mkdir -p "$RUNNER_DIR"
cd "$RUNNER_DIR"

RUNNER_VERSION="2.311.0"
DOWNLOAD_URL="https://github.com/actions/runner/releases/download/v${RUNNER_VERSION}/actions-runner-linux-x64-${RUNNER_VERSION}.tar.gz"

sudo curl -L -o runner.tar.gz "$DOWNLOAD_URL"
sudo tar xzf runner.tar.gz
sudo rm runner.tar.gz
sudo chown -R $(whoami) "$RUNNER_DIR"
```

### 3. Install Vulkan SDK
```bash
# Using distro packages
sudo apt install -y vulkan-sdk

# Or from https://vulkan.lunarg.com/sdk/home
# Download and run installer
```

### 4. Register Runner
```bash
cd "$RUNNER_DIR"

./config.sh \
  --url https://github.com/YOUR_ORG/DaktLib-GUI \
  --token <REGISTRATION_TOKEN> \
  --name "linux-x64-runner-1" \
  --labels "linux,x64" \
  --unattended

# Install service (systemd)
sudo ./svc.sh install
sudo systemctl start actions.runner.*

# Verify
sudo systemctl status actions.runner.*
```

### 5. Verify Build Environment
```bash
cd /workspace/DaktLib-GUI
cmake -S . -B build-linux \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_TOOLCHAIN_FILE=toolchains/linux-x64.cmake

ninja -C build-linux
```

---

## macOS x64 (Intel) Runner Setup

### 1. Install Dependencies
```bash
# Using Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

brew install cmake ninja clang vulkan-sdk

# Verify
cmake --version
ninja --version
clang++ --version
```

### 2. Environment Preparation
```bash
RUNNER_DIR="$HOME/runners/macos-x64"
mkdir -p "$RUNNER_DIR"
cd "$RUNNER_DIR"

RUNNER_VERSION="2.311.0"
DOWNLOAD_URL="https://github.com/actions/runner/releases/download/v${RUNNER_VERSION}/actions-runner-osx-x64-${RUNNER_VERSION}.tar.gz"

curl -L -o runner.tar.gz "$DOWNLOAD_URL"
tar xzf runner.tar.gz
rm runner.tar.gz
```

### 3. Xcode Setup
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Optionally install full Xcode from App Store
```

### 4. Register Runner
```bash
cd "$RUNNER_DIR"

./config.sh \
  --url https://github.com/YOUR_ORG/DaktLib-GUI \
  --token <REGISTRATION_TOKEN> \
  --name "macos-x64-runner-1" \
  --labels "macos,x64" \
  --unattended

# Install service (launchd)
./svc.sh install
launchctl start actions.runner.*

# Verify
launchctl list | grep actions
```

### 5. Verify Build Environment
```bash
cd ~/workspace/DaktLib-GUI
cmake -S . -B build-macos \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DCMAKE_TOOLCHAIN_FILE=toolchains/macos-x64.cmake

ninja -C build-macos
```

---

## macOS ARM64 (Apple Silicon) Runner Setup

### 1. Install Dependencies (same as x64)
```bash
brew install cmake ninja clang vulkan-sdk
```

### 2. Environment Preparation
```bash
RUNNER_DIR="$HOME/runners/macos-arm64"
mkdir -p "$RUNNER_DIR"
cd "$RUNNER_DIR"

# Download ARM64 runner
RUNNER_VERSION="2.311.0"
DOWNLOAD_URL="https://github.com/actions/runner/releases/download/v${RUNNER_VERSION}/actions-runner-osx-arm64-${RUNNER_VERSION}.tar.gz"

curl -L -o runner.tar.gz "$DOWNLOAD_URL"
tar xzf runner.tar.gz
rm runner.tar.gz
```

### 3. Register Runner
```bash
cd "$RUNNER_DIR"

./config.sh \
  --url https://github.com/YOUR_ORG/DaktLib-GUI \
  --token <REGISTRATION_TOKEN> \
  --name "macos-arm64-runner-1" \
  --labels "macos,arm64" \
  --unattended

./svc.sh install
launchctl start actions.runner.*
```

### 4. Verify Build Environment
```bash
cd ~/workspace/DaktLib-GUI
cmake -S . -B build-arm64 \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_TOOLCHAIN_FILE=toolchains/macos-arm64.cmake

ninja -C build-arm64
```

---

## GitHub Secrets Configuration

### NuGet API Token
1. Create **GitHub Secret**: `NUGET_API_KEY`
2. Value: Your nuget.org API key (obtain from nuget.org → API Keys)
3. Used in `.github/workflows/release.yml` for package publishing

### Optional: Signing Certificates (Future)
- If NuGet packages require code signing, store certificate in GitHub Secrets as `SIGNING_CERT_BASE64`

---

## Maintenance

### Update Runner
```bash
# All platforms: stop service, update, restart
# Windows:
Stop-Service "GitHub Actions Runner*"
# Download new release, extract, restart

# Linux/macOS:
systemctl stop actions.runner.*
# Download new release, extract, restart
```

### Monitor Runner Health
- GitHub UI: **Settings → Runners** shows status (Idle, Running, Offline)
- Check logs:
  - Windows: `C:\runners\win-x64\_diag\...`
  - Linux/macOS: `$RUNNER_DIR/_diag/...`

### Common Issues

**Runner offline/not responding:**
- Check network connectivity
- Restart service: `systemctl restart actions.runner.*` (Linux/macOS)
- Re-register if token expired

**Build failures on specific architecture:**
- Verify CMake/Ninja versions match requirements
- Check compiler installation
- Test local builds before pushing

---

## Next Steps

1. Provision all 5 runners (win-x86, win-x64, linux-x64, macos-x64, macos-arm64)
2. Verify status in GitHub: **Settings → Runners**
3. Create GitHub Actions workflows (see `.github/workflows/` in repository)
4. Test first workflow run on each runner
