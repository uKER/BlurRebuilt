#pragma once
#include "resource.h"

#include "wrap32_com.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Singlethread-COM-Objekte werden auf der Windows CE-Plattform nicht vollständig unterstützt. Windows Mobile-Plattformen bieten beispielsweise keine vollständige DCOM-Unterstützung. Definieren Sie _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA, um ATL zu zwingen, die Erstellung von Singlethread-COM-Objekten zu unterstützen und die Verwendung eigener Singlethread-COM-Objektimplementierungen zu erlauben. Das Threadmodell in der RGS-Datei wurde auf 'Free' festgelegt, da dies das einzige Threadmodell ist, das auf Windows CE-Plattformen ohne DCOM unterstützt wird."
#endif

void Wrap32DllMain();

class ATL_NO_VTABLE C_Wrap32 :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<C_Wrap32, &CLSID_Wrap32>,
	public IDispatchImpl<IServerWrapper, 
		&IID_IServerWrapper, 
		&LIBID_COMServerLib, 
		/*wMajor =*/ 1, 
		/*wMinor =*/ 0>
{
public:
	C_Wrap32()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SERVERWRAPPER)


BEGIN_COM_MAP(C_Wrap32)
	COM_INTERFACE_ENTRY(IServerWrapper)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	STDMETHOD(XGTileTextureLevel)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGTileSurface)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGGetMipTailLevelOffset)(UINT callingPID, UINT sharedMemoryID, UINT* result);

	STDMETHOD(XGGetMipTailBaseLevel)(UINT callingPID, UINT sharedMemoryID, UINT* result);

	STDMETHOD(XGGetTextureDesc)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGSetTextureHeader)(UINT callingPID, UINT sharedMemoryID, UINT* result);

	STDMETHOD(XGEndianSwapSurface)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGCopySurface)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGTileVolume)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGGetBlockDimensions)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGSetVolumeTextureHeader)(UINT callingPID, UINT sharedMemoryID);

	STDMETHOD(XGSetCubeTextureHeader)(UINT callingPID, UINT sharedMemoryID);
};

OBJECT_ENTRY_AUTO(__uuidof(Wrap32), C_Wrap32)


