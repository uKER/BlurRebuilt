#ifndef DISPLAY_TEXTURE_TEXTURE_PARAMS_H
#define DISPLAY_TEXTURE_TEXTURE_PARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace Display
{

typedef Display2::C_LockedRect C_LockedRect;

typedef Display2::C_ConstLockedRect C_ConstLockedRect;

typedef Display2::C_LockedBox C_LockedBox;

typedef Display2::C_ConstLockedBox C_ConstLockedBox;

class C_TextureParams
{
public:

	C_TextureParams();
	~C_TextureParams();

public:

	Util::C_String	m_Name;
	U32				m_Width;
	U32				m_Height;
	U32				m_Depth;
	U32				m_Levels;
	U32				m_Flags;
	E_Format		m_Format;
	U32				m_MipHead;
	U32				m_MipTail;
	U32				m_MipSize;
	U32				m_MipOffset;
	U8*				m_pMipData;
	bool			m_MipCopy;

	#ifdef PLATFORM_PS3
	U32				m_TransferMode;
	#endif
};

}; // namespace Display

#endif // DISPLAY_TEXTURE_TEXTURE_PARAMS_H
