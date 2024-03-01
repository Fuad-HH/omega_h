cmake -S . -B build \
	-DCMAKE_CXX_COMPILER=`which mpicxx` \
    -DCMAKE_C_COMPILER=`which mpicc` \
    -DCMAKE_INSTALL_PREFIX=/lore/hasanm4/Omega_H_OMP/ \
    -DGmsh_INCLUDE_DIRS=/lore/hasanm4/Gmsh/include/ \
    -DKokkos_DIR=/lore/hasanm4/Kokkos/kokkosInstall/lib64/cmake/Kokkos \
    -DCMAKE_BUILD_TYPE=Debug \
    -DOmega_h_DBG=OFF

cd build
make -j4
make install -j4
