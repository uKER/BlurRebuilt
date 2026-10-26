#pragma once

class TextureData
{
public:
	float3			GetAverageColour() { return m_vAverageRGB; }
	U32				GetTexel(int x, int y) { assert(x>=0 && x<m_cX && y>=0 && y<m_cY); return m_pData[x+m_cX*y]; }

	std::string		m_name;
	float3			m_vAverageRGB;
	int				m_cX, m_cY;
	U32				*m_pData;

	friend ost::Engine& operator << (ost::Engine& ar, TextureData &t)
	{
		ar << t.m_name << t.m_vAverageRGB << t.m_cX << t.m_cY;
		ar.writeBinary(t.m_pData, t.m_cX * t.m_cY * sizeof(U32));
		return ar;
	}
	friend ost::Engine& operator >> (ost::Engine& ar, TextureData &t)
	{
		ar >> t.m_name >> t.m_vAverageRGB >> t.m_cX >> t.m_cY;
		t.m_pData = new U32[t.m_cX * t.m_cY];
		ar.readBinary(t.m_pData, t.m_cX * t.m_cY * sizeof(U32));
		return ar;
	}
};

// this class pre-processes textures. It stores their average colour, so that we can access those quickly
// when calculating bounce-lighting
class TexturePreprocessor
{
public:
								TexturePreprocessor();
								~TexturePreprocessor();	

	bool						LoadDatabase();
	bool						SaveDatabase();
	TextureData					*LookupTexture(std::string &name);
	TextureData					*GetTextureData(const Aftk::C_Polygon *pPoly);
	bool						AddTexture(const std::string &name);

	static const char			*m_szDatabaseFilename;
	static const int			m_iDatabaseVersion;
	static const int			m_iMaxResolution;

	Ut::Vector<TextureData>		m_PreprocessData;
	std::map<std::string, int>	m_NameMap;					// map names in the database to their database entry index

	std::map<std::string, int>	m_NotFoundNameMap;			// map of names that we failed to load

	bool						m_bGotLatestDatabase;
	bool						m_bDatabaseIsCheckedOut;
};

