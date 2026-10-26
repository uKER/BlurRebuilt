  SCE CONFIDENTIAL                                      
  PLAYSTATION(R)3 Programmer Tool Runtime Library 270.001
  Copyright (C) 2008 Sony Computer Entertainment Inc.   
  All Rights Reserved.                                  


<Description of the Sample>
        This sample uses the make_package_npdrm command to create the
        NPDRM package.

        When you create pkg file in this sample, please double-click package.bat
        or, type the following in command line:
        
        > package.bat

<Files>
        package.conf:         Configuration file to be used by the 
                              make_package_npdrm command
        Makefile:             Description of the sample data and the
                              procedure for creating the NPDRM package
        package.bat:          Windows bat file for creating the NPDRM package
        
<Created Files>
        USRDIR/:              Directory to store the sample data
        USRDIR/sample.png:    Symbolic link to the sample raw data
        USRDIR/sample.edat:   The above file converted to the NPDRM edata
                              format
        IV0002-NPXS00021_00-SAMPLE0000000001.pkg:
                              File under USDIR/ that has been packaged 
                              as a NPDRM package

        *Although this sample stores the same file in the raw data format
         and the NPDRM edata format, this is not necessary in actual use.

