{ pkgs ? import <nixpkgs> { config.android_sdk.accept_license = true; } }:

rec {
  android = pkgs.androidenv.composeAndroidPackages {
    toolsVersion = "26.1.1";
    buildToolsVersions = [ "28.0.3" ];
    includeEmulator = true;
    emulatorVersion = "30.6.3";
    platformVersions = [ "28" ];
    includeSources = true;
    includeSystemImages = false;
    systemImageTypes = [ "google_apis_playstore" ];
    abiVersions = [ "x86_64" "armeabi-v7a" "arm64-v8a" ];
    includeNDK = true;
    ndkVersions = [ "17.2.4988734" ];
    useGoogleAPIs = false;
    useGoogleTVAddOns = false;
    includeExtras = [ "extras;google;gcm" ];
  };

  envVars = rec {
    ANDROID_SDK_ROOT = "${android.androidsdk}/libexec/android-sdk";
    ANDROID_NDK_ROOT = "${ANDROID_SDK_ROOT}/ndk-bundle";
    ANDROID_HOME = "${ANDROID_SDK_ROOT}";
    ANDROID_NDK_HOME = "${ANDROID_NDK_ROOT}";
  };
}
