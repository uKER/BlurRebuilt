// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provides a global singleton pointer to the Direct3D9 device used on Xenon and Win32 (DirectX)

#ifndef BD_D3D_H
#define BD_D3D_H
#include <bdCore/bdCore.h>

// Include the correct headers for the platform and correctly define BD_D3D_DEV for the current platform.
#ifdef BD_PLATFORM_WIN32
// Win32: Use DirectX9
#	include <d3d9.h>
#	define BD_D3D_DEV LPDIRECT3DDEVICE9
#elif defined BD_PLATFORM_XENON
// Xenon: Use the Xenon version of DirectX 9
#	include <xtl.h>
#	define BD_D3D_DEV LPDIRECT3DDEVICE9
#elif defined BD_PLATFORM_XBOX
// XBox: Use the XBox version of DirectX 8
#	include <xtl.h>
#	define BD_D3D_DEV LPDIRECT3DDEVICE8
#else
#	error This File is only supported on Win32, XBox and Xenon
#endif

/// Wrapper class for the single global direct 3d device
class bdD3DDevWrapper
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Empty constructor.
	inline bdD3DDevWrapper();

	/// Empty destructor.
	inline ~bdD3DDevWrapper();
	
	/// Set the direct 3d device.
	inline void setDevice(BD_D3D_DEV const device);

	/// Get the direct 3d device.
	inline BD_D3D_DEV getDevice();
protected:
	/// Pointer to the direct 3d device.
	BD_D3D_DEV m_device;
};

#include <bdGraphics/bdD3D.inl>

/// Creates a single global instance of the Direct 3D Device
class bdD3D : public bdSingleton<bdD3DDevWrapper>
{
	BD_DECLARE_NEW_AND_DELETE_OPERATORS
	protected:
		bdD3D();
};

#endif // BD_D3D_H
