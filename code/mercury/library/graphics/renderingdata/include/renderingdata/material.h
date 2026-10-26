#ifndef RD_MATERIAL_H
#define RD_MATERIAL_H
#ifdef _MSC_VER
#pragma once
#endif

#include "maths.h"
#include "resourceblock.h"
#include "datatypes.h"
#include "materialindex.h"
#include "system/scheduler/cache.h"

#if PLATFORM_SPU
#define USE_MATERIAL_CACHE 1
#endif

namespace RenderingData
{
	namespace Build {class Scene;}

	struct MaterialElement
	{
		Btl::String name; // Matches to a parameter name in the effect.
		MaterialElementType type;
		S32 count; // Number of components of the above type.
	};

	bool operator == (const MaterialElement& p1, const MaterialElement& p2);
	bool operator != (const MaterialElement& p1, const MaterialElement& p2);

	class MaterialType
	{
	public:
		MaterialType();

		void SetName(const Btl::String& s); // Name of the material type (which does not need to be unique).
		S32 AddElement(const MaterialElement& e, S32 align = 0); // (align = 0) -> use minimum alignment for the type.
		void SetDynamicListIndex(S32 listIndex); // 0 = 1st dynamic list
		void SetEffect(S32 effect);
		void SetParameterIndex(S32 element, S32 index);

		const Btl::String& GetName() const;
		S32 GetNumElements() const;
		const MaterialElement& GetElement(S32 i) const;
		S32 GetMaterialListIndex() const;
		S32 GetElementIndex(const Btl::String& name) const;
		S32 GetEffect() const;
		S32 GetMaxAlign() const;

		bool operator == (const MaterialType& other) const;

		U32 GetDataSize() const;
		U32 GetElementDataSize(S32 element) const;
		S32 GetOffset(S32 element, S32 index) const;

		FRIEND_SERIALISE(MaterialType);

	private:
		Btl::String m_name;
		Btl::Vector<MaterialElement> m_elements;

		U32 m_dataSize;
		U32 m_maxAlign;
		Btl::Vector<U16> m_offsets;

		S32 m_effect;
		S32 m_listIndex;
	};

	class MaterialList;
	class MaterialListData;
	class MaterialNode;
	namespace Build{class MaterialList;}

	// Data for effect parameters.
	class Material
	{
	public:
		Material();
		Material(const Material& material);
		Material& operator = (const Material& material);

		S32 GetTypeIndex() const; // Index of the material type in the list.
		
		// Access to the material data.
		U8* GetData();
		U8* GetData(S32 offset);
		const U8* GetData() const;
		const U8* GetData(S32 offset) const;

		// T should be the appropriate type in DataTypes.
		template<typename T> const T& GetData(S32 offset) const;
		template<typename T> void SetData(S32 offset, const T& t);

		void SetData(S32 offset, const void* p, U32 size);
		void SetData(const void* p, U32 size);

		U32 GetDataSize() const;

		// To match interface for cached materials
		void Release() const {}
		void Bind(const Material& material) const {}

		void OffsetLocation(S32 offset);

		friend class Build::MaterialList;
		friend class MaterialList;
		friend class MaterialNode;
		FRIEND_SERIALISE(MaterialList)
		template <class Archive> friend void DeserialiseMaterial(Archive& ar, Material& material, const MaterialType& materialType);
		template <typename Archive> friend void DeserialiseMaterialData(Archive& ar, MaterialListData& data, MaterialList& types, bool internalStorage);

	private:
		void Copy(const Material& material);

		U16 m_dataSize;
		U16 m_typeIndex : 11;
		U16 m_alignment : 5;

#if CFG_CONVERTER
		Btl::AutoPointer<U32, Btl::AlignedAllocator<U32> > m_pData;
#elif PLATFORM_SPU
		Btl::AutoPointer<U32, Btl::LinearAlignedAllocator<U32> > m_pData;
#else
		Btl::AutoPointer<U32, Btl::StackArrayAllocator<U32> > m_pData;
#endif
	};

	inline void MoveArray(void* pDst, Material* pSrc, int n) 
	{
		Btl::MoveByMemcopy<Material>::MoveArray(pDst, pSrc, n);
	}

#if USE_MATERIAL_CACHE

	struct ConstMaterialReference_Temp;

	class ConstMaterialReference
	{
	public:
		ConstMaterialReference(const Material& material, const MaterialListData& materialList, const U8* pData);
		ConstMaterialReference(ConstMaterialReference& other);
		ConstMaterialReference(const ConstMaterialReference_Temp& temp);
		ConstMaterialReference& operator = (ConstMaterialReference& other);
		ConstMaterialReference& operator = (const ConstMaterialReference_Temp& temp); 
		operator ConstMaterialReference_Temp ();
		
		~ConstMaterialReference();

		S32 GetTypeIndex() const; // Index of the material type in the list.

		// Access to the material data.
		const U8* GetData() const;
		const U8* GetData(S32 offset) const;

		// T should be the appropriate type in DataTypes.
		template<typename T> const T& GetData(S32 offset) const;

		U32 GetDataSize() const;

		void Release();
		void Bind(ConstMaterialReference& other);
		void Bind(const ConstMaterialReference_Temp& temp);

	private:
		operator ConstMaterialReference_Temp () const;

		friend class MaterialListData;

		const Material& m_material;
		const MaterialListData& m_materialList;
		const U32* m_pData;
	};

#else

	typedef const Material& ConstMaterialReference;

#endif

	bool operator < (const Material& m1, const Material& m2);
	bool operator == (const Material& m1, const Material& m2);

	class MaterialListData
	{
	public:
		MaterialListData();

		void CopyData(const MaterialListData& materialList, bool internalStorage = true);

#if !CFG_CONVERTER && !PLATFORM_SPU
		void Begin(S32 reservedBytes = 0); // Call before adding materials.
		void End(); // Call when finished adding materials.
#endif

		S32 GetNumMaterials() const;

		ConstMaterialReference GetMaterial(S32 i) const;
		const void* GetDmaData() const;
		void SpuFetch(const void* pData, Btl::StackAllocatorSimple<>* pAllocator, Spu::CircularCache* pCache, const S32 offset = 0, const S32 count = 0);

#if USE_MATERIAL_CACHE
		void Release(ConstMaterialReference& m) const;
#else
		void Release(ConstMaterialReference m) const;
#endif

		S32 GetDataSize() const;

		template <typename Archive> friend void DeserialiseMaterialData(Archive& ar, MaterialListData& data, MaterialList& types, bool internalStorage);
	protected:

		struct ALIGN16 DmaData
		{
			const void* materials;
			SrcData buffer;
		};

		mutable DmaData m_dmaData;
		
#if !CFG_CONVERTER && !PLATFORM_SPU
		AutoStackAllocator m_autoStackAllocator;
#endif

#if USE_MATERIAL_CACHE
		Spu::CircularCache* m_pCache;
#endif
		typedef Btl::Vector<Material> VectorType;
		VectorType ALIGN16 m_materials;
	};

	class MaterialList : public MaterialListData
	{
	public:
		void CreateMaterial(Material& m, S32 typeIndex) const;
		const MaterialType& GetTypeOfMaterial(S32 i) const;

		S32 GetNumMaterialTypes() const;
		const MaterialType& GetMaterialType(S32 i) const;
		S32 FindMaterialType(const Btl::String& name) const;

		void Prefetch() const;

		// Copy the types from another list. Any existing materials are destroyed.
		void CopyTypes(const MaterialList& other);

		FRIEND_SERIALISE(MaterialList)

	protected:
		Btl::Vector<MaterialType> m_materialTypes;
	};

	extern S32 materialDataTypeSizes[];
	extern S32 materialDataTypeAlignment[];

	// Node that stores a single material.
	class MaterialNode : public CullNode
	{
	public:
		void CreateMaterial(const MaterialType& t);

		const MaterialType& GetMaterialType() const;
		const Material& GetMaterial() const;
		Material& GetMaterial();
		S32 GetNumTextures() const; // 0 during build, # textures in material on target.
		const ResourceIndex& GetTexture(S32 i) const;

		// Node functions.
		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual MaterialNode* Clone() const;

		// If p is a MaterialNode, return a pointer to it, otherwise return a null pointer.
		static const MaterialNode* GetPointer(const Node* p);
		static MaterialNode* GetPointer(Node* p);

		friend class Build::Scene;
		FRIEND_SERIALISE(MaterialNode)
		VIRTUAL_SERIALISATION_KEY_DECL

	private:
		MaterialType m_materialType;
		Material m_material;
		Btl::Vector<ResourceIndex> m_textureSource;
	};

	#include "noninterface/material.inl"
}

#endif

