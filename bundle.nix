{ stproject-extract
, ndstool
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-bundle";
  version = "0.1";

  src = stproject-extract;

  nativeBuildInputs = [ ndstool ];

  buildPhase = ''
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
