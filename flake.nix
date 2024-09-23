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
    devShells = eachSystem ({ pkgs, ... }: { default = pkgs.callPackage ./shell.nix {}; });
    packages = eachSystem ({ pkgs, ... }: rec {
      bmg = pkgs.callPackage ./bmg {};
      tools = pkgs.callPackage ./tools {};

      extract = pkgs.callPackage ./extract.nix { stproject-rom = originalRom; };
      disassemble = pkgs.callPackage ./disassemble.nix {
        stproject-extract = extract;
        stproject-bmg = bmg;
      };
      patch = pkgs.callPackage ./patch.nix { stproject-disassemble = disassemble; };
      reassemble = pkgs.callPackage ./reassemble.nix {
        stproject-patch= patch;
        stproject-bmg = bmg;
      };
      bundle = pkgs.callPackage ./bundle.nix {
        stproject-extract = extract;
        stproject-reassemble = reassemble;
      };
    });
  };
}
