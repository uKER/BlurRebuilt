#ifndef SPEEDTREE_BILLBOARD_TEXTURE_LAYOUT_INCLUDE
#define SPEEDTREE_BILLBOARD_TEXTURE_LAYOUT_INCLUDE


namespace SpeedtreeBillboard
{

	class C_TextureBounds
	{
	public:

		C_TextureBounds(Aftk::Vector2U32 min, Aftk::Vector2U32 max)
		: m_Min(min), m_Max(max)
		{ };

		C_TextureBounds(const C_TextureBounds& bounds)
		: m_Min(bounds.m_Min), m_Max(bounds.m_Max)
		{ };

		Aftk::Vector2U32 Size() { return Aftk::Vector2U32(m_Max.X()-m_Min.X(), m_Max.Y()-m_Min.Y()); };

		Aftk::Vector2U32 m_Min;
		Aftk::Vector2U32 m_Max;
	};


	template <typename Tag>
	class C_NonUniformTextureLayoutNode
	{
	public:
		C_NonUniformTextureLayoutNode()
		:	m_TextureBounds(C_TextureBounds(Aftk::Vector2U32(0, 0), Aftk::Vector2U32(0, 0))),
			m_Tag(0)
		{
			memset(&m_pChildren[0], 0, sizeof(C_NonUniformTextureLayoutNode<typename Tag>*)*2);
		};

		C_NonUniformTextureLayoutNode(const C_TextureBounds& bounds)
		:	m_TextureBounds(bounds),
			m_Tag(0)
		{
			memset(&m_pChildren[0], 0, sizeof(C_NonUniformTextureLayoutNode<typename Tag>*)*2);
		};

		~C_NonUniformTextureLayoutNode()
		{
			Clear();
		};

		const C_NonUniformTextureLayoutNode* Insert(const Aftk::Vector2U32& textureSize, const Tag* tag)
		{
			assert(textureSize.X()>0 && textureSize.Y()>0);

			if (m_Tag)
				return 0;

			// Recurse to child nodes if not a leaf
			if (m_pChildren[0]!=0 && m_pChildren[1]!=0)
			{
				const C_NonUniformTextureLayoutNode<typename Tag> *retVal;

				retVal = m_pChildren[0]->Insert(textureSize, tag);
				if (!retVal)
					retVal = m_pChildren[1]->Insert(textureSize, tag);

				return retVal;
			}

			// Check texture fits
			const Aftk::Vector2U32 mSize = m_TextureBounds.Size();
			if (textureSize.X() > mSize.X() ||
				textureSize.Y() > mSize.Y())
			{
				return 0;
			}

			// Texture does fit...
			// If perfect fit, dont need to create partition
			if ((mSize.X()-textureSize.X()) == 0 &&
				(mSize.Y()-textureSize.Y()) == 0)
			{
				m_Tag = tag;
				return this;
			}

			//spawn new child nodes
			Aftk::Vector2U32 delta(mSize.X()-textureSize.X(), mSize.Y()-textureSize.Y());

			if (delta.X() > delta.Y())
			{
				// Vertical split
				m_pChildren[0] = new C_NonUniformTextureLayoutNode< typename Tag>(
					C_TextureBounds(m_TextureBounds.m_Min,
									Aftk::Vector2U32(m_TextureBounds.m_Min.X() + textureSize.X(), m_TextureBounds.m_Max.Y())));
				m_pChildren[1] = new C_NonUniformTextureLayoutNode< typename Tag>(
					C_TextureBounds(Aftk::Vector2U32(m_TextureBounds.m_Min.X() + textureSize.X(), m_TextureBounds.m_Min.Y()),
									m_TextureBounds.m_Max));
			}
			else
			{
				// Horizontal split
				m_pChildren[0] = new C_NonUniformTextureLayoutNode< typename Tag>(
					C_TextureBounds(m_TextureBounds.m_Min,
									Aftk::Vector2U32(m_TextureBounds.m_Max.X(), m_TextureBounds.m_Min.Y() + textureSize.Y())));
				m_pChildren[1] = new C_NonUniformTextureLayoutNode< typename Tag>(
					C_TextureBounds(Aftk::Vector2U32(m_TextureBounds.m_Min.X(), m_TextureBounds.m_Min.Y() + textureSize.Y()),
									m_TextureBounds.m_Max));
			}

			return m_pChildren[0]->Insert(textureSize, tag);
		};

		void Clear()
		{
			delete m_pChildren[0];
			delete m_pChildren[1];

			m_pChildren[0] = 0;
			m_pChildren[1] = 0;
			m_Tag		   = 0;
		};

		C_NonUniformTextureLayoutNode<typename Tag>	*m_pChildren[2];
		C_TextureBounds						m_TextureBounds;
		const Tag							*m_Tag;
	};


	//A helper class to position textures in the final texture atlas
	//Uses a binary tree to recursively subdivide texture and check for space,
	//and can therefore handle textures of varying sizes
	template <typename Tag>
	class C_NonUniformTextureLayout
	{
	public:

		C_NonUniformTextureLayout(U32 width, U32 height)
		:	m_Width(width),
			m_Height(height),
			m_RootNode(C_TextureBounds(Aftk::Vector2U32(0, 0), Aftk::Vector2U32(width, height)))
		{ };

		~C_NonUniformTextureLayout()
		{
			Clear();
		}

		const C_TextureBounds* Insert(const Aftk::Vector2U32& textureSize, const Tag* tag)
		{
			const C_NonUniformTextureLayoutNode<typename Tag> *pTexturePosition =  m_RootNode.Insert(textureSize, tag);
			if (pTexturePosition)
				return &(pTexturePosition->m_TextureBounds);

			return 0;
		};

		void Clear()
		{
			m_RootNode.Clear();
		};

	private:
		C_NonUniformTextureLayoutNode<typename Tag> m_RootNode;
		U32		m_Width;
		U32		m_Height;

	};



	//Position textures in a uniform grid in atlas.
	// - obviously more predictable & we can keep count of how many more textures can be inserted.
	template <typename Tag>
	class C_UniformTextureLayout
	{
	public:
		C_UniformTextureLayout(U32 width, U32 height, U32 texWidth, U32 texHeight)
		:	m_Width(width), m_Height(height), m_TexWidth(texWidth), m_TexHeight(texHeight), m_NumTextures(0)
		{ }

		~C_UniformTextureLayout()
		{
			Clear();
		}

		const C_TextureBounds* Insert(const Tag* tag)
		{
			if (m_NumTextures < GetTextureCapacity())
			{
				U32 numRows = GetNumRows();
				U32 numColumns = GetNumColumns();

				U32 column = m_NumTextures % numRows;
				U32 row = m_NumTextures / numColumns;

				C_TextureBounds *pTextureBounds = new C_TextureBounds(
					Aftk::Vector2U32(column*m_TexWidth, row*m_TexHeight),
					Aftk::Vector2U32((column+1)*m_TexWidth, (row+1)*m_TexHeight));

				m_TextureTags[pTextureBounds] = tag;
				m_NumTextures++;

				return pTextureBounds;
			}
			else
				return 0;
		}

		void Clear()
		{
			Ut::UtMap<C_TextureBounds*, const Tag*>::Iterator it = m_TextureTags.Begin();
			while (it != m_TextureTags.End())
			{
				delete it->GetKey();
				++it;
			}
			
			m_TextureTags.Clear();
			m_NumTextures = 0;
		}

		U32 GetNumTextures()		{ return m_NumTextures; };
		U32 GetTextureCapacity()	{ return GetNumColumns() * GetNumRows(); };

	private:

		U32 GetNumColumns() { return m_Width/m_TexWidth; };
		U32 GetNumRows()	{ return m_Height/m_TexHeight; };

		U32 m_Width, m_Height, m_TexWidth, m_TexHeight;

		U32 m_NumTextures;
		Ut::UtMap<C_TextureBounds*, const Tag*> m_TextureTags;
	};

}
#endif

