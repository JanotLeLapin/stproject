{ gcc
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-tools";
  version = "0.1";

  src = ./.;

  nativeBuildInputs = [ gcc ];

  buildPhase = ''
    mkdir -p $out/bin
    gcc tools.c util.c bmg.c -o $out/bin/stproject-tools
  '';
}
