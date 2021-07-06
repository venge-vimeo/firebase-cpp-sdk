rec {
    pkgs = import (builtins.fetchTarball {
        name = "nixpkgs-20.09";
        url = "https://github.com/NixOS/nixpkgs/archive/refs/tags/21.05.tar.gz";
    }) { config.android_sdk.accept_license = true; };
}
