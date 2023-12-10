{
  inputs = {
    nixpkgs.url = "github:carlossless/nixpkgs/fixups"; # latest patches to get astyle 4.3.10 & sdcc 4.3.0 working
    utils.url = "github:numtide/flake-utils";
    sinowealth-kb-tool.url = "github:carlossless/sinowealth-kb-tool/corrected-isp-behaviour";
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
            astyle
            sinowealth-kb-tool.packages."${system}".default
            uhubctl
          ];
        };
      }
    );
}
