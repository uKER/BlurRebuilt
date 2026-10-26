#ifndef SPEEDTREE_BILLBOARD_ATLAS_DEFINITION_INCLUDE
#define SPEEDTREE_BILLBOARD_ATLAS_DEFINITION_INCLUDE


#ifdef _MSC_VER
#pragma once
#endif

#include "TreeRendererSettings.h"

namespace Sys
{
	class I_File;
}

namespace SpeedtreeBillboard
{

	extern Debug::Logger& g_Log;

	class C_TreeBillboard
	{
	public:

		struct TreeBillboardVariation
		{
			F32 m_Azimuth;
			F32 m_Elevation;
			/*E_Conditions*/U32 m_Condition;
			U32 m_TextureIndex;

			F32 m_MinX, m_MinY, m_MaxX, m_MaxY;

			TreeBillboardVariation()
			:	m_Azimuth(0.f), m_Elevation(0.f), m_Condition(E_Condition_Sunny),
				m_TextureIndex(0),
				m_MinX(0.f), m_MinY(0.f), m_MaxX(0.f), m_MaxY(0.f)
			{ };
		};

		C_TreeBillboard(
			Util::C_String sptName = "",
			U32 seed = 0,
			F32 size = 0.0f,
			F32 sizeVariance = 0.0f,
			F32 azimuth = 0.0f,
			F32 elevation = 0.0f);

		~C_TreeBillboard();

		const Util::C_String& GetName() const;
		void SetName(Util::C_String& treeName);

		S32 GetSeed() const;
		void SetSeed(S32 seed);

		F32 GetSize() const;
		void SetSize(F32 size);

		F32 GetSizeVariance() const;
		void SetSizeVariance(F32 sizevariance);

		U32 GetNumVariants() const;

		U32 AddVariant(const TreeBillboardVariation& variant);
		const TreeBillboardVariation& GetVariant(U32 index) const;

		template <typename S>
		void Serialise(S& s) const;

		template <typename S>
		void Deserialise(S& s);

		U32 GetHash() const;

	private:

		Util::C_String		m_Name;
		S32					m_Seed;
		F32					m_Size;
		F32					m_SizeVariance;

		Ut::UtVector< Ut::DynamicVector<TreeBillboardVariation> > m_Variants;
	};

	inline U32 C_TreeBillboard::GetHash() const
	{
		U32 hash = Ut::HashKeyOpString::GetHash(m_Name.Chars());
		hash += Ut::HashKeyOpByteArray<S32>::GetHash(m_Seed);
		//hash += Ut::HashKeyOpByteArray<F32>::GetHash(m_Size);
		//hash += Ut::HashKeyOpByteArray<F32>::GetHash(m_SizeVariance);
		return hash;
	};

	class HashKeyOpTreeKey
	{
	public:
		static U32 GetHash(const C_TreeBillboard *pKey)
		{
			return pKey->GetHash();
		}

		static bool Equal(
			const C_TreeBillboard *pKey1,
			const C_TreeBillboard *pKey2)
		{
			if (pKey1->GetHash() == pKey2->GetHash())
				return true;
			else
				return false;
		}
	};


	class C_TreeBillboardSet
	{
	public:
		struct TextureNameSet
		{
			Util::C_String m_Diffuse;
			Util::C_String m_Normal;
		};

		C_TreeBillboardSet();
		~C_TreeBillboardSet();

		U32 CreateNewTextureSet(const Util::C_String& diffuse, const Util::C_String & normal);
		const TextureNameSet& GetTextureNameSet(U32 set);
		U32 GetNumTextureNameSets() const;

		void AddTreeBillboardRecord(const C_TreeBillboard& billboard);
		const C_TreeBillboard& GetTreeBillboardRecord(U32 index) const;
		U32 GetNumTreeBillboardRecords() const;

		inline bool Serialise(Sys::I_File& file) const;
		inline bool Deserialise(Sys::I_File& file);

	private:

		Ut::UtVector< Ut::DynamicVector<TextureNameSet> >	m_BillboardTextures;
		Ut::UtVector< Ut::DynamicVector<C_TreeBillboard> >	m_TreeBillboards;

		const char* m_TreeBillboardSet_HeaderBlock;
		const char* m_TreeBillboardSet_TextureBlock;
		const char* m_TreeBillboardSet_TreeBlock;

		static const U32 m_Version;
	};

}
	#include "TreeDefinition.inl"

#endif