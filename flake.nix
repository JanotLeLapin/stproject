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
      tools = pkgs.callPackage ./tools {};

      extract = pkgs.callPackage ./extract.nix { stproject-rom = originalRom; };
      disassemble = pkgs.callPackage ./disassemble.nix {
        stproject-extract = extract;
        stproject-tools = tools;
      };
      patch = pkgs.callPackage ./patch.nix { stproject-disassemble = disassemble; };
      reassemble = pkgs.callPackage ./reassemble.nix {
        stproject-patch= patch;
        stproject-tools = tools;
      };
      bundle = pkgs.callPackage ./bundle.nix {
        stproject-extract = extract;
        stproject-reassemble = reassemble;
      };
    });
  };
}
