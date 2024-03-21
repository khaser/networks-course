{
  description = "C++ networks course practice(fuck python!)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    khaser.url = "github:khaser/nix-vim-config";
  };

  outputs = { self, nixpkgs, flake-utils, khaser }:
    flake-utils.lib.eachDefaultSystem ( system:
    let
      pkgs = import nixpkgs { inherit system; config.allowUnfree = true; };
      configured-vim = (khaser.lib.vim.override {
        extraPlugins = with pkgs.vimPlugins; [
          vim-cpp-enhanced-highlight
          YouCompleteMe
        ];
        extraRC = ''
          let g:cpp_class_scope_highlight = 1
          let g:cpp_member_variable_highlight = 1
          let g:cpp_class_decl_highlight = 1
          let g:cpp_posix_standard = 1
          let g:cpp_experimental_simple_template_highlight = 0
          let g:cpp_concepts_highlight = 1

          let &path.="src,${pkgs.glibc.dev}/include"
          let g:ycm_clangd_binary_path = '${pkgs.clang-tools}/bin/clangd'
        '';
      });
      toolchain = with pkgs; [ gcc cmake ];
      stdEnv = pkgs.stdenv.mkDerivation;
      libs = with pkgs; [ ];
      pkgFabric = (name: stdEnv {
          inherit system;
          name = "cpp-beast-${name}";
          nativeBuildInputs = toolchain;
          buildInputs = libs;

          src = ./${name};
          enableParallelBuilding = true;
          doCheck = false;
          installPhase = ''
            mkdir -p $out/bin
            cp ./* $out/bin/
          '';
        });
      appFabric = (name: {
        type = "app";
        program = "${self.packages.${system}.${name}}/bin/${name}";
      });
      vectorizeFabric = (fabric: list:
        with builtins; listToAttrs (map (x: { name=x; value=fabric x; }) list
      ));
      cppPackages = ["broadcasted_clock"];
    in {

      packages = vectorizeFabric pkgFabric cppPackages;
      apps = vectorizeFabric appFabric cppPackages;

      devShell = stdEnv {
        name = "cpp";
        nativeBuildInputs = [configured-vim pkgs.gdb] ++ toolchain ++ libs;
      };

    });
}
