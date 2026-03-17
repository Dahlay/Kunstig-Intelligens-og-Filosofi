#!/usr/bin/env bash
# packaging/macos/notarize.sh
#
# Codesign + notarize the macOS app bundle for distribution outside the App Store.
#
# Prerequisites:
#   - Xcode command-line tools
#   - Valid "Developer ID Application" certificate in Keychain
#   - Notarytool credentials stored via:
#       xcrun notarytool store-credentials "MAP_PROFILE" --apple-id ... --team-id ... --password ...
#
# Usage:
#   ./packaging/macos/notarize.sh \
#       build/ModularAudioPatcher_artefacts/Release/"Modular Audio Patcher.app" \
#       "Developer ID Application: Your Name (TEAMID)" \
#       MAP_PROFILE

set -euo pipefail

APP_PATH="${1:?Usage: $0 <App.app> <SIGN_IDENTITY> <NOTARYTOOL_PROFILE>}"
SIGN_IDENTITY="${2:?}"
NOTARYTOOL_PROFILE="${3:?}"
ENTITLEMENTS_PLIST="$(dirname "$0")/entitlements.plist"
ZIP_PATH="${APP_PATH%.app}.zip"

echo "==> Signing $APP_PATH"
codesign --deep --force --timestamp --options runtime \
  --entitlements "$ENTITLEMENTS_PLIST" \
  --sign "$SIGN_IDENTITY" \
  "$APP_PATH"

echo "==> Verifying signature"
codesign --verify --verbose=4 "$APP_PATH"
spctl --assess --verbose=4 --type exec "$APP_PATH" || true

echo "==> Creating ZIP for notarization"
ditto -c -k --keepParent "$APP_PATH" "$ZIP_PATH"

echo "==> Submitting to Apple Notary Service (profile: $NOTARYTOOL_PROFILE)"
xcrun notarytool submit "$ZIP_PATH" \
  --keychain-profile "$NOTARYTOOL_PROFILE" \
  --wait

echo "==> Stapling notarization ticket"
xcrun stapler staple "$APP_PATH"

echo "==> Done. App is signed and notarized: $APP_PATH"
