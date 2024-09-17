{ stproject-extract
, stproject-patch
, ndstool
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-bundle";
  version = "0.1";

  src = stproject-extract;

  nativeBuildInputs = [ ndstool ];

  buildPhase = ''
    cp ${stproject-patch}/data/French/Message/select.bmg data/French/Message/select.bmg

    mkdir -p $out
    ndstool -c $out/rom.nds \
      -9 arm9.bin \
      -7 arm7.bin \
      -y9 overlay9.bin \
      -y7 overlay7.bin \
      -d data/ \
      -y overlay/ \
      -t banner.bin \
      -h header.bin
  '';
}
