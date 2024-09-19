{ stproject-extract
, stproject-bmg
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-disassemble";
  version = "0.1";

  src = stproject-extract;

  nativeBuildInputs = [ stproject-bmg ];

  buildPhase = ''
    mkdir -p $out/data/French/Message
    stproject-bmg decode $out/data/French/Message/battle_parent < data/French/Message/battle_parent.bmg
  '';
}
