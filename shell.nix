with import <nixpkgs> { config.android_sdk.accept_license = true; };

let
  pythonEnv = python3.withPackages (ps: [ ps.absl-py ps.protobuf ps.attrs ]);
  xcodeenv = import ./nix/xcodeenv.nix { inherit stdenv; } { };
  androidDevEnv = import ./nix/android-dev-env.nix { };
in mkShell (androidDevEnv.envVars // rec {
  packages = [
    pythonEnv
    cmake
    ccache
    protobuf
    git
    ninja

    nixfmt
    which
    vim
    neovim
    emacs
  ] ++ (if stdenv.isDarwin then [ xcodeenv cocoapods ] else [ ]);
  buildInputs = [ jdk11 androidDevEnv.android.androidsdk ];
  nativeBuildInputs = [ boringssl ];
  # TODO(wuandy): this is probably a setup error on my machine, not required for other
  # people. We should fix the issue properly and remove this hack.
  shellHook = ''
    unset CMAKE_FRAMEWORK_PATH
  '';
})
