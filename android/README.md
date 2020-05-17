# Android

This directory contains an Android Studio project to build the test applications to run on Android. This also provides a reference for setting up other projects that utalize DeepSea. This provides both debug and release targets for each of the GUI testers, and will build for the four main ABIs. (x86, x86_64, armeabi-v7a, and arm64-v8a) Release builds will use debug signing so they can be deployed without setting up a signature intended for general distribution.

In order to build the project, you must have run `update.sh -l android-all` to download the pre-built Android library dependnencies. You will also need the following packages installed in Android Studio:

* SDK Platform API 18 (Android 4.3 Jelly Bean)
* SDK Tools:
	* CMake
	* LLDB
	* NDK

# Key components

These are the key components for setting up the project, assuming you start with an empty Android Studio project. (i.e. new project without enabling any options or creating any pre-defined activity)

## App build.gradle

The `build.gradle` file in the app directory sets up how to build the application. This has a few pieces that need to be added:

* Under the `android -> defaultConfig` section, an `externalNativeBuild` block must define the ABIs to build for. For example:

	externalNativeBuild {
		ndk {
			abiFilters 'x86', 'x86_64', 'armeabi-v7a', 'arm64-v8a'
		}
	}

* Under the `android` section, a separate `externalNativeBuild` block must give the path to the root `CMakeLists.txt file`. For example:

	externalNativeBuild {
		cmake {
			path "${projectDir}/../../CMakeLists.txt"
		}
	}

* Under the `android` section, copy steps need to be added for the SDL libraries for the separate ABIs. While most dependencies are statically linked, since SDL is loaded and used directly by the Android activity, it must be a separate shared library. The following commands are used to copy from the pre-built packages downloaded by `update.sh`. `prebuild.dependsOn(target)` lines at the root level add a dependency on `target`. (e.g. `preBuild.dependsOn(copyArm32Libraries)`)

		task copyArm32Libraries(type: Copy) {
			from ("${projectDir}/../../dependencies/libs/android-armeabi-v7a/lib") {
				include "*.so"
			}
			into "${projectDir}/src/main/jniLibs/armeabi-v7a"
		}
		task copyArm64Libraries(type: Copy) {
			from ("${projectDir}/../../dependencies/libs/android-arm64-v8a/lib") {
				include "*.so"
			}
			into "${projectDir}/src/main/jniLibs/arm64-v8a"
		}
		task copyX86Libraries(type: Copy) {
			from ("${projectDir}/../../dependencies/libs/android-x86/lib") {
				include "*.so"
			}
			into "${projectDir}/src/main/jniLibs/x86"
		}
		task copyX86_64Libraries(type: Copy) {
			from ("${projectDir}/../../dependencies/libs/android-x86_64/lib") {
				include "*.so"
			}
			into "${projectDir}/src/main/jniLibs/x86_64"
		}

* Yet another `externalNativeBuild` block is required to link the CMake target to the APK bundle. Separate flavors are used to generate multiple APKs in this project. For example, this excerpt under the `android` section to set up the TestCube tester:

		flavorDimensions "tester"
		productFlavors {
			TestCube {
				externalNativeBuild {
					cmake {
						targets "deepsea_test_cube_app"
					}
				}
				dimension "tester"

				applicationIdSuffix ".testcube"
				versionNameSuffix "-TestCube"
				resValue "string", "app_name", "TestCube"
			}
		}

	* When setting `app_name` like in the above example, make sure to delete `app_name` from `app/src/main/res/values/strings.xml`.
* The following dependencies need to be added under the `dependencies` section:
	* implementation 'com.android.support.constraint:constraint-layout:1.1.3'
	
## Project files

The following project files need to be added or modified:
	
* A properly configured `AndroidManifest.xml` file must be placed under `app/src/main`. The one included in this project can be used as a template.
* Java source must be added for the project under `app/src/main/java`. The activity can be found under `app/src/main/java/com/akb825/deepsea/DeepSeaActivity.java`. The SDL activity source code is also present under `app/src/main/java/org/libsdl/app`. If you download the SDL source code, you can also find it under the `android-project` directory.

	> **Note:** To avoid issues when showing notifications, the implementation of `SDLActivity.onWindowFocusChanged()` should be deleted. This fixes two issues:

	> 1. Allows animations to continue when notifications are up.
	> 2. Avoids crashes in Vulkan due to destroying and re-creating the surface on the same window.
