#!/bin/sh
set -e

REPO="PPeter2/CoreX"
INSTALL_DIR="$HOME/.corex"
BIN_DIR="$INSTALL_DIR/bin"

detect_platform() {
    OS=$(uname -s)
    ARCH=$(uname -m)

    case "$OS" in
        Linux)
            ASSET="corex-linux-x64"
            ;;
        Darwin)
            case "$ARCH" in
                arm64) ASSET="corex-macos-arm64" ;;
                x86_64) ASSET="corex-macos-x64" ;;
                *)
                    echo "corex install: unsupported macOS architecture '$ARCH'"
                    exit 1
                    ;;
            esac
            ;;
        *)
            echo "corex install: unsupported operating system '$OS'"
            exit 1
            ;;
    esac
}

fetch_latest_release_url() {
    API_URL="https://api.github.com/repos/$REPO/releases/latest"
    DOWNLOAD_URL=$(curl -fsSL "$API_URL" | grep "browser_download_url" | grep "$ASSET" | cut -d '"' -f 4)

    if [ -z "$DOWNLOAD_URL" ]; then
        echo "corex install: could not find a release asset named '$ASSET'"
        echo "corex install: check that $REPO has a published release"
        exit 1
    fi
}

install_binary() {
    mkdir -p "$BIN_DIR"
    echo "corex install: downloading $ASSET"
    curl -fsSL "$DOWNLOAD_URL" -o "$BIN_DIR/corex"
    chmod +x "$BIN_DIR/corex"
    echo "corex install: installed to $BIN_DIR/corex"
}

update_path() {
    SHELL_NAME=$(basename "$SHELL")
    PROFILE_FILE="$HOME/.profile"

    case "$SHELL_NAME" in
        bash) PROFILE_FILE="$HOME/.bashrc" ;;
        zsh) PROFILE_FILE="$HOME/.zshrc" ;;
    esac

    if ! echo "$PATH" | grep -q "$BIN_DIR"; then
        if ! grep -q "$BIN_DIR" "$PROFILE_FILE" 2>/dev/null; then
            echo "export PATH=\"$BIN_DIR:\$PATH\"" >> "$PROFILE_FILE"
            echo "corex install: added $BIN_DIR to PATH in $PROFILE_FILE"
            echo "corex install: run 'source $PROFILE_FILE' or restart your terminal"
        fi
    fi
}

detect_platform
fetch_latest_release_url
install_binary
update_path

echo "corex install: done. Run 'corex version' after restarting your terminal."
