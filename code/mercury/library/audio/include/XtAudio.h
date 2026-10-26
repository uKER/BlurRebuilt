#ifndef _DB_ENTITY_DEF_H_
#define _DB_ENTITY_DEF_H_

#include <Xt/XtGame.h>
#include <btl/containers/set.h>

// dont like including this. TODO: get rid of it somehow?
#include "AppInterfaceLayer.h"

//#include <resource_utils/ResourceUtilsDeclare.h>

namespace Audio
{

struct XtEntityDef;
struct XtEntityProxy;
struct XtEntityGroup;
struct XtAudioObject;
struct C_XtAudioPack;
struct C_XtAudioDb;

struct AudioXtCallbackContext : public XtCallbackContext
{
	inline AudioXtCallbackContext() : XtCallbackContext( 'AUD_' ) { }
};

const XtI32 InvalidIndex = 0xffffffff;

//-----------------------------------------------------------------------------
// C_XtAudioPack
//-----------------------------------------------------------------------------
struct C_XtAudioPack
{
public:
	friend struct C_XtAudioDb;

	~C_XtAudioPack() { assert(m_pXtPack == NULL); }
	C_XtAudioPack() : m_pXtPack(NULL) {}

	void LoadDefs(const Sys::C_FilePath& strDbFname, XtDb* pDb = NULL);
	void Flush(XtDb* pDbIn= NULL);

private:
	C_XtAudioDb*		m_pAudioDb;
	XtPack*				m_pXtPack;
};

//=============================================================================
// XtEntityGroup
//=============================================================================
struct XtEntityGroup
{
	XtSz8	name;
	XtU32	index;
};

typedef Ut::Vector< XtHandle<Audio::XtEntityDef> > XtEntityDefList;
typedef Ut::Vector< XtHandle<Audio::XtEntityProxy> > XtEntityProxyList;
typedef Ut::Vector< XtHandle<Audio::XtAudioObject> > XtAudioObjectList;
typedef Btl::List< XtHandle<Audio::XtEntityGroup> >  XtEntityGroupList;

//-----------------------------------------------------------------------------
// C_XtAudioDb
// this can be used with its own internal db (for use in standalone tests apps
// such as Audition or can pass a pre-created db in instead.
//-----------------------------------------------------------------------------
struct C_XtAudioDb
{
public:
	friend struct C_XtAudioPack;

	static C_XtAudioDb* Get() {	assert(s_pInstance); return s_pInstance; }

	static bool Create();
	void Release(XtDb* pDb = NULL);

	bool XtInit();

	void CreateEntityDefClasses();

	C_XtAudioPack&	AudioPack() { return m_AudioPack; }
	C_XtAudioPack&	EntitiesPack() { return m_EntitiesPack; }

	XtEntityProxy* LookupXtEntityProxy(const Util::C_String& strName);
	XtHandle<Audio::XtEntityProxy> LookupXtEntityProxyHandle(const Util::C_String& strName);
	
	bool SetupEntityResourceLists(const Sys::C_FilePath& strPath);

	void Flush(XtDb* pDb = NULL)
	{
		m_AudioPack.Flush(pDb);
		m_EntitiesPack.Flush(pDb);
	}

	// for internal use but needs to be public 
	// |-------
	void AddXtEntityProxy(XtHandle<XtEntityProxy> hProxy) { XtEntityProxies().PushBack(hProxy); }
	void AddXtEntityDef(XtHandle<XtEntityDef> hDef) { XtEntities().PushBack(hDef); }
	void AddXtAudioObject(XtHandle<XtAudioObject> hObj) { XtAudioObjects().PushBack(hObj); }
	void AddXtEntityGroup(XtHandle<XtEntityGroup> hGrp) { m_XtGroups.PushFront(hGrp); m_XtGroups.Sort(); }
	// |--------
private:
	C_XtAudioDb() : m_pXtDb(NULL) 
	{ 
		m_AudioPack.m_pAudioDb = this;
		m_EntitiesPack.m_pAudioDb = this;
	}
	~C_XtAudioDb() 
	{
		// Note: Flush() needs to be called before the destructor
		assert(!m_AudioPack.m_pXtPack);
		assert(!m_EntitiesPack.m_pXtPack);
		m_XtDefs.Destroy();
		m_XtProxyRecs.Destroy();
		m_XtAudioObjs.Destroy();
	}

	bool VerifyRecords();
	bool HasInternalDb() { return m_pXtDb ? true : false; }

	XtEntityDef* LookupXtEntity(const Util::C_String& strName);

public:
	XtEntityDefList& XtEntities() { return m_XtDefs; }

	XtEntityProxyList& XtEntityProxies() { return m_XtProxyRecs; }

	XtAudioObjectList& XtAudioObjects() { return m_XtAudioObjs; }

private:

	static C_XtAudioDb*					s_pInstance;

	C_XtAudioPack						m_AudioPack;
	C_XtAudioPack						m_EntitiesPack;

	XtDb*								m_pXtDb;
	AudioXtCallbackContext				m_Context;

	XtEntityDefList						m_XtDefs;			// list of the xt sorcery structs. used to create the EntityDefinition classes
	XtEntityProxyList					m_XtProxyRecs;		// list of the sorcery proxy records.
	XtAudioObjectList					m_XtAudioObjs;		// list of the xt audio object records
	XtEntityGroupList					m_XtGroups;			// list of all entity groups
};

#define AUDIO_DEFERRED_LOADING

//=============================================================================
// C_DeferredLoadManager
//=============================================================================
#ifdef AUDIO_DEFERRED_LOADING

extern Debug::Bool gbUseDeferredLoading;

typedef Btl::Set<Btl::String> ResourceList;

typedef Btl::Vector<Definition*> DefinitionList;
typedef Btl::Vector<WM_WAVE_HANDLE> BankHandleList;

class C_DeferredLoadManager
{
public:
	C_DeferredLoadManager() : m_pWaveManager(0), 
		                    m_bDeferred(false) {}

	static C_DeferredLoadManager& Get() { return m_sInstance; }

	void Load();
	void DecrementRefCounts();
	void Clear();
	
	bool AddBafsToQueue(Btl::Vector<Btl::String>& listBafNames);
	bool AddDatsToQueue(Btl::Vector<Btl::String>& listDatNames);

	void SetDeferredLoading(bool bTrueFalse) { m_bDeferred = bTrueFalse; }
	bool LoadingIsDeferred() const { if (gbUseDeferredLoading) return m_bDeferred; else return false; }

	void SetWaveManager(Audio::IWaveManager* pManager) { m_pWaveManager = pManager; }

private:

	void LoadBafs();
	void LoadDats();

	bool m_bDeferred;

	ResourceList m_BafList;		// list of banks to load
	ResourceList m_DatList;		// list of dats to load

	// definition pointers (one for each dat). we release these after all entity types have been created to get the ref counts back to how they should be
	DefinitionList m_DefsList;			

	// wave bank handles (one for each baf). same as above
	BankHandleList m_BankHandleList;

	Audio::IWaveManager* m_pWaveManager;

	static C_DeferredLoadManager m_sInstance;
};
#endif

//=============================================================================
// XtEntityProxy record
// This is the struct/record that is placed on objects in sorcery. Its purpose
// is just to link to an XtEntityDef, so the XtEntityDefs can be kicked out
// (after we've created EntityDefinition classes of them all)
// as we don't want to keep them around as they contains many strings.
// After setup, it contains the EntityDefinitionHandle which is needed to 
// create an instance of an entity.
//=============================================================================
struct XtEntityProxy
{
public:
	friend struct C_XtAudioDb;

	XtEntityProxy() : m_sResourceRefCount(0) {}

	const EntityDefinitionHandle&			GetEntityDefHnd() const { return m_hEntityDef; }

	AudioEntityTypeHandle&					GetEntityTypeHnd() { return m_hEntityType; }
	const AudioEntityTypeHandle&			GetEntityTypeHnd() const { return m_hEntityType; }

	//const XtHandle<Audio::XtEntityGroup>	XtGroup() const { return m_group; }
	const U32								Group() const { return m_group ? m_group->index : 0; }
	
	const XtHandle<Audio::XtEntityDef>		XtEntityDef() const { return m_hXtEntity; }

	bool									SetupEntityType(bool& bLoadedResources = m_sbDefaultBool);
	void									ReleaseEntityType(bool& bLoadedResources = m_sbDefaultBool);
	
	const char*								Name() const { return m_strName.Chars(); }
	void									SetName(const Btl::String& str) { m_strName = str; }

	bool									IsValid() const { return (m_hEntityDef != NULL && m_hEntityDef.IsValid()); }

	// these functions can be used to query which resources are needed to created the entity.
	// note: these will only be setup if SetupEntityResourceLists() has been called
	U32										NumDatResources() const { return m_DatFnameList.Size(); }
	U32										NumWaveBankResources() const { return m_WaveBankFnameList.Size(); }
	const Btl::String&						GetDatResourceName(U32 index) { return m_DatFnameList[index]; }
	const Btl::String&						GetWaveBankResourceName(U32 index) { return m_WaveBankFnameList[index]; }

	//void									AddToResourceBag(C_ResourceBag &bag) const;

private:
	void									SetEntity(EntityDefinitionHandle hDef);

public:
	Btl::String								m_strName;
	mutable XtHandle<Audio::XtEntityDef>	m_hXtEntity;			// Xt annotated. this will be NULL after EntityDef records are kicked out

	XtI32									id;					

private:
	// these vars are not xt annotated and are setup when the EntityDefinition classes are created.

	// list of files needed to setup/load this entity
	Btl::Vector<Btl::String>				m_DatFnameList;
	Btl::Vector<Btl::String>				m_WaveBankFnameList;

	XtHandle<Audio::XtEntityGroup>			m_group;

	mutable EntityDefinitionHandle			m_hEntityDef;		
	mutable AudioEntityTypeHandle			m_hEntityType;		
	mutable S32								m_sResourceRefCount;	// how many requests have we had for resource loads?

	static  bool							m_sbDefaultBool;
};

} // namespace Audio


#endif	//	#ifndef _DB_ENTITY_DEF_H_
