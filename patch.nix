{ stproject-disassemble
, bsdiff
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-patch";
  version = "0.1";

  src = ./patch;

  nativeBuildInputs = [ bsdiff ];

  buildPhase = ''
    mkdir -p $out/data/French/Message
    patch -o $out/data/French/Message/battle_parent ${stproject-disassemble}/data/French/Message/battle_parent data/French/Message/battle_parent.patch
  '';
}
