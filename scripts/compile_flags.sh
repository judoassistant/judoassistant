#!/bin/bash

SRC_DIR="$MESON_SOURCE_ROOT/flags/flags/4x3/"
OUTPUT_DIR="$MESON_BUILD_ROOT/flags"

# INPUT_COLOR="#000000"
# OUTPUT_COLOR="#000000"

rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

function generate_flag {
    # sed -e "s/$INPUT_COLOR/$OUTPUT_LIGHT_COLOR/" "$SRC_DIR/$1" > "$LIGHT_DIR/$2"
    cp "$SRC_DIR/$1.svg" "$OUTPUT_DIR/$2.svg";
}

generate_flag "dk" "den"
generate_flag "gb" "gbr"
generate_flag "fr" "fra"
