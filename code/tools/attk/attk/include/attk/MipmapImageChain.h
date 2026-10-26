#ifndef ATTK_MIPMAP_CHAIN_H
#define ATTK_MIPMAP_CHAIN_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	///\brief Mipmap chain.
	///
	class C_MipmapImageChain
	{
	public:
		C_MipmapImageChain();
		
		~C_MipmapImageChain();

		U32 AddImagePtr(C_Image* pImage);

		U32 GetNumLevels() const;

		C_Image* GetLevelPtr(U32 mipLevel);

		const C_Image* GetLevelPtr(U32 mipLevel) const;

		void Clear();

	private:
		//Deny use of copy constructor.
		C_MipmapImageChain(const C_MipmapImageChain& rhs);

		//Deny use of asignment operator.
		const C_MipmapImageChain operator=(const C_MipmapImageChain& rhs);

		Ut::UtVector<Ut::DynamicVector<C_Image*> > m_mipLevels;
	};

	typedef C_MipmapImageChain C_MipmapCubeMapImageChain[E_CubeMap_Face_Count];
}

#endif
