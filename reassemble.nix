{ stproject-disassemble
, stproject-bmg
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-reassemble";
  version = "0.1";

  src = stproject-disassemble;

  nativeBuildInputs = [ stproject-bmg ];

  buildPhase = ''
    mkdir -p $out/data/French/Message
    stproject-bmg encode < data/French/Message/battle_parent > $out/data/French/Message/battle_parent.bmg
  '';
}
