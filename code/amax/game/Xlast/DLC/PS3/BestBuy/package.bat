mkdir USRDIR
mkdir "USRDIR/DLC1"
make_edata_npdrm "DLC Pack Best Buy ToC.bin" "USRDIR/DLC1/ToC.edat"
copy "DLC Pack Best Buy.bin" "USRDIR/DLC1/DLCPackBestBuy.bin"
make_package_npdrm package.conf .

pause