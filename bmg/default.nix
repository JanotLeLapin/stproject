{ gcc
, stdenv
}: stdenv.mkDerivation {
  name = "stproject-bmg";
  version = "0.1";

  src = ./.;

  nativeBuildInputs = [ gcc ];

  buildPhase = ''
    mkdir -p $out/bin
    gcc main.c encode.c decode.c -o $out/bin/stproject-bmg
  '';
}
