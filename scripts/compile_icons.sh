#!/bin/bash

SRC_DIR="$MESON_SOURCE_ROOT/icons/icons/svg"
OUTPUT_DIR="$MESON_BUILD_ROOT/icons"

# INPUT_COLOR="#000000"
# OUTPUT_COLOR="#000000"

rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

function generate_icon {
    # sed -e "s/$INPUT_COLOR/$OUTPUT_LIGHT_COLOR/" "$SRC_DIR/$1" > "$LIGHT_DIR/$2"
    cp "$SRC_DIR/$1" "$OUTPUT_DIR/$2";
}

generate_icon "account-plus.svg" "player-add.svg"
generate_icon "account-multiple-plus.svg" "category-add.svg"
generate_icon "filter.svg" "filter.svg"
generate_icon "eye-off.svg" "hide.svg"

