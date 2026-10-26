#ifndef TEXTUREATLAS_ATLAS_H
#define TEXTUREATLAS_ATLAS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace TextureAtlas
{
	template <typename PackStrategy>
	class C_Atlas
	{
	public:
		C_Atlas();
		~C_Atlas();


		bool AddImage(const Btl::String& fileName, const ImageSize& size);
		bool RemoveImage(const Btl::String& fileName);

		const InputImageDescArray& GetInputImageDescArray() const;
		const OutputImageDescArray& GetOutputImageDescArray() const;

		bool Pack(ImageSizeArray& result, U32 maxWidth=2048, U32 maxHeight=2048, U32 maxAtlases=0, U32 border=0);

	private:
		InputImageDescArray		m_inputImageDesc;
		OutputImageDescArray	m_outputImageDesc;

		friend PackStrategy;	
	};

	#include "textureatlas/Atlas.inl"
}

#endif