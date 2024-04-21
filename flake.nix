{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable"; # for sdcc 4.4.0
    utils.url = "github:numtide/flake-utils";
    sinowealth-kb-tool.url = "github:carlossless/sinowealth-kb-tool";
  };

  outputs = { self, nixpkgs, utils, sinowealth-kb-tool }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            sdcc
            gnumake
            binutils
            clang-tools # for clang-format
            sinowealth-kb-tool.packages."${system}".default
            uhubctl
            srecord
            meson
            ninja
          ];
        };
      }
    );
}
