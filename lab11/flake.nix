{
  description = "Pure python env";

  inputs = {
    nixpkgs.follows = "khaser/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    khaser.url = "github:khaser/nix-vim-config";
  };

  outputs = { self, nixpkgs, flake-utils, khaser }:
    flake-utils.lib.eachDefaultSystem ( system:
    let
      pkgs = import nixpkgs { inherit system; config.allowUnfree = true; };
      configured-vim = khaser.lib.vim;
    in {
      devShell = pkgs.stdenv.mkDerivation {
        name = "py";
        nativeBuildInputs = with pkgs; [ configured-vim python311 ];
      };
    });
}
