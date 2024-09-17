{ stproject-extract
, bsdiff
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-patch";
  version = "0.1";

  src = ./patch;

  nativeBuildInputs = [ bsdiff ];

  buildPhase = ''
    mkdir -p $out/data/French/Message
    bspatch ${stproject-extract}/data/French/Message/select.bmg $out/data/French/Message/select.bmg data/French/Message/select.bmg.patch
  '';
}
