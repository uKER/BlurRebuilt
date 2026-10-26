#ifndef __ALPHATEXTUREINFO_H__
#define __ALPHATEXTUREINFO_H__

// contains information about an alpha texture that has been loaded
class AlphaTextureInfo
{
public:
	int			m_cX, m_cY;

	U8			*m_pData;

	AlphaTextureInfo() { m_pData = NULL; }
	~AlphaTextureInfo() { if (m_pData) delete [] m_pData; }
};

// read
template<typename Archive>
Archive& operator >> (Archive& ar, AlphaTextureInfo &ati)
{
	int i,c;

	ar >> ati.m_cX >> ati.m_cY;
	c = ati.m_cX * ati.m_cY;
	ati.m_pData = new U8[c];
	for (i=0; i<c; i++) ar >> ati.m_pData[i];

	return ar;
}

// write
template<typename Archive>
Archive& operator << (Archive& ar, AlphaTextureInfo &ati)
{
	ar << ati.m_cX << ati.m_cY;
	for (int i=0; i<ati.m_cX * ati.m_cY; i++) ar << ati.m_pData[i];
	
	return ar;
}

#endif
