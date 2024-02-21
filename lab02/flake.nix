{
  description = "C++ web server";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";
    flake-utils.url = "github:numtide/flake-utils";
    khaser.url = "git+ssh://git@109.124.253.149/~git/nixos-config?ref=master";
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

          autocmd filetype cpp map <silent> <F1> :!bear -- make build -j8 <CR>
          autocmd filetype cpp map <silent> <F2> :!make run <CR>
          let &path.="src,${pkgs.glibc.dev}/include"
          let g:ycm_clangd_binary_path = '${pkgs.clang-tools}/bin/clangd'
        '';
      });
    in {
      devShell = pkgs.mkShell {
        name = "cpp";

        nativeBuildInputs = with pkgs; [
          configured-vim
          gcc # compiler
          bear # compiler flags provider for ycm
          clang-tools
          (postman.overrideAttrs (finalAttrs: pa: {
            src = fetchurl {
              url = "https://dl.pstmn.io/download/version/10.23.5/linux_64";
              sha256 = "sha256-NH5bfz74/WIXbNdYs6Hoh/FF54v2+b4Ci5T7Y095Akw=";
              name = "${pa.pname}-${pa.version}.tar.gz";
            };
            buildInputs = pa.buildInputs ++ [ libsecret ];
            postFixup = ''
              pushd $out/share/postman
              patchelf --set-interpreter "$(cat $NIX_CC/nix-support/dynamic-linker)" postman
              patchelf --set-interpreter "$(cat $NIX_CC/nix-support/dynamic-linker)" chrome_crashpad_handler
              for file in $(find . -type f \( -name \*.node -o -name postman -o -name \*.so\* \) ); do
                ORIGIN=$(patchelf --print-rpath $file); \
                patchelf --set-rpath "${lib.makeLibraryPath finalAttrs.buildInputs}:$ORIGIN" $file
              done
              popd
              wrapProgram $out/bin/postman --set PATH ${lib.makeBinPath [ openssl ]}
            '';
          }))
        ];
      };
    });
}

