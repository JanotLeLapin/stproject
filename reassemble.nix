{ stproject-patch
, stproject-bmg
, stdenv
}: let
  encode = file: "stproject-bmg encode $out/data/French/Message/${file}.bmg < data/French/Message/${file}";
in stdenv.mkDerivation {
  name = "stproject-reassemble";
  version = "0.1";

  src = stproject-patch;

  nativeBuildInputs = [ stproject-bmg ];

  buildPhase = ''
    mkdir -p $out/data/French/Message

    ${encode "battle_parent"}
  '';
}
