#!/bin/bash

echo "Building Project Files"
cd dwtools-ng && python projgen.py
cd ..

echo "Compiling Client Libraries (Debug)"
make -f Makefile.unix DEBUG=1

echo "Building Authentication Server (Debug)"
cd bdLobbyServices/bdAuthenticationService2
./generaterevisionheader
make -f Makefile CFG=debug

echo "Building Authentication Server (Release)"
make -f Makefile CFG=release
