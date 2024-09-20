{ stproject-disassemble
, bsdiff
, stdenv
}: let
  patch = file: dir: "patch -o $out/data/French/Message/${file} ${dir}/data/French/Message/${file} data/French/Message/${file}.patch";
in stdenv.mkDerivation {
  name = "stproject-patch";
  version = "0.1";

  src = ./patch;

  nativeBuildInputs = [ bsdiff ];

  buildPhase = ''
    mkdir -p $out/data/French/Message

    ${patch "battle_parent" stproject-disassemble}
  '';
}
