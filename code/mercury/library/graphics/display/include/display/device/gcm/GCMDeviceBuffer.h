#ifndef DISPLAY_GCMDEVICEBUFFER_H
#define DISPLAY_GCMDEVICEBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"
#include "Display/Device/GCM/GCMDeviceParams.h"

namespace NewDisplay
{
namespace GCM
{

class C_GCMBuffer
{
public:

	C_GCMBuffer();
	~C_GCMBuffer();

	const bool IsCreated() const;

	bool Create(const C_DeviceParams& deviceParams);

	void Set();
	void Wait();
	void Flip();

private:

	bool CreateColourSurface();
	bool CreateDepthSurface();
	bool CreateResc();

public:

	U32				m_Width;
	U32				m_Height;
	E_Format		m_Format;
	U32				m_ColourPitch;
	U32				m_DepthPitch;
	U32				m_ColourOffset;
	U32				m_DepthOffset;
	CellGcmSurface	m_Surface;

private:

	//C_Resource* m_pColourResource;
	//C_Resource* m_pDepthResource;
	//C_Resource* m_pRescColourResource;
	//C_Resource* m_pRescVertexResource;
	//C_Resource*	m_pRescShaderResource;
	bool		m_Created;
};

}; // namespace GCM
}; // namespace NewDisplay

#endif // DISPLAY_GCMDEVICEBUFFER_H
