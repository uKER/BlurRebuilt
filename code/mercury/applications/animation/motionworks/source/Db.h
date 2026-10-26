#ifndef DB_H
#define DB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

class C_XtDatabaseListener;

class C_XtDatabase
{
private:
	C_XtDatabase();
	~C_XtDatabase();
public:
	
	static C_XtDatabase& Instance();
	
	static bool CopyDbTemplate( const char* srcPath, const char* dstPath );
	
	bool IsOpen() const;
	
	const char* GetDbPath() const;

	bool Open( const char* path );
	bool Reopen();
	void Close();
	bool Commit();	
	
	Xt::DomDb* GetDomDb();
	
	// Update DB with current DOM DB contents.
	void UpdateFromPack( Xt::DomPack* pack );
	void UpdateFromRecord( Xt::DomRecord* record );
	
	Xt::DomPack* NewPack( const char* name );
	Xt::DomPack* FindPack( const char* name );
	U32 GetNumPacks();
	void DeletePack( Xt::DomPack* pack );
	
	Xt::DomRecord* NewRecord( Xt::DomPack* pack, const char* name, const char* typeName );
	Xt::DomRecord* FindRecord( const char* name, const char* typeName );
	void DeleteRecord( Xt::DomRecord* record );
	
	void AddListener( C_XtDatabaseListener* listener );
	void RemoveListener( C_XtDatabaseListener* listener );
			
private:

	void AddMonitor( Xt::DomElement* element );
	void ClearMonitors();

	class C_Storage : public Xt::Storage
	{
	public:
		virtual bool Start();
		virtual bool New(const char* filename);
		virtual bool Dirty(const char* filename);
		virtual bool Rename(const char* oldFilename, const char* newFilename);
		virtual bool Remove(const char* filename);
		virtual bool IsModified(const char* filename);
		virtual bool Stop();
	};
	
	static void MonitorCallback( Xt::DomMonitor* monitor, Xt::DomEvent* event, void* context );

	XtDb*				m_db;
	Xt::DomDb			m_domDb;
	C_Storage			m_storage;
	XtCallbackContext	m_xtCallbackContext;
	Btl::Vector<Xt::DomMonitor*> m_monitors;
	Btl::Vector<C_XtDatabaseListener*> m_listeners;
	XtString			m_dbPath;
	Xt::DomPack*		m_lastUpdatedPack;
	Xt::DomRecord*		m_lastUpdatedRecord;
	bool				m_open;
	bool				m_dbDirty;
	bool				m_doingUpdate;
};

class C_XtDatabaseListener
{
public:
	C_XtDatabaseListener():
		m_pDb(NULL)
	{
	}
	
	explicit C_XtDatabaseListener( C_XtDatabase* db ):
		m_pDb(NULL)
	{
		SetXtDatabase( db );
	}

	virtual ~C_XtDatabaseListener()
	{
		SetXtDatabase( NULL );
	}

	virtual void OnXtDatabaseChanged( Xt::DomEvent* domEvent ) = 0;
	
	void SetXtDatabase( C_XtDatabase* db )
	{
		if ( m_pDb )
		{
			m_pDb->RemoveListener( this );
		}
		m_pDb = db;
		
		if ( db )
		{
			db->AddListener( this );
		}
	}
	
private:
	C_XtDatabase* m_pDb;
};

/*
	Helper functions for working with specifically structured databases.
	
	Part of application core state.
	
	Create cycle DOM block in the currently selected graph record.
		-> automatically moves node to centre of graph and populates
		   certain fields according to cycle path.
	
	CreateCycleBlock( Xt::DomRecord* graphRecord, const char* cycleFilename )
	
	
	State:
		- Currently selected graph record.
		- DomPack for currently selected record.
			
	
*/

class C_AnimationGraphEditor: public C_XtDatabaseListener
{
public:
	C_AnimationGraphEditor( C_XtDatabase& db );
	
	virtual ~C_AnimationGraphEditor();
		
	
	Xt::DomRecord* NewGraphRecord( Xt::DomPack* pack, const char* name );
	
	Xt::DomRecord* GetCurrentGraphRecord();
	
	void SetCurrentGraphRecord( Xt::DomRecord* record );
	
	Xt::DomRecord* FindGraphRecord( const char* name );
	
	Xt::DomPack* GetCurrentPack(); 

	Xt::DomBlock* CreateCycleBlock( Xt::DomRecord* graphRecord, const char* cycleFilename );
	
	virtual void OnXtDatabaseChanged( Xt::DomEvent* domEvent );
	
private:

	struct C_Bounds
	{
		C_Bounds():
			minX(0.f),
			maxX(0.f),
			minY(0.f),
			maxY(0.f)
		{}

		F32	minX;
		F32 maxX;
		F32 minY;
		F32 maxY;
	};


	void FindNodeGraphBounds( Xt::DomArray* nodeArray, C_Bounds* outBounds ) const;

	C_XtDatabase&	m_db;
	Xt::DomRecord*	m_currentGraphRecord;
	Xt::DomPack*	m_currentGraphPack;
	XtString		m_currentGraphName;
	XtString		m_graphTypeName;
	XtString		m_cycleTypeName;
	XtString		m_nodeArrayName;
	XtString		m_currentGraphRecordName;

};





#endif
