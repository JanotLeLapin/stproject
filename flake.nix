{
  description = "Toy NDS rom patch";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs = {
    nixpkgs,
    ...
  }:
  let
    eachSystem = fn: nixpkgs.lib.genAttrs [
      "x86_64-linux"
      "aarch64-linux"
    ] (system: let
      pkgs = (import nixpkgs { inherit system ; });
    in (fn { inherit system pkgs; }));
    originalRom = ./rom.nds;
  in {
    packages = eachSystem ({ pkgs, ... }: rec {
      extract = pkgs.callPackage ./extract.nix { stproject-rom = originalRom; };
      patch = pkgs.callPackage ./patch.nix { stproject-extract = extract; };
      bundle = pkgs.callPackage ./bundle.nix {
        stproject-extract = extract;
        stproject-patch = patch;
      };
    });
  };
}
