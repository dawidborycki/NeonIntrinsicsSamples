# NeonIntrinsicsSamples

## Overview

An Android application that demonstrates scalar C++ and Arm NEON implementations of thresholding and one-dimensional convolution. It generates a noisy sine wave, displays the input and processed signals, and lets the user switch between scalar and SIMD processing while viewing elapsed time.

> This repository is retained as a historical technical example. It reflects the development environment and APIs used when the accompanying material was originally published in 2020.

## Technical focus

- Native C++ and Arm NEON SIMD intrinsics
- Signed 8-bit signal data, thresholding, and a 16-element convolution kernel
- Android NDK, CMake, and Gradle integration
- Java-to-native calls through JNI
- Signal visualization with GraphView

## Workflow

`Generate signal → select scalar or NEON processing → threshold / convolve → return byte array through JNI → plot signal and timing`

[`native-lib.cpp`](app/src/main/cpp/native-lib.cpp) contains the signal generation and processing routines. [`MainActivity.java`](app/src/main/java/com/example/neonintrinsicssamples/MainActivity.java) handles the controls and plots the input, truncated, and convolved signals with GraphView.

The displayed timings illustrate the comparison on the chosen device and build. They are not a reproducible benchmark or a guarantee that SIMD is faster for every workload.

## Known limitations

The convolution routines read outside the input buffer at the signal boundaries; they need an explicit boundary policy before reuse. The NEON convolution temporary buffer also uses scalar `delete` for an array allocation. The example has fixed-size inputs and Android project-template tests, rather than numerical validation tests, so treat its output and timing as educational material until those issues are corrected and tested.

## Getting started

### Historical toolchain

The checked-in project uses Android Gradle Plugin 3.6.2, Gradle 5.6.4, Android SDK 29, Build Tools 29.0.3, CMake 3.10.2, and GraphView 4.2.2. The IDE configuration references JDK 8. The Android NDK version is not pinned.

1. Clone the repository and open its root directory in a compatible Android Studio environment.
2. Install Android SDK 29, Build Tools 29.0.3, the Android NDK, and CMake 3.10.2 through the SDK Manager.
3. For the NEON example, replace the existing ABI filter in [`app/build.gradle`](app/build.gradle) with:

   ```groovy
   ndk.abiFilters 'armeabi-v7a', 'arm64-v8a'
   ```

   The historical configuration also lists `x86`. With `HAVE_NEON` set to `1`, the native code requires an Arm target. The source comment describes setting `HAVE_NEON` to `0` for scalar-only experimentation on x86.
4. Keep the existing CMake argument `-DANDROID_ARM_NEON=ON`, synchronize Gradle, and run the `app` configuration on an Android device with NEON support.
5. Select **Generate Signal**, choose the **Use Neon?** setting, and select **Truncate** or **Convolution** to view the corresponding output.

After configuring the historical toolchain and Arm ABIs, a command-line debug build is:

```sh
bash gradlew :app:assembleDebug
```

On Windows, use `gradlew.bat :app:assembleDebug`. The APK is generated under `app/build/outputs/apk/debug/`.

Current Android Studio releases may require additional setup for these older build tools and dependencies. This documentation update does not migrate the project or certify a build with a current Android toolchain.

## Related material

- [Arm developer guide: Neon Intrinsics on Android — Thresholding and Convolution](https://developer.arm.com/-/media/Arm%20Developer%20Community/PDF/Android%20Dev%20Guides/Neon%20Intrinsics%20on%20Android-%20How%20to%20Truncate%20Thresholding%20and%20Convolution%20of%20a%201D%20Signal%202.0.pdf?revision=edfbe7c8-ccdd-4bfc-9e6a-3df506df3a5e) — the public guide links to this repository and describes the signal-processing example.
- [NeonIntrinsics-Android](https://github.com/dawidborycki/NeonIntrinsics-Android) — the related native Android setup and dot-product example.

## License

See [LICENSE](LICENSE) for the GPL-3.0 license.
