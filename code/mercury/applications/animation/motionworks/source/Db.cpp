#include "stdafx.h"
#include "Db.h"
#include "AnimationGraph.h"
#include <wx/filename.h>
#include <wx/dir.h>

XT_APPLICATION
	XT_DEPENDS_ON(AnimationGraph)
XT_END_APPLICATION

Debug::Logger& gLog = Debug::GetLogger("motionworks.db");

class C_XtStatusReport: public Xt::StatusCallback
{
public:
	virtual void Status(const Xt::StatusMessage& msg)
	{
		if (msg.severity == Xt::StatusMessage::kError)
		{
			errorMessage = msg.message.Text();
		}
	}
	
	Btl::String		errorMessage;
};

//
// C_XtDatabase::C_Storage
//
bool C_XtDatabase::C_Storage::Start()
{
	return true;
}

bool C_XtDatabase::C_Storage::New(const char* filename)
{
	return true;
}

bool C_XtDatabase::C_Storage::Dirty(const char* filename)
{
	return true; 
}

bool C_XtDatabase::C_Storage::Rename(const char* oldFilename, const char* newFilename)
{
	// This is just signalling a rename: the new file already exists, so just delete the old one.
	::wxRemoveFile( oldFilename );
	return true;
}

bool C_XtDatabase::C_Storage::Remove(const char* filename)
{
	return ::wxRemoveFile( filename );
}

bool C_XtDatabase::C_Storage::IsModified(const char* filename)
{	
	return true;
}

bool C_XtDatabase::C_Storage::Stop()
{
	return true;
}

//
// C_XtDatabase
//

C_XtDatabase::C_XtDatabase():
	m_db(NULL),
	m_xtCallbackContext( 0 ),
	m_lastUpdatedPack(NULL),
	m_lastUpdatedRecord(NULL),
	m_open(false),
	m_dbDirty(true),
	m_doingUpdate(false)
{
	XtInitialise();
	m_db = XtDbCreate(XtGetApplicationSchema(), 8000, &m_xtCallbackContext);
}

C_XtDatabase::~C_XtDatabase()
{
	Close();
	
	// invalid pointers to DB held by listeners?
	for ( S32 i = 0; i < m_listeners.Size(); ++i )
	{
		m_listeners[i]->SetXtDatabase( NULL );
	}
	
	XtDbDestroy(m_db);
	
	ClearMonitors();
	
	m_db = NULL;
	XtShutdown();
}

C_XtDatabase& C_XtDatabase::Instance()
{
	static C_XtDatabase inst;
	return inst;
}

void MakeWriteable( const wxString& filename )
{
	if ( ::wxFileExists( filename ) )
	{
		U32 attr = ::GetFileAttributes( filename.c_str() );
		attr &= ~FILE_ATTRIBUTE_READONLY;	
		::SetFileAttributes( filename, attr );
	}
}

bool CopyFileAndMakeDir( const wxString& src, const wxString& dst )
{
	wxFileName srcFilename(src);
	wxFileName dstFilename(dst);
	
	bool ok = true;
	if ( !::wxDirExists( dstFilename.GetPath() ) )
	{
		wxFileName dstDir = wxFileName::DirName( dstFilename.GetPath() );
		LOGINFO( gLog, "Making dir '%s'", dstDir.GetPath().c_str() );
		ok = dstDir.Mkdir( 511, wxPATH_MKDIR_FULL );
		if ( !ok ) return false;
	}
	
	// To prevent the copy failing, make sure destination file isn't read only.
	MakeWriteable( dstFilename.GetFullPath() );
		
	LOGINFO( gLog, "Copying '%s' to '%s'", srcFilename.GetFullPath().c_str(), dstFilename.GetFullPath().c_str() );
	ok = ::wxCopyFile( srcFilename.GetFullPath(), dstFilename.GetFullPath() );
	
	return ok;
}

int GetXtConfigFileVersion(const char* path)
{
	C_XtStatusReport status;
	Xt::DomBlock dom;
	bool ok = Xt::Import2(&dom, path, 0, &status);
	int version = -1;
	if (ok)
	{
		version = atoi(Xt::AsText(Xt::At(&dom, "__version")));
	}
	return version;
}

bool C_XtDatabase::CopyDbTemplate( const char* srcPath, const char* dstPath )
{
	wxFileName srcDir = wxFileName::DirName( srcPath );
	wxFileName dstDir = wxFileName::DirName( dstPath );

	// Copy schema folder and everything inside it.
	wxArrayString files;
	wxDir::GetAllFiles( srcDir.GetPathWithSep() + "schema", &files, wxEmptyString, wxDIR_FILES | wxDIR_DIRS );
	
	wxFileName dstSchemaDir( dstDir.GetPathWithSep() );
	dstSchemaDir.AppendDir( "schema" );
	
	bool ok = true;
	for ( size_t i = 0; i < files.size(); ++i )
	{
		wxFileName srcFilename( files[i] );
		wxString dstFilename = dstSchemaDir.GetPathWithSep() + srcFilename.GetFullName();
		ok = CopyFileAndMakeDir( files[i], dstFilename );
		if ( !ok ) return false;
	}
	
	// Copy db.xtd
	if (!CopyFileAndMakeDir( srcDir.GetPathWithSep() + "db.xtd", dstDir.GetPathWithSep() + "db.xtd"))
	{
		return false;
	}
	
	// Copy config.xtp
	wxString srcFilename = srcDir.GetPathWithSep() + "config.xtp";
	wxString dstFilename = dstDir.GetPathWithSep() + "config.xtp";
	
	if (::wxFileExists(dstFilename))
	{
		int userVersion = GetXtConfigFileVersion(dstFilename);
		int currentVersion = GetXtConfigFileVersion(srcFilename);
		
		if (userVersion < currentVersion)
		{
			// We have a problem: the database has changed. The old packs will be moved into Packs\Backup.
			
			::wxMessageBox(
				"The Sorcery file format has changed.\n"
				"Your existing animation graph packs will be backed up and a new empty database created.\n"
				"If you would like to upgrade your old data, please see Chris Cookson.",
				"Important Message", 
				wxOK | wxICON_EXCLAMATION
			);
			
			wxArrayString packFiles;
			wxDir::GetAllFiles(dstDir.GetPathWithSep() + "packs", &packFiles, "*.xtd", wxDIR_FILES);
			
			wxString backupDir(dstDir.GetPathWithSep() + "backup" + wxFileName::GetPathSeparator() + wxString::Format("Version_%3d", userVersion));
						
			ok = true;
			for (U32 i = 0; i < packFiles.GetCount(); ++i)
			{
				wxFileName packFilename(packFiles[i]);
				wxString backupFilename = backupDir + wxFileName::GetPathSeparator() + packFilename.GetFullName();
				
				bool copyOk = CopyFileAndMakeDir( packFilename.GetFullPath(), backupFilename );
				LOGINFO(gLog, "Moving '%s' to '%s' (%s)\n", packFilename.GetFullPath().c_str(), backupFilename.c_str(), ok ? "OK" : "Failed");
				
				// Delete existing file.
				if (copyOk)
				{
					::wxRemoveFile(packFilename.GetFullPath());
				}
				ok = ok && copyOk;
			}
			
			if (!ok)
			{
				int result = ::wxMessageBox("An error occurred whilst backing up your data. Do you want to continue?", "Backup Error", wxYES_NO | wxICON_ERROR);
				if (result == wxNO)
				{
					return false;
				}
			}
		}
	}
		
	ok = CopyFileAndMakeDir( srcFilename, dstFilename );
	
	MakeWriteable( dstFilename );
	
	// Create packs folder.
	wxFileName dstPacksFolder = wxFileName::DirName( dstDir.GetPathWithSep() + "packs" );
	
	if ( !dstPacksFolder.DirExists() )
	{
		dstPacksFolder.Mkdir( 511, wxPATH_MKDIR_FULL );
	}
	
	return ok;
}

bool C_XtDatabase::IsOpen() const
{
	return m_open;
}

const char* C_XtDatabase::GetDbPath() const
{
	return m_dbPath.Text();
}

bool C_XtDatabase::Open( const char* path ) // path = db
{
	Close();
	
	m_dbPath = path;

	C_XtStatusReport status;
	Xt::OpenResult result = Xt::Open( &m_domDb, path, &m_storage, &status );
	
	if ( result != Xt::kOpenSuccess )
	{
		LOGERROR( gLog, "Error opening Xt database: %s\n", status.errorMessage.Chars() );
		return false;
	}
	
	// Monitor DOM database for changes.
	AddMonitor( &m_domDb );
	
	// For some reason Xt::Notify doesn't propogate events to the root DB object.
	// To get round this, add a monitor to every record.
	//for ( XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&m_domDb.records); it; ++it )
	//{
	//	LOGINFO( gLog, "Monitoring %s", it->name.Text() );
	//	
	//	AddMonitor( it );
	//}
	
	m_open = true;
	
	return true;
}

bool C_XtDatabase::Reopen()
{
	XtString p( m_dbPath );
	return Open( p );
}

void C_XtDatabase::Close()
{
	if ( m_open )
	{
		Xt::StatusCallback status;
		Xt::Close( &m_domDb, &m_storage, &status );
		ClearMonitors();
		m_open = false;
	}
}

bool C_XtDatabase::Commit()
{
	if ( !m_open ) return false;
	Xt::StatusCallback status;
	bool result = Xt::Flush(&m_domDb, &m_storage, &status);

#if 0	
	// ... is this even necessary?
	for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&m_domDb.records); it; ++it)
	{
		Notify(it, &Xt::DomRecordChanged(it));
	}
#endif
	
	return result;
}

Xt::DomDb* C_XtDatabase::GetDomDb()
{
	return m_open ? &m_domDb : NULL;
}

void PackOpenCallback(XtPackOpenStatus* status, void* context)
{
}

void C_XtDatabase::UpdateFromPack( Xt::DomPack* pack )
{
	if ( !pack ) return;
	
	if ( m_doingUpdate ) return;
	m_doingUpdate = true;
			
	if ( m_lastUpdatedPack != pack || m_lastUpdatedRecord != NULL )
	{
		m_dbDirty = true;
	}
	m_lastUpdatedPack = pack;
	m_lastUpdatedRecord = NULL;
	
	
	if ( m_dbDirty && m_db )
	{
		XtTargetProfile targetProfile;
		targetProfile.endian = XtGetEndian();
		XtUt::Block block;
		XtPackBuildParams packBuildParams;
		bool ok = XtPackBuild(&targetProfile, &m_domDb, pack, &block, &packBuildParams);
		
		if ( ok )
		{
			XtPackOpenParams params;
			params.callback = &PackOpenCallback;
			params.context = NULL;
			
			XtPack* readPack = XtPackOpen( m_db, block.start, block.end - block.start, &params );
		}
		else
		{
			LOGWARN( gLog, "XtPackBuild error\n");
		}
		
		m_dbDirty = false;
	}
	m_doingUpdate = false;
}

void C_XtDatabase::UpdateFromRecord( Xt::DomRecord* record )
{
	if ( !record ) return;
	if ( m_doingUpdate ) return;
	m_doingUpdate = true;
	
	if ( m_lastUpdatedPack != NULL || m_lastUpdatedRecord != record )
	{
		m_dbDirty = true;
	}
	m_lastUpdatedPack = NULL;
	m_lastUpdatedRecord = record;	
	
	if ( m_dbDirty && m_db )
	{
		XtTargetProfile targetProfile;
		targetProfile.endian = XtGetEndian();
		XtPackBuildParams packBuildParams;
		XtUt::Block block;
		bool ok = XtPackBuild(&targetProfile, &m_domDb, &record, 1, &block, &packBuildParams);
		
		if ( ok )
		{
			XtPackOpenParams params;
			params.callback = &PackOpenCallback;
			params.context = NULL;
			
			XtPack* readPack = XtPackOpen( m_db, block.start, block.end - block.start, &params );
		}
		else
		{
			LOGWARN( gLog, "XtPackBuild error\n");
		}
		
		m_dbDirty = false;
	}
	m_doingUpdate = false;
}

Xt::DomPack* C_XtDatabase::NewPack( const char* name )
{
	if ( !m_open ) return NULL;
	
	XtString uniqueName;
	S32 index = 1;
	bool notUnique = true;
	while ( notUnique )
	{
		uniqueName = Format( "%s %d", name, index );
		++index;
		notUnique = (NULL != FindPack( uniqueName ));
	}
	
	Xt::DomPack* pack = Xt::NewPack( &m_domDb, uniqueName );
	pack->dirty = true;
	return pack;
}
	
Xt::DomPack* C_XtDatabase::FindPack( const char* name )
{
	if ( !m_open ) return NULL;
	
	for ( XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&m_domDb.packs); it; ++it )
	{
		if ( it->name == name )
		{
			return it;
		}
	}
	return NULL;
}

U32 C_XtDatabase::GetNumPacks()
{
	if ( !m_open ) return 0;
	U32 count = 0;
	for ( XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&m_domDb.packs); it; ++it )
	{
		++count;
	}
	return count;
}

void C_XtDatabase::DeletePack( Xt::DomPack* pack )
{
	if ( !IsOpen() ) return;
	
	XtString packFilename = pack->filename;
	
	Xt::RemovePack( &m_domDb, pack->id );
	
	if ( packFilename.Length() > 0 )
	{
		m_storage.Remove( packFilename );
	}
}

Xt::DomRecord* C_XtDatabase::NewRecord( Xt::DomPack* pack, const char* name, const char* typeName )
{
	if ( !m_open ) return NULL;
	
	XtString uniqueName;
	S32 index = 1;
	bool notUnique = true;
	while ( notUnique )
	{
		uniqueName = Format( "%s %d", name, index );
		++index;
		notUnique = (NULL != FindRecord( uniqueName, typeName ));
	}
	
	Xt::DomRecord* r = Xt::NewRecord( pack, uniqueName, typeName );
	Xt::SetDirty( r, false );
	SetDefaults( m_domDb.schema, Xt::TypeOf( m_domDb.schema, r ), &r->root );
	
	// Monitor record for changes.
	AddMonitor( r );
	
	return r;
}

Xt::DomRecord* C_XtDatabase::FindRecord( const char* name, const char* typeName )
{
	if ( !m_open ) return NULL;
	
	for ( Xt::DomDb::Iterator dbIter(&m_domDb); dbIter; ++dbIter )
	{
		for ( Xt::DomPack::Iterator it(dbIter); it; ++it )
		{
			if ( it->name == name && it->root.type == typeName )
			{
				return it;
			}
		}
	}
	return NULL;
}

void C_XtDatabase::DeleteRecord( Xt::DomRecord* record )
{
	if ( !IsOpen() ) return;
	Xt::RemoveRecord( &m_domDb, record->id );
}

void C_XtDatabase::AddListener( C_XtDatabaseListener* listener )
{
	if ( listener == NULL ) return;

	// Don't add duplicates.
	for ( S32 i = 0; i < m_listeners.Size(); ++i )
	{
		if ( m_listeners[i] == listener )
		{
			return;
		}
	}

	m_listeners.PushBack( listener );
}

void C_XtDatabase::RemoveListener( C_XtDatabaseListener* listener )
{
	for ( S32 i = 0; i < m_listeners.Size(); ++i )
	{
		if ( m_listeners[i] == listener )
		{
			m_listeners.EraseByIndex( i );
			break;
		}
	}
}

void C_XtDatabase::AddMonitor( Xt::DomElement* element )
{
	m_monitors.PushBack( new Xt::DomMonitor(&C_XtDatabase::MonitorCallback, (void*) this ) );
	Xt::Monitor( m_monitors.Back(), element );
}

void C_XtDatabase::ClearMonitors()
{
	for ( S32 i = 0; i < m_monitors.Size(); ++i )
	{
		delete m_monitors[i];
	}
	m_monitors.Clear();
}


void C_XtDatabase::MonitorCallback( Xt::DomMonitor* monitor, Xt::DomEvent* event, void* context )
{
	C_XtDatabase* self = (C_XtDatabase*) context;

	if ( !self->m_open )
	{
		return;
	}
	
	bool update = true;
	if ( event->code == Xt::kDomFieldChanged )
	{
		if ( Substring(event->from->parentKey, 0, 2) == "__" && event->from->parentKey != "__type" )
		{
			update = false;
		}
	}
	/*
	else if ( event->code == Xt::kDomRemoved ||
			   event->code == Xt::kDomDbClosing )
	{
		update = false;
	}
	*/
	
	if ( update )
	{
		for ( S32 i = 0; i < self->m_listeners.Size(); ++i )
		{
			self->m_listeners[i]->OnXtDatabaseChanged( event );
		}
	}
	
	self->m_dbDirty = self->m_dbDirty || update;	
}

//
// C_AnimationGraphEditor
//

C_AnimationGraphEditor::C_AnimationGraphEditor( C_XtDatabase& db ):
	C_XtDatabaseListener( &db ),
	m_db( db ),
	m_currentGraphRecord(NULL),
	m_currentGraphPack(NULL),
	m_graphTypeName( "Graph" ),
	m_cycleTypeName( "CycleNode" ),
	m_nodeArrayName( "Nodes" )
{

	assert( db.IsOpen() );
	
	if ( db.IsOpen() )
	{
		// Create defaults.
		if ( db.GetNumPacks() == 0 )
		{
			Xt::DomPack* pack = db.NewPack( "New Pack" );
			Xt::DomRecord* record = NewGraphRecord( pack, "New Graph" );
			SetCurrentGraphRecord( record );
		}
	}
}
	
C_AnimationGraphEditor::~C_AnimationGraphEditor()
{
}

Xt::DomRecord* C_AnimationGraphEditor::NewGraphRecord( Xt::DomPack* pack, const char* name )
{
	return m_db.NewRecord( pack, name, m_graphTypeName );
}
	
Xt::DomRecord* C_AnimationGraphEditor::GetCurrentGraphRecord()
{
	return m_currentGraphRecord;
}
	
void C_AnimationGraphEditor::SetCurrentGraphRecord( Xt::DomRecord* record )
{
	m_currentGraphRecord = record;
	m_currentGraphPack = Xt::PackOf( record );
}
	
Xt::DomRecord* C_AnimationGraphEditor::FindGraphRecord( const char* name )
{
	return m_db.FindRecord( name, m_graphTypeName );
}
	
Xt::DomPack* C_AnimationGraphEditor::GetCurrentPack()
{
	return m_currentGraphPack;
}

Xt::DomField* NewXtDomField(const char* str)
{
	Xt::DomField* f = new Xt::DomField;
	f->value = str;
	return f;
}

Xt::DomBlock* C_AnimationGraphEditor::CreateCycleBlock( Xt::DomRecord* graphRecord, const char* cycleFilename )
{
	if ( !graphRecord ) return NULL;
	
	XtString path, file, type;
	bool ok = XtsSplitFileName( cycleFilename, path, file, type );
	if ( !ok ) return NULL;
	
	// Find node array in graph record.
	Xt::DomArray* nodeArray = Xt::AsArray( Xt::At( &graphRecord->root.data, m_nodeArrayName ) );
	if (nodeArray == NULL) return NULL;
	
	// Calculate bounds of node array in the node graph view.
	C_Bounds bounds;
	FindNodeGraphBounds( nodeArray, &bounds );
	
	F32 x = (bounds.maxX + bounds.minX) / 2.f;
	F32 y = (bounds.maxY + bounds.minY) / 2.f;
	
	// Create new cycle block 
	Xt::DomComponent* cycleComponent = new Xt::DomComponent;
	Xt::DomBlock* cycleBlock = &cycleComponent->data;
		
	cycleComponent->type = m_cycleTypeName;
	
	GenerateIdForComponent(cycleComponent, true);
	
	// Set node graph coordinates.
	Xt::Set(cycleBlock, "__x", Format( "%g", x ));
	Xt::Set(cycleBlock, "__y", Format( "%g", y ));
	
	// Set other defaults
	SetDefaults( m_db.GetDomDb()->schema, AsStruct(FindTypeByName( m_db.GetDomDb()->schema, m_cycleTypeName )), cycleComponent );
	
	// Set filename field.
	if ( Xt::DomField* filenameField = Xt::AsField( Xt::At( cycleBlock, "Filename" ) ) )
	{
		filenameField->value = cycleFilename;
	}
	
	// Add new block to node array.
	Xt::Append( nodeArray, cycleComponent );
	//Xt::AppendWithId( nodeArray, Xt::CreateUniqueArrayId( nodeArray ), cycleBlock );
	
	return cycleBlock;
	
}
	
void C_AnimationGraphEditor::OnXtDatabaseChanged( Xt::DomEvent* domEvent )
{
	if ( !domEvent ) return;
	
	switch (domEvent->code)
	{
		case Xt::kDomDbClosing:
			// save name of record
			m_currentGraphRecordName = m_currentGraphRecord ? m_currentGraphRecord->name : "";
			m_currentGraphRecord = NULL;
			m_currentGraphPack = NULL;
			break;
			
		case Xt::kDomDbOpened:
			// try and find previously selected record.
			SetCurrentGraphRecord( FindGraphRecord( m_currentGraphRecordName ) );
			break;
		
		case Xt::kDomRemoved:
			// Current record removed?
			if ( domEvent->from == m_currentGraphRecord )
			{
				m_currentGraphRecord = NULL;
				// Keep pack active.
			}
			break;
		
		default:
			break;
	}
}

void C_AnimationGraphEditor::FindNodeGraphBounds( Xt::DomArray* nodeArray, C_Bounds* outBounds ) const
{
	if ( !outBounds ) return;
	
	bool first = true;
	for (Xt::DomArrayElementIterator it(nodeArray); it; ++it)
	{
		if ( Xt::DomBlock* block = Xt::AsBlock( (Xt::DomElement*)it ) )
		{
			F32 x = static_cast<F32>(atof( Xt::AsText(Xt::At( block, "__x" ))));
			F32 y = static_cast<F32>(atof( Xt::AsText(Xt::At( block, "__y" ))));
			
			if ( first )
			{
				outBounds->minX = x;
				outBounds->maxX = x;
				outBounds->minY = y;
				outBounds->maxY = y;
				first = false;
			}
			else
			{
				outBounds->minX = Maths::Min( outBounds->minX, x );
				outBounds->maxX = Maths::Max( outBounds->maxX, x );
				outBounds->minY = Maths::Min( outBounds->minY, y );
				outBounds->maxY = Maths::Max( outBounds->maxY, y );
			}
		}
	}
}



