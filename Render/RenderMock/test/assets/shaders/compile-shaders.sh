#!/bin/sh

CUR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$CUR_DIR/../../../.."
INCLUDE_DIR="$ROOT_DIR/Render/include"

mslc -c "$CUR_DIR/glsl.conf" -o "$CUR_DIR/test.mslb" -I "$INCLUDE_DIR" "$CUR_DIR/test.msl"
mslc -c "$CUR_DIR/glsl-nobuffers.conf" -o "$CUR_DIR/test-nobuffers.mslb" -I "$INCLUDE_DIR" "$CUR_DIR/test.msl"
