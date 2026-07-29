{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    pkg-config
    gnumake
    gcc
  ];

  buildInputs = with pkgs; [
    libX11
  ];
}
