#!/usr/bin/env bash

DIR="$(dirname "$(realpath "$0")")"
SHADERSDIR="$(realpath "$DIR/../run/shaders")"

rm -r "$SHADERSDIR"