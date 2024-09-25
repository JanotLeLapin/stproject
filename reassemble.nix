{ stproject-patch
, stproject-tools
, stdenv
}: let
  encode = file: "stproject-tools bmg encode data/French/Message/${file} $out/data/French/Message/${file}.bmg";
in stdenv.mkDerivation {
  name = "stproject-reassemble";
  version = "0.1";

  src = stproject-patch;

  nativeBuildInputs = [ stproject-tools ];

  buildPhase = ''
    mkdir -p $out/data/French/Message

    ${encode "castle_town"}
  '';
}
