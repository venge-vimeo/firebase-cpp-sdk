with (import ./nix/pinned-nixpkgs.nix);
with pkgs;

let
  pythonEnv = python3.withPackages (ps: [ ps.absl-py ps.protobuf ps.attrs ]);
  xcodeenv = import ./nix/xcodeenv.nix { inherit stdenv; inherit pkgs; } { };
  androidDevEnv = import ./nix/android-dev-env.nix pkgs ;
in pkgs.mkShell (androidDevEnv.envVars // rec {
  packages = [
    pythonEnv
    cmake
    # This is broken in the pinned version.
    # ccache
    protobuf
    git
    ninja
    pkg-config

    nixfmt
    which
    vim
    neovim
    emacs
  ] ++ (if stdenv.isDarwin then [ xcodeenv cocoapods ] else [ ]);
  buildInputs = [ clang jdk11 androidDevEnv.android.androidsdk ];
  nativeBuildInputs = [ libsecret  boringssl ];
})
