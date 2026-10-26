#ifndef TEXTUREATLAS_BINARY_PACK
#define TEXTUREATLAS_BINARY_PACK

#ifdef _MSC_VER
#pragma once
#endif

namespace TextureAtlas
{

	template <typename Node>
	class C_TreePackNode
	{
	public:
		C_TreePackNode(const ImageSize& size, U32 border=0);
		C_TreePackNode(const ImageRect& bounds, U32 border=0);

		~C_TreePackNode();
		void Clear();

		U32 GetBorderSize() const;
		const ImageRect& GetBounds() const;

		const C_TreePackNode* Insert(const Node* pNode);

	private:
		const Node*			m_pNode;
		C_TreePackNode*		m_pChildren[2];
		ImageRect			m_bounds;
		U32					m_border;
	};


	class C_BinaryTreePack
	{
	public:

		C_BinaryTreePack();
		~C_BinaryTreePack();

		bool Pack(const InputImageDescArray& input, OutputImageDescArray& output, const ImageSizeArray& atlasSizes, U32 border);
	};


#include "BinaryTreePack.inl"

}

#endif