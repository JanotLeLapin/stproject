{ nil
, clang-tools
, valgrind
, xxd
, desmume
, mkShell
}: mkShell {
  buildInputs = [
    nil clang-tools
    valgrind xxd
    desmume
  ];
}
