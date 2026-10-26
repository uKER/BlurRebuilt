#ifndef CONVERTER_SHARED_POLYFLAG_CONFIG_H
#define CONVERTER_SHARED_POLYFLAG_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#define USEPOLYFLAG_CACHE			1		// Use the polyflag cache.

#include <btl/containers/vector.h>
#include <btl/containers/chainedhashmap.h>
#include <aftk/Aftk.h>

namespace Aftk
{
	class C_Polygon;
	class C_SceneNode;
}

namespace Polyflags
{

typedef U32 C_Value;

class C_Polyflags;
class C_Config;
class C_DefineList;

typedef Btl::Vector<Util::C_String> C_PolyflagNames;

enum E_Source
{
	E_Source_Polyflags = 0,
	E_Source_TextureTags,
	E_Source_NodeFlags,

	E_Source_Count,
	E_Source_Default = E_Source_Polyflags
};

// C_Define

class C_Define
{
friend class C_DefineList;

public:
	C_Define();
	C_Define(const Util::C_String& name, const Util::C_String& value);

private:
	Util::C_String m_Name;
	Util::C_String m_Value;
};

class C_DefineList
{
public:
	void AddDefine(const Util::C_String& name, const Util::C_String& value);
	Util::C_String Process(const Util::C_String& input) const;
public:
	Btl::Vector<C_Define> m_DefineList;
};

// C_PolyflagDataSource
class C_PolyflagDataSource
{
public:
	C_PolyflagDataSource();
	C_PolyflagDataSource( const Aftk::C_SceneNode* pMeshNode, const Aftk::C_BlindDataResource* pBlindDataResource, const Aftk::C_MaterialNode* pMaterialNode );

	const Aftk::C_SceneNode* m_pSceneNode;
	const Aftk::C_BlindDataResource* m_pBlindDataResource;
	const Aftk::C_MaterialNode* m_pMaterialNode;
};

// I_Source
class I_Source
{
public:
	I_Source();
	virtual ~I_Source();	
	virtual bool ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList ) = 0;
	virtual C_Value GetValue( const C_PolyflagDataSource& dataSource ) const = 0;
	virtual void Dump(const Util::C_String& prefix);
	virtual void GetFlagNames( C_PolyflagNames& polyFlagNames ) const;
};

// C_Flag
class C_Flag : public I_Source
{
public:
	C_Flag();
	~C_Flag();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList );
	C_Value GetValue( const C_PolyflagDataSource& dataSource ) const;
	void Dump(const Util::C_String& prefix);
	void GetFlagNames( C_PolyflagNames& polyFlagNames ) const;
private:
	C_Value GetValue( const Aftk::C_BlindDataResource* pBlindDataResource ) const;
	C_Value GetValue( const Aftk::C_MaterialNode* pMaterialNode ) const;
	C_Value GetValue( const Aftk::C_SceneNode* pSceneNode ) const;

public:
	E_Source		m_Source;
	Util::C_String	m_Id;
	C_Value			m_Value;
};

// C_EnumItem
class C_EnumItem
{
public:
	C_EnumItem();
	~C_EnumItem();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList );
	void Dump(const Util::C_String& prefix);

public:
	U32				m_Index;
	Util::C_String	m_Id;
	C_Value			m_Value;
};

// C_Enum
class C_Enum : public I_Source
{
public:
	C_Enum();
	~C_Enum();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList );
	C_Value GetValue( const C_PolyflagDataSource& dataSource ) const;
	void Dump(const Util::C_String& prefix);
private:
	C_Value GetValue( const Aftk::C_BlindDataResource* pBlindDataResource ) const;
	C_Value GetValue( const Aftk::C_MaterialNode* pMaterialNode ) const;
	C_Value GetValue( const Aftk::C_SceneNode* pSceneNode ) const;

public:
	E_Source		m_Source;
	Util::C_String	m_Id;
	Btl::Vector<C_EnumItem> m_EnumItemList;
};

class C_LayerFlag : public I_Source
{
public:
	C_LayerFlag();
	~C_LayerFlag();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList );
	C_Value GetValue( const C_PolyflagDataSource& dataSource ) const;
	void Dump(const Util::C_String& prefix);

public:
	Util::C_String	m_Id;
	C_Value			m_Value;
};

// C_Container
class C_Container : public I_Source
{
public:
	C_Container();
	~C_Container();
	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList );
	C_Value GetValue( const C_PolyflagDataSource& dataSource ) const;
	void Dump(const Util::C_String& prefix);
	void GetFlagNames( C_PolyflagNames& polyFlagNames ) const;

public:
	enum E_Mode
	{
		E_Mode_Combine = 0,
		E_Mode_SelectFirst,
		E_Mode_MaskOut
	};

	E_Mode m_Mode;
	Btl::Vector<I_Source*> m_pSourceList;
};

// C_Item
class C_Item 
{
friend class C_Polyflags;
public:
	C_Item();
	C_Item( const Util::C_String& name );
	~C_Item();

	bool ParseXml( const Sys::C_XmlDomBaseNode* pNode, const C_DefineList& defineList );

	const Util::C_String& GetName() const;

	void GetFlagNames( C_PolyflagNames& polyFlagNames ) const;

	void Dump();

	C_Value GetValue( const C_PolyflagDataSource& dataSource ) const;

private:
	Util::C_String m_Name;
	I_Source* m_pSource;
};

// C_Config

class C_Config
{
friend class C_Polyflags;
public:
	C_Config();
	~C_Config();

	void Dump();

	bool ReadXml( const Util::C_String& xmlFilePath );

	bool RegisterItems( Btl::Vector<Util::C_String> itemNameList );

	C_Item* LookupItem( const Util::C_String& itemName );

	S32 GetItemCount() const;

	C_Item& GetItem( const S32 itemIndex );

	const C_Item& GetItem( const S32 itemIndex ) const;

private:
	bool ParseXml( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom );
	bool ParseXml( const Sys::C_XmlDomBaseNode* pRootNode );

private:
	Btl::Vector<C_Item> m_ItemList;
};


// C_Polyflags 

class C_Polyflags
{
public:
	/// Key for the PolyFlag cache.
	struct PolyflagCacheKey
	{
	public:
		/// C'tor.
		explicit PolyflagCacheKey(const Aftk::C_SceneNode* pSceneNode, const Aftk::C_BlindDataResource* pBlindDataResource, const Aftk::C_MaterialNode* pMaterialNode)
		{
			m_pSceneNode = pSceneNode;
			m_pBlindDataResource = pBlindDataResource;
			m_pMaterialNode = pMaterialNode;
			return;
		}
	
		const Aftk::C_SceneNode*			m_pSceneNode;
		const Aftk::C_BlindDataResource*	m_pBlindDataResource;
		const Aftk::C_MaterialNode*			m_pMaterialNode;

		// Disallow calls to this:
		explicit PolyflagCacheKey();
	};

	C_Polyflags();
	C_Polyflags( const C_Config& config, const Aftk::C_Polygon* pPolygon );
	C_Polyflags( const C_Config& config, const Aftk::C_SceneNode* pSceneNode );
	~C_Polyflags();

	C_Value GetValue( const S32 index );
	
private:
	void SetValues( const C_Config& config, const C_PolyflagDataSource& dataSource );

private: 
	Btl::Vector<C_Value> m_ValueList;
	static Btl::ChainedHashMap<PolyflagCacheKey, Btl::Vector<C_Value> > m_PolyflagCache;
};

} // namespace Polyflags

#endif
