#!/bin/bash

SRC_DIR="$MESON_SOURCE_ROOT/icons/icons/svg"
OUTPUT_DIR="$MESON_BUILD_ROOT/icons"
DARK_DIR="$OUTPUT_DIR/dark"
LIGHT_DIR="$OUTPUT_DIR/light"

INPUT_COLOR="#000000"
OUTPUT_DARK_COLOR="#ffffff"
OUTPUT_LIGHT_COLOR="#000000"

rm -rf $OUTPUT_DIR
mkdir -p $DARK_DIR
mkdir -p $LIGHT_DIR

function generate_icon {
    sed -e "s/$INPUT_COLOR/$OUTPUT_DARK_COLOR/" "$SRC_DIR/$1" > "$DARK_DIR/$2"
    sed -e "s/$INPUT_COLOR/$OUTPUT_LIGHT_COLOR/" "$SRC_DIR/$1" > "$LIGHT_DIR/$2"
}

generate_icon "account-plus.svg" "player-add.svg"
generate_icon "account-remove.svg" "player-erase.svg"
generate_icon "account-multiple-plus.svg" "category-add.svg"
generate_icon "account-multiple-minus.svg" "category-erase.svg"
