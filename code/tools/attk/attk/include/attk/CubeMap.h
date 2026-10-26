#ifndef ATTK_CUBE_MAP_H
#define ATTK_CUBE_MAP_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	class C_CubeMap
	{
	public:
		C_CubeMap();
		C_CubeMap(const C_CubeMap& rhs);

		~C_CubeMap();

		const C_CubeMap& operator =(const C_CubeMap& rhs);
		
		C_Image& operator [](U32 face);

		const C_Image& operator [](U32 face) const;

		void Clear();

		//Resizes all faces so that they are all the same size (to maximum width/height of all faces),
		//And converts all faces to the same pixel format. 
		C_Status Validate();

		C_Status ResizeAll(U32 width, U32 height);

		//Read from dds cube map texture
		C_Status Read(
			const char* pFileName,
			E_ImageFormat imageFormat = E_ImageFormat_BestGuess,
			const C_Options* pOptions = 0);

		//Writes out each face individually
		C_Status Write(
			const char* pFileNameFormat,	// c-style sprintf string (eg "image_%d.tga")
			E_ImageFormat imageFormat = E_ImageFormat_BestGuess,
			const C_Options* pOptions = 0) const;

	private:
		C_Image		m_Faces[E_CubeMap_Face_Count];
	};




}
#endif
