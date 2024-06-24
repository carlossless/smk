{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
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
            meson
            ninja

            sinowealth-kb-tool.packages."${system}".default # flashing
            clang-tools # for clang-format

            # useful supplementary tools
            binutils # for objcopy
            uhubctl
          ];
        };
      }
    );
}
