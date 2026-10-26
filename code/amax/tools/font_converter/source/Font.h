#ifndef FONT_H
#define FONT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "btl/Containers/vector.h"
#include "attk/attk.h"


namespace Font
{

class C_BSPPageNode;

#define DEF_TPAGE_SIZE 512

enum E_VersionMajor
{
    E_VersionMajor_Initial = 0,
    E_VersionMajor_Current = 1
};

// comment out + replace minor enum when major version changes

enum E_VersionMinor
{
    E_VersionMinor_Initial = 0,
    E_VersionMinor_Current = E_VersionMinor_Initial
};


enum E_Format
{
	E_Format_Invalid = 0,
	E_Format_PC,
	E_Format_XB2,
	E_Format_PS3,
    E_Format_Count
};


struct Rect
{ 
	U32 m_UMin;
	U32 m_VMax;
	U32 m_UMax;
	U32 m_VMin;
};

struct RectF
{ 
	F32 m_UMin;
	F32 m_VMax;
	F32 m_UMax;
	F32 m_VMin;
};

class C_CharInfo
{
public:
	C_CharInfo()
	{
		m_Blank = 1;

		m_rectSrc.m_UMin = 0;
		m_rectSrc.m_VMax = 0;
		m_rectSrc.m_UMax = 0;
		m_rectSrc.m_VMin = 0;
	}
	~C_CharInfo(){}

	template < typename T_Stream >
    void Deserialise( T_Stream& ar )
	{
		ar >> m_Output.m_Unicode;
		ar >> m_Output.m_TexturePage;
		ar >> m_Output.m_Offset;
		ar >> m_Output.m_Height;
		ar >> m_Output.m_Width;
		ar >> m_Output.m_RectOut.m_UMin;
		ar >> m_Output.m_RectOut.m_VMax;
		ar >> m_Output.m_RectOut.m_UMax;
		ar >> m_Output.m_RectOut.m_VMin;
	}

	template < typename T_Stream >
    void Serialise( T_Stream& ar ) const
	{
		ar << m_Output.m_Unicode;
		ar << m_Output.m_TexturePage;
		ar << m_Output.m_Offset;
		ar << m_Output.m_Height;
		ar << m_Output.m_Width;
		ar << m_Output.m_RectOut.m_UMin;
		ar << m_Output.m_RectOut.m_VMax;
		ar << m_Output.m_RectOut.m_UMax;
		ar << m_Output.m_RectOut.m_VMin;
	}

	void	AddBorder(U8 borderSize);

	void	CheckExtents(U32 x, U32 y);

	void	SizeOutputRect();


public:
	
	U8		m_Blank;

	U16		m_SourcePageTexture;		///< Which Texture Page the Character is on

	U32		m_UOffsetSrc;
	U32		m_VOffsetSrc;
	Rect	m_rectSrc;				///< Coord Extents of the Character Rectangle in the Source Texture
	Rect	m_rectDst;				///< Coord Extents of the Character Rectangle in the Destination Texture

	struct
	{
		U16		m_Unicode;			///< Unicode of the Character
		U16		m_TexturePage;		///< Which Texture Page the Character is on
		F32		m_Offset;			///< (0.0 -> 1.0) - Dist from Top of Row down to Top of Character
		F32		m_Height;			///< (0.0 -> 1.0) - Dist from Base of Char up to Top of Character
		F32		m_Width;			///< (0.0 -> 1.0) - Relative Width of Character, 1.0 = Widest
		RectF	m_RectOut;			///< Output Coord Extents of the Character Rectangle

	} m_Output;
};

class C_Font
{
	//Source Page Textures
	Attk::C_Image*	 m_pSourcePageTexture[256];
	//Index of each Source Page Texture
	U16					m_PageIndex[256];
	//Number of source pages
	U16					m_NumSourcePages;

	Btl::Vector<C_CharInfo> m_CharInfoList;
	
	//Border Size
	U8					m_BorderSize;
	//Font sizes
	U16					m_MinU;
	U16					m_MaxU;
	U16					m_MinV;
	U16					m_MaxV;
		
	//List of BSP trees of texture Pages
	Btl::Vector<C_BSPPageNode*>	m_pPageTree;		

	//New Texture Pages
	Btl::Vector<Attk::C_Image*>	m_pNewTexturePage;


	void AddBSPPage();

	void CalculateFontSize();
	
	bool ArrangeCharacters();

	void FillNewTexturePages();

	Attk::C_Image* ProcessImage(Attk::C_Image* pSrcImage);
	
public:

	C_Font();
	~C_Font();

	void ReadFontTexture(const Util::C_String& textureFileName);

	void ReadCharList(const Util::C_String& textureFileName);

	void Process();

	void Write(Tools::E_Platform platform, Sys::I_File& file);
};

} //Font
#endif
