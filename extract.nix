{ stproject-rom
, ndstool
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-extract";
  version = "0.1";

  src = builtins.filterSource (path: type: false) ./.;

  nativeBuildInputs = [ ndstool ];

  buildPhase = ''
    mkdir -p $out/data
    mkdir -p $out/overlay
    ndstool -x ${stproject-rom} \
      -9 $out/arm9.bin \
      -7 $out/arm7.bin \
      -y9 $out/overlay9.bin \
      -y7 $out/overlay7.bin \
      -d $out/data \
      -y $out/overlay \
      -t $out/banner.bin \
      -h $out/header.bin
  '';
}
