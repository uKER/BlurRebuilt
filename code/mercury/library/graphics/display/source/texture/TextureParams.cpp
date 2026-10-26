#include "stdafx.h"
#include "Display/Texture/TextureParams.h"

namespace Display
{

C_TextureParams::C_TextureParams() :
	m_Name(""),
	m_Width(0),
	m_Height(0),
	m_Depth(0),
	m_Levels(0),
	m_Flags(0),
	m_Format(E_Format_Unknown),
	m_MipHead(0),
	m_MipTail(0),
	m_MipSize(0),
	m_MipOffset(0),
	m_pMipData(NULL),
	m_MipCopy(false)
{
	#ifdef PLATFORM_PS3
	m_TransferMode = CELL_GCM_TRANSFER_MAIN_TO_LOCAL;
	#endif
}

C_TextureParams::~C_TextureParams()
{
}

}; // namespace Display
