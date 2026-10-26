#include "StdAfx.h"
#include "Main.h"
#include "Display/Device/D3D9/PCD3D9CapsBuilder.h"

int _tmain(int argc, _TCHAR* argv[])
{
	int errorCode = 1;

	IDirect3D9* pd3dObject = Direct3DCreate9(D3D_SDK_VERSION);
	if(pd3dObject)
	{
		D3DCAPS9 d3dCaps;
		HRESULT hr = pd3dObject->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);
		if(SUCCEEDED(hr))
		{
			if(Display::D3D9::D3DBuildCaps(d3dCaps, "Object"))
			{
				if(Display::D3D9::D3DBuildCaps(d3dCaps, "Device"))
				{
					errorCode = 0;
				}
				else
				{
					Display::D3D9::DBError("Failed to build device caps from file\n");
				}
			}
			else
			{
				Display::D3D9::DBError("Failed to build object caps from file\n");
			}
		}
		else
		{
			Display::D3D9::DBError("Failed to get the d3d caps from the d3d object\n");
		}


		ULONG refCount = pd3dObject->Release();
		assert(refCount == 0);
	}
	else
	{
		Display::D3D9::DBError("Failed to create the d3d object\n");
	}

	Display::D3D9::DBPrint("Press any key to continue\n");

	char c = _getch();

	return errorCode;
}
