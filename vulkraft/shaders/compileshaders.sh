#!/usr/bin/env bash

DIR="$(dirname "$(realpath "$0")")"
EXTENSIONS=("*.comp")
DESTDIR="$DIR/../run/shaders"

echo "$DESTDIR"

mkdir -p "$DESTDIR"

for EXT in "${EXTENSIONS[@]}"; do
  find "$DIR" -type f -name "$EXT" | while read -r FILE; do
    # Get the relative path of the file
    REL_PATH="${FILE#$DIR/}"
  
    # Create the corresponding subdirectory in the destination
    DEST_SUBDIR="$DESTDIR/$(dirname "$REL_PATH")"
    mkdir -p "$DEST_SUBDIR"
  
    # Define the destination file path
    DEST_FILE="$DEST_SUBDIR/$(basename "$REL_PATH" .comp).spv"

    # Copy the file only if it doesn't exist in the destination or if the source file is newer
    if [ ! -f "$DEST_FILE" ] || [ "$FILE" -nt "$DEST_FILE" ]; then
        "$DIR/glslc" -o "$DEST_FILE" -x hlsl --target-env=vulkan1.2 "$FILE"
    fi
  done
done