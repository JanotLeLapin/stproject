{ stproject-patch
, stproject-bmg
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-reassemble";
  version = "0.1";

  src = stproject-patch;

  nativeBuildInputs = [ stproject-bmg ];

  buildPhase = ''
    mkdir -p $out/data/French/Message
    stproject-bmg encode $out/data/French/Message/battle_parent.bmg < data/French/Message/battle_parent
  '';
}
