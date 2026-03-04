#!/bin/bash
# Build script for WebUI — creates JS bundle + .gz versions for ESP32 deployment.
# The ESP32 RestServer serves .gz files exclusively (Content-Encoding: gzip).
#
# Usage:  cd sdcard_image/www && ./build-webui.sh
#
# This script:
#   1. Concatenates 6 JS source files into js/app-bundle.js (reduces 6 HTTP
#      requests to 1 — critical for ESP32 httpd's 7-socket limit)
#   2. Gzips all assets for production deployment
#
# Note: The full build pipeline (create_sd_archive.sh) also gzips these files.
#       This script is for quick local testing of gzipped assets.

set -e
cd "$(dirname "$0")"

# ── Step 1: Create JS bundle ──
# Order matters — dependency chain: Sortable (vendor) → shared → display-hints
# → plugin-manager → sample-manager → app (shell, boots last)
BUNDLE_SOURCES=(
  js/Sortable.min.js
  js/webaudio-controls.js
  js/shared.js
  js/display-hints.js
  js/plugin-manager.js
  js/sample-manager.js
  js/app.js
)
BUNDLE_OUT="js/app-bundle.js"

echo "Building JS bundle..."
> "$BUNDLE_OUT"
for src in "${BUNDLE_SOURCES[@]}"; do
  if [ -f "$src" ]; then
    echo "// ── $(basename "$src") ───" >> "$BUNDLE_OUT"
    cat "$src" >> "$BUNDLE_OUT"
    echo "" >> "$BUNDLE_OUT"
    printf "  + %-30s\n" "$src"
  else
    echo "  WARN: $src not found, skipping"
  fi
done

bundle_size=$(wc -c < "$BUNDLE_OUT" | tr -d ' ')
echo "  → $BUNDLE_OUT ($bundle_size bytes)"
echo ""

# ── Step 2: Gzip all production assets ──
# Only the files that the ESP32 actually serves need .gz versions.
# Individual JS source files are NOT gzipped — only the bundle is served.
GZIP_FILES=(
  index.html
  js/app-bundle.js
  js/shoelace-bundle.js
  shoelace/themes/dark.css
)

echo "Creating gzip assets..."
for f in "${GZIP_FILES[@]}"; do
  if [ -f "$f" ]; then
    gzip -k -f -9 "$f"
    orig=$(wc -c < "$f" | tr -d ' ')
    comp=$(wc -c < "$f.gz" | tr -d ' ')
    ratio=$((100 - comp * 100 / orig))
    printf "  %-40s %6s → %6s  (%d%% smaller)\n" "$f" "$orig" "$comp" "$ratio"
  else
    echo "  SKIP: $f (not found)"
  fi
done

echo ""
echo "Done. Bundle + gzipped assets ready for ESP32 deployment."
