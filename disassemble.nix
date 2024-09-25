{ stproject-extract
, stproject-tools
, stdenv
}: let
  decode = file: "stproject-tools bmg decode data/French/Message/${file}.bmg $out/data/French/Message/${file}";
in stdenv.mkDerivation {
  name = "stproject-disassemble";
  version = "0.1";

  src = stproject-extract;

  nativeBuildInputs = [ stproject-tools ];

  buildPhase = ''
    mkdir -p $out/data/French/Message

    ${decode "castle_town"}
  '';
}
