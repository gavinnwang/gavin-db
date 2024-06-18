{
  description = "GavinDB";

  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }: 
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs { inherit system; };
    in
    {
      packages = {
        default = pkgs.stdenv.mkDerivation {
          name = "GavinDB";
          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
          ];

          buildInputs = with pkgs; [
            fmt
            gtest
          ];
        };
      };

      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          clang
          cmake
          fmt
          gtest
        ];
      };
    });
}
