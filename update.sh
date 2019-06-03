#!/usr/bin/env bash
set -e

# Perform updates in this directory.
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
cd "$DIR"

function printHelp {
	echo "Usage: `basename "$0"` [options]"
	echo
	echo "Updates the code and dependencies. This simplifies updating the git submodules"
	echo "and managing pre-built tools and library dependencies."
	echo
	echo "Options:"
	echo "-a, --all                     Updates git, submodules, tools, and libraries. See"
	echo "                              the following options for more information of"
	echo "                              each stage. In the case of updating tools and"
	echo "                              libraries, it will only update existing installed"
	echo "                              packages without installing any new ones."
	echo "-m, --most                    Updates submodules, tools, and libraries. Use this"
	echo "                              instead of --all if you checked out a specific"
	echo "                              revision and want to update everything but the"
	echo "                              latest code."
	echo "-g, --git                     Pulls the latest commit of the current branch."
	echo "-s, --submodules              Update submodules."
	echo "-t, --tools                   Updates the tools required for building."
	echo "-l, --libs [platforms...]     Updates the library dependencies for the platforms"
	echo "                              listed. If no platforms are listed, the currently"
	echo "                              installed lilbrary platforms are updated. The"
	echo "                              supported platforms supported are:"
	echo "                              - linux (Linux with glibc 2.19 for x86-64)"
	echo "                              - mac (macOS 10.11 for x86-64)"
	echo "                              - ios (iOS 10.0 for ARM64)"
	echo "                              - win32 (Windows for x86)"
	echo "                              - win64 (Windows for x86-64)"
	echo "                              - android-x86 (Android for x86)"
	echo "                              - android-x86_64 (Android for x86-64)"
	echo "                              - android-armeabi-v7a (Android for ARM v7a)"
	echo "                              - android-arm64-v8a (Android for ARM64 v8a)"
	echo "                              - android-all (All Android platforms)"
	echo "-c, --clean                  Removes downloaded tool and library dependencies."
}

DEPENDENCIES_DIR="dependencies"
ANDROID_PLATFORMS=(android-x86 android-x86_64 android-armeabi-v7a android-arm64-v8a)
SUPPORTED_PLATFORMS=(linux mac ios win32 win64 ${ANDROID_PLATFORMS[@]})
GIT=0
SUBMODULES=0
TOOLS=0
LIBS=0
PLATFORMS=()
CLEAN=0
ANY=0

while [ $# -gt 0 ]
do
	case "$1" in
		-h|--help)
			printHelp
			exit 0
			;;
		-a|--all)
			GIT=1
			SUBMODULES=1
			if [ $TOOLS -eq 0 ]; then
				TOOLS=2
			fi
			LIBS=1
			ANY=1
			;;
		-m|--most)
			SUBMODULES=1
			if [ $TOOLS -eq 0 ]; then
				TOOLS=2
			fi
			LIBS=1
			ANY=1
			;;
		-g|--git)
			GIT=1
			ANY=1
			;;
		-s|--submodules)
			SUBMODULES=1
			ANY=1
			;;
		-t|--tools)
			TOOLS=1
			ANY=1
			;;
		-l|--libs)
			LIBS=1
			ANY=1
			shift
			while [ $# -gt 0 -a "${1:0:1}" != '-' ]
			do
				if [ "$1" = "android-all" ]; then
					PLATFORMS+=(${ANDROID_PLATFORMS[@]})
					shift
					continue
				fi

				FOUND=0
				for PLATFORM in "${SUPPORTED_PLATFORMS[@]}"; do
					if [ "$PLATFORM" = "$1" ]; then
						FOUND=1
						PLATFORMS+=("$1")
						break
					fi
				done
				if [ $FOUND -eq 0 ]; then
					echo "Unknown platform '$1'"
					echo
					printHelp
					exit 1
				fi
				shift
			done
			continue
			;;
		-c|--clean)
			CLEAN=1
			ANY=1
			;;
		*)
			echo "Unknown option '$1'"
			echo
			printHelp
			exit 1
	esac
	shift
done

if [ $ANY -eq 0 ]; then
	printHelp
	exit 0
fi

if [ $CLEAN -ne 0 ]; then
	rm -rf "$DEPENDENCIES_DIR"
fi

if [ $GIT -ne 0 ]; then
	echo "Updating git..."
	git pull
fi

if [ $SUBMODULES -ne 0 ]; then
	echo "Updating git submodules..."
	git submodule init
	git submodule update --recursive
fi

if [[ ($TOOLS -eq 2 && -d "$DEPENDENCIES_DIR/tools" ) || $TOOLS -eq 1 ]]; then
	case `uname` in
		Linux)
			PLATFORM=linux
			EXTENSION=tar.gz
			;;
		Darwin)
			PLATFORM=mac
			EXTENSION=tar.gz
			;;
		CYGWIN*|MINGW*)
			PLATFORM=win32
			EXTENSION=zip
			;;
		*)
			echo "WARNING: Uknown platform, defaulting to Linux."
			PLATFORM=linux
			EXTENSION=tar.gz
			;;
	esac

	echo "Updating tools..."
	VERSION=`cat scripts/tools.version`
	FILE="DeepSea-tools-$PLATFORM.$EXTENSION"
	scripts/update-package.sh \
		"https://github.com/akb825/DeepSea-tools/releases/download/v$VERSION/$FILE" \
		"$FILE" $VERSION "$DEPENDENCIES_DIR/tools"
fi

if [ $LIBS -ne 0 ]; then
	VERSION=`cat scripts/libs.version`
	for PLATFORM in "${SUPPORTED_PLATFORMS[@]}"; do
		UPDATE=0
		if [ -d "$DEPENDENCIES_DIR/libs/$PLATFORM" ]; then
			UPDATE=1
		else
			for SET_PLATFORM in "${PLATFORMS[@]}"; do
				if [ "$SET_PLATFORM" = "$PLATFORM" ]; then
					UPDATE=1
					break
				fi
			done
		fi

		if [ "${PLATFORM:0:3}" = "win" ]; then
			EXTENSION="zip"
		else
			EXTENSION="tar.gz"
		fi

		if [ $UPDATE -ne 0 ]; then
			echo "Updating libs for $PLATFORM..."
			FILE="DeepSea-libs-$PLATFORM.$EXTENSION"
			scripts/update-package.sh \
				"https://github.com/akb825/DeepSea-libs/releases/download/v$VERSION/$FILE" \
				"$FILE" $VERSION "$DEPENDENCIES_DIR/libs/$PLATFORM"
		fi
	done
fi

echo "Done."
