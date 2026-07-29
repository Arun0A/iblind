{
  description = "A customizable screen magnifier for X11";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = nixpkgsFor.${system};
        in
        {
          iblind = pkgs.stdenv.mkDerivation {
            pname = "iblind";
            version = "1.0.0";

            src = ./.;

            nativeBuildInputs = [ pkgs.pkg-config pkgs.gnumake ];
            buildInputs = [ pkgs.libX11 ];

            installPhase = ''
              mkdir -p $out/bin
              cp bin/iblind $out/bin/
            '';
          };
          default = self.packages.${system}.iblind;
        });

      apps = forAllSystems (system: {
        iblind = {
          type = "app";
          program = "${self.packages.${system}.iblind}/bin/iblind";
        };
        default = self.apps.${system}.iblind;
      });

      devShells = forAllSystems (system: {
        default = nixpkgsFor.${system}.mkShell {
          nativeBuildInputs = with nixpkgsFor.${system}; [
            pkg-config
            gnumake
            gcc
          ];
          buildInputs = with nixpkgsFor.${system}; [
            libX11
          ];
        };
      });
    };
}
