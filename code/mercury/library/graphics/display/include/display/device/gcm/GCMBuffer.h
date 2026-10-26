#ifndef DISPLAY_GCMBUFFER_H
#define DISPLAY_GCMBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"
#include "Display/Device/GCM/GCMDeviceParams.h"

namespace NewDisplay
{

class C_Resource;

namespace GCM
{

class C_Buffer
{
public:

	C_Buffer();

	~C_Buffer();

	const bool IsCreated() const;

	bool Create(const C_DeviceParams& deviceParams);


	void Set();

	void Wait();

	void Flip();

private:

	const bool CreateColourSurface();

	const bool CreateDepthSurface();

	const bool CreateResc();

public:

	U32 m_Width;

	U32 m_Height;

	E_TextureFormat m_Format;

	U32 m_ColourPitch;

	U32 m_DepthPitch;

	U32 m_ColourOffset;

	U32 m_DepthOffset;

	CellGcmSurface m_Surface;

private:

	C_Resource* m_pColourResource;

	C_Resource* m_pDepthResource;

	C_Resource* m_pRescColourResource;

	C_Resource* m_pRescVertexResource;

	C_Resource* m_pRescShaderResource;

	bool m_Created;
};

}; // namespace GCM
}; // namespace NewDisplay

#endif // DISPLAY_GCMBUFFER_H
