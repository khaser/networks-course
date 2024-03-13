{
  description = "C++ beast http multithreaded server and client couple";

  inputs = {
    nixpkgs.follows = "khaser/nixpkgs";
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

          autocmd filetype cpp map <silent> <F1> :!cmake -B build && cmake --build build <CR>
          autocmd filetype cpp map <silent> <F2> :!./build/server/server 6363 3 <CR>
          autocmd filetype cpp map <silent> <F3> :!./build/client/client 127.0.0.1 6363 kekw <CR>
          let &path.="src,${pkgs.glibc.dev}/include"
          let g:ycm_clangd_binary_path = '${pkgs.clang-tools}/bin/clangd'
        '';
      });
      toolchain = with pkgs; [ gcc cmake ];
      libs = with pkgs; [ boost ];
      pkgFabric = (name: pkgs.stdenv.mkDerivation {
          inherit system;
          name = "cpp-beast-${name}";
          nativeBuildInputs = toolchain;
          buildInputs = libs;

          src = ./${name};
          enableParallelBuilding = true;
          doCheck = false;
          installPhase = ''
            mkdir -p $out/bin
            cp ./${name} $out/bin/
          '';
        });
      appFabric = (name: {
        type = "app";
        program = "${self.packages.${system}.${name}}/bin/${name}";
      });
    in {

      packages."server" = pkgFabric "server";
      apps."server" = appFabric "server";

      packages."client" = pkgFabric "client";
      apps."client" = appFabric "client";

      devShell = pkgs.mkShell {
        name = "cpp";
        nativeBuildInputs = [configured-vim pkgs.gdb] ++ toolchain ++ libs;
      };

    });
}
