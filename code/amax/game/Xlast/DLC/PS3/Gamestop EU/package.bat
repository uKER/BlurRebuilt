mkdir USRDIR
mkdir "USRDIR/DLC3"
make_edata_npdrm "DLC Pack Gamestop ToC.bin" "USRDIR/DLC3/ToC.edat"
copy "DLC Pack Gamestop.bin" "USRDIR/DLC3/DLCPackGamestop.bin"
make_package_npdrm package.conf .

pause