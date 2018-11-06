#!/usr/bin/env bash
set -e

URL="$1"
FILE="$2"
VERSION="$3"
DIRECTORY="$4"

CUR_VERSION=
if [ -f "$DIRECTORY/version" ]; then
	CUR_VERSION=`cat "$DIRECTORY/version"`
fi

if [ "$CUR_VERSION" = "$VERSION" ]; then
	exit 0
fi

rm -rf "$DIRECTORY"
mkdir -p "$DIRECTORY"
cd "$DIRECTORY"

curl -L "$URL" -o "$FILE"

if [ "${FILE: -4}" = ".zip" ]; then
	unzip "$FILE"
else
	tar xzf "$FILE"
fi

rm "$FILE"
echo $VERSION > "version"
