mkdir USRDIR
mkdir "USRDIR/DLC4"
make_edata_npdrm "DLC Pack Walmart ToC.bin" "USRDIR/DLC4/ToC.edat"
copy "DLC Pack Walmart.bin" "USRDIR/DLC4/DLCPackWalmart.bin"
make_package_npdrm package.conf .

pause