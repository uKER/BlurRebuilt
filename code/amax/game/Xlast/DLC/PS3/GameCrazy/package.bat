mkdir USRDIR
mkdir "USRDIR/DLC2"
make_edata_npdrm "DLC Pack GameCrazy ToC.bin" "USRDIR/DLC2/ToC.edat"
copy "DLC Pack GameCrazy.bin" "USRDIR/DLC2/DLCPackGameCrazy.bin"
make_package_npdrm package.conf .

pause