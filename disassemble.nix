{ stproject-extract
, stproject-bmg
, stdenv
}: let
  decode = file: "stproject-bmg decode $out/data/French/Message/${file} < data/French/Message/${file}.bmg";
in stdenv.mkDerivation {
  name = "stproject-disassemble";
  version = "0.1";

  src = stproject-extract;

  nativeBuildInputs = [ stproject-bmg ];

  buildPhase = ''
    mkdir -p $out/data/French/Message

    ${decode "battle_parent"}
  '';
}
