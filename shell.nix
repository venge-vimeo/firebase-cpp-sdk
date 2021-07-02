with import <nixpkgs> { config.android_sdk.accept_license = true; };

let
  pythonEnv = python3.withPackages (ps: [ ps.absl-py ps.protobuf ps.attrs ]);
  xcodeenv = import ./nix/xcodeenv.nix { inherit stdenv; } { };
  # android = androidenv.androidPkgs_9_0;
    android = androidenv.composeAndroidPackages {
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
      ndkVersions = ["17.2.4988734"];
      useGoogleAPIs = false;
      useGoogleTVAddOns = false;
      includeExtras = [
        "extras;google;gcm"
      ];
    };
in mkShell rec {
  ANDROID_SDK_ROOT = "${android.androidsdk}/libexec/android-sdk";
  ANDROID_NDK_ROOT = "${ANDROID_SDK_ROOT}/ndk-bundle";
  ANDROID_HOME = "${ANDROID_SDK_ROOT}";
  ANDROID_NDK_HOME = "${ANDROID_NDK_ROOT}";
  # GRADLE_OPTS = "-Dorg.gradle.project.android.aapt2FromMavenOverride=${android.androidsdk}/libexec/android-sdk/build-tools/28.0.3/aapt2";

  packages = [
    pythonEnv
    cmake
    ccache
    protobuf
    xcodeenv
    cocoapods
    git
    ninja
    # android-studio

    nixfmt
    which
    vim
    neovim
    emacs
  ];
  buildInputs = [
    jdk11
    android.androidsdk
  ];
  nativeBuildInputs = [ boringssl ];
  # TODO(wuandy): this is probably a setup error on my machine, not required for other
  # people. We should fix the issue properly and remove this hack.
  shellHook = ''
    unset CMAKE_FRAMEWORK_PATH
  '';
}
