
#ifndef IPF_ADOBE_DEFINES_H
#define IPF_ADOBE_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Ipf
{

#define IPF_VersionMajor	( 16 ) // Increase this to force full export of all objects
#define IPF_VersionMinor	( 0 )  // Increase this when structor has changed but when previous versions can be 

// Return codes.
enum E_Result
{
	E_Result_None = 0,
	E_Result_Failed,
	E_Result_Success,
	E_Result_VersionDeprecated,
	E_Result_FileNotFound,
	E_Result_FileNotSupported,
	E_Result_ReadOnly,
};


enum E_ElementType
{
	E_ElementType_None = 0,		///< @a
	E_ElementType_Polygon,		///< Unbuilt shape
	E_ElementType_Shape,		///< @a
	E_ElementType_Group,		///< Layers are groups too.
	E_ElementType_Path,			///< @a

	E_ElementType_Count			///< @a
};

} // namespace Ipf

#endif // IPF_ADOBE_DEFINES_H