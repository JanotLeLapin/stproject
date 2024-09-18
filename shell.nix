{ nil
, clang-tools  
, mkShell
}: mkShell {
  buildInputs = [ nil clang-tools ];
}
