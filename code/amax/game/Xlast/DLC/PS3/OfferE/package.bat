mkdir USRDIR
mkdir "USRDIR/DLC2"
make_edata_npdrm "DLC Pack Offer E ToC.bin" "USRDIR/DLC4/ToC.edat"
copy "DLC Pack OfferE.bin" "USRDIR/DLC4/DLCPackOfferE.bin"
make_package_npdrm package.conf .

pause