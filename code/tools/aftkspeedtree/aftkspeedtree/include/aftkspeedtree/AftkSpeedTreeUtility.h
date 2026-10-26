#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SPEEDTREE_UTILITY_H
#define AFTK_SPEEDTREE_UTILITY_H

namespace XERCES_CPP_NAMESPACE
{
	class DomAttr;
}

namespace AftkSpeedTree
{
/*
class Filename : public std::string
{
public:
        Filename() {}
		Filename(const char* pFilename) : std::string(pFilename) {}
		Filename(std::string strFilename) : std::string(strFilename) {}

        Filename GetExtension(void);
        Filename GetPath(void);
        Filename NoExtension(void);
        Filename NoPath(void);
		Filename Replace(const std::string& find, const std::string& replace);
};

inline Filename Filename::Replace(const std::string& find, const std::string& replace)
{
	std::string str = *this;
	std::string::iterator begin;
	std::string::iterator end;

	while(str.find(find) != std::string::npos && find != replace)
	{
		begin = str.begin() + str.find(find);
		end = begin + find.size();
		str.replace(begin, end, replace);
	}

	return str;
}

inline Filename Filename::GetExtension(void)
{
	std::string strExtension;

    int nLength = (int)length( );
    for (int i = nLength - 1; i > 0 && (*this)[i] != '/'; i--)
        if ((*this)[i] == '.')
        {
			strExtension = std::string(c_str( ) + i + 1);
            break;
        }

    return strExtension;
}

inline Filename Filename::GetPath(void)
{
    char szPath[1024];

    szPath[0] = '\0';
    int nLength = (int)length( );
    for (int i = nLength - 1; i >= 0; i--)
        if ((*this)[i] == '/' ||
            (*this)[i] == '\\')
        {
			int j;
            for (j = 0; j < i + 1; j++)
                szPath[j] = (*this)[j];
            szPath[j] = '\0';
            break;
        }

    return Filename(szPath);
}

inline Filename Filename::NoExtension(void)
{
    char szExtension[1024];

    strcpy_s(szExtension, lengthof(szExtension), c_str( ));
    int nLength = (int)length( );
    for (int i = nLength - 1; i >= 0 && (*this)[i] != '/'; i--)
        if ((*this)[i] == '.')
        {
			int j;
            for (j = 0; j < i; j++)
                szExtension[j] = (*this)[j];
            szExtension[j] = '\0';
            break;
        }

    return szExtension;
}

inline Filename Filename::NoPath(void)
{
    char szPath[1024];

    strcpy_s(szPath, lengthof(szPath), c_str( ));
    int nLength = (int)length( );
    for (int i = nLength - 1; i >= 0; i--)
        if ((*this)[i] == '/' || (*this)[i] == '\\')
        {
			int j,k;
            for (j = i + 1, k = 0; j < nLength; j++, k++)
                szPath[k] = (*this)[j];
            szPath[k] = '\0';
            break;
        }

	return std::string(szPath);
}
*/
struct LuxSH
{
	Ut::UtVector<Ut::ArrayVector<float, 4>> red, green, blue;
};

struct LuxSamplePoint
{
	LuxSH sh[2];
	float iAnimIdx;
};

class Utility
{
public:
	template <typename T>
	static void GetValue(T& num, XERCES_CPP_NAMESPACE::DOMAttr* pAttr)
	{
		char* pValue = XMLString::transcode(pAttr->getValue());
		std::stringstream s;
		s << pValue;
		XMLString::release(&pValue);
		s >> num;
	}

	static const Aftk::C_MaterialResource*	CreateMaterial(Aftk::C_Scene* pScene, const Util::C_String& strFilename, const Util::C_String& stdNormalMapFilename, std::string sFXName);
	static INLINE void SetupDefaultSHBlindData(Aftk::C_SceneNode* pSource, Aftk::C_SceneNode* pDest);
	static const Aftk::C_GenericAttribute* FindAttribute(const Aftk::C_SceneNode& node, const Util::C_String& name);
	static void AddShaderImageNode(Aftk::C_MaterialNode *pNode, Aftk::C_ShaderNode *pShaderNode, std::string sNodeName, const Util::C_String &strFilename);

	/// returns true if the given node is meant to be lit using SH
	static INLINE bool DoesNodeHaveSHLighting(const Aftk::C_SceneNode &node);

	/// get SH lighting values for a given aftk scene node
	static INLINE bool GetNodeSHLighting(const Aftk::C_SceneNode &node, LuxSamplePoint &sample);

	// convert a hex digit into an int
	static INLINE int HexToI(char c);

	template<typename T>
	static INLINE bool ConvertHexToBin(const std::string &name, T *pData);
};

#include "AftkSpeedTreeUtility.inl"
}

#endif


