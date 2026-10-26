call mk_common_pc

cls

echo.

%COMP% /T vs_3_0 /E main /Zpr /Fo %ODIR%/vsMesh.pcvsh vsMesh.fx
%COMP% /T ps_3_0 /E main /Zpr /Fo %ODIR%/psMesh.pcpsh psMesh.fx

echo.

%COPY% %ODIR%\vsMesh.pcvsh %CDIR%
%COPY% %ODIR%\psMesh.pcpsh %CDIR%

echo.

pause