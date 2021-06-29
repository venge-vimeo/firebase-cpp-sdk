with import <nixpkgs> { };

let
  pythonEnv = python3.withPackages (ps: [ ps.absl-py ps.protobuf ps.attrs ]);
  xcodeenv = import ./nix/xcodeenv.nix { inherit stdenv; } { };
in mkShell {
  packages = [
    pythonEnv
    cmake
    ccache
    protobuf
    xcodeenv
    cocoapods
    git

    nixfmt
    which
    vim
    neovim
    emacs
  ];
  nativeBuildInputs = [ boringssl ];
  # TODO(wuandy): this is probably a setup error on my machine, not required for other
  # people. We should fix the issue properly ana remove this hack.
  shellHook = ''
    unset CMAKE_FRAMEWORK_PATH
  '';
}
