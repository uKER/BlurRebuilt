#include "stdafx.h"
#include "ResourceCache.h"
#include <system/serialise/C_MemoryStream.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/BinarySerialise.h>
#include <animation/cycle/Cycle.h>
#include <animation/cycle/CycleDeserialise.inl>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/SkeletonDeserialise.inl>
#include <animation/tools/CycleBuilder.h>
#include <animation/tools/Scene.h>
#include <animation/tools/ExportConstraints.h>
#include <animation/tools/Status.h>
#include <system/misc/StringUtil.h>
#include "SkinnedMesh.h"
#include <xtwin/AsyncOpWin.h>
#include "Progress.h"

namespace
{

U64 GetSimpleFileTime( const char* filename )
{
	Sys::C_FileInfo fileInfo;
	U64 fileTime = 0;
	if ( Sys::C_FileSystem::Instance().FileInfo( filename, &fileInfo ) )
	{
		fileTime = fileInfo.m_FileTime;
	}
	return fileTime;
}


class C_AftkProgress : public Aftk::I_Progress
{
public:
	C_AftkProgress(Progress::I_ProgressBar* pProgressBar, const Util::C_String& desc = "AftkProgress" ) : m_ProgressBar(pProgressBar, desc)
	{
	}

	virtual ~C_AftkProgress()
	{
	}

	// Inherited from AftkProgressNotification
	virtual bool OnProgress(F32 percent)
	{
		//m_pProgressBar->ProgressNotify( 0, percent, "AftkProgress" );
		m_ProgressBar.SetProgress(percent);
		return false;
	}

	//Callback function for printing output to the user while they wait.
	virtual void OnPrintProgress(const std::string& text) 
	{
		//LOGDEBUG( Tools::g_log, "%s\n", text.c_str());
	}

	virtual void OnPrintWarning(const std::string& text)
	{
		//LOGWARN( Tools::g_log, "%s\n", text.c_str());
	}

	virtual void OnPrintError(const std::string& text)
	{
		//LOGFATAL( Tools::g_log, "%s\n", text.c_str());
	}

private:
	Progress::UpdateSimple			m_ProgressBar;
};

AnimationTools::C_Status LoadSceneAndBuildSkeleton(Aftk::C_Scene& aftkScene,
							  const char* filename,
							  Animation::C_Skeleton& outSkeleton,
							  AnimationTools::C_Scene& scene,
							  const AnimationTools::C_SceneConfig& sceneConfig,
							  Progress::I_ProgressBar* progressBar
							  )
{
	C_AftkProgress aftkProgress( progressBar, "Loading Aftk Scene..." );
	Aftk::C_Status readStatus = aftkScene.Read(filename, Aftk::E_FileType_BestGuess, NULL, &aftkProgress );

	if (!readStatus)
	{
		return AnimationTools::C_Status(false, readStatus.GetErrorString().c_str());
	}
	
	AnimationTools::C_Status status = scene.Import( aftkScene, sceneConfig );
	if (!status)
	{
		return status;
	}
	
	bool endianSwap = true;
	Sys::C_MemoryStream outMemoryStream( 4096 );
	outMemoryStream.SetEndianSwapFlag( endianSwap );
	status = AnimationTools::SerialiseSkeleton( scene, outMemoryStream );
	
	if ( !status )
	{
		return status;
	}
	
	Sys::C_MemoryStream inMemoryStream( outMemoryStream.GetBufferPtr(), static_cast< U32 >( outMemoryStream.GetBufferSize() ) );
	inMemoryStream.SetEndianSwapFlag( endianSwap );
	AnimationTools::DeserialiseSkeleton( outSkeleton, inMemoryStream );
	return AnimationTools::C_Status(true);
}

void BuildGameCycle(
	const AnimationTools::C_CycleBuilder& cycleBuilder,
	Animation::C_Cycle& gameCycle
)
{
	Sys::C_MemoryStream outMemoryStream( 4096 );
	outMemoryStream.SetEndianSwapFlag( true );
	cycleBuilder.Serialise( outMemoryStream );
	Sys::C_MemoryStream inMemoryStream( outMemoryStream.GetBufferPtr(), static_cast< U32 >( outMemoryStream.GetBufferSize() ) );
	inMemoryStream.SetEndianSwapFlag( true );
	
	Sys::BinaryDeserialiserAutoEndian in( inMemoryStream );
	in >> gameCycle;
}

AnimationTools::C_Status LoadScene( Aftk::C_Scene& scene, const char* filename, Progress::I_ProgressBar* progressBar = NULL )
{
	C_AftkProgress aftkProgress( progressBar, "Loading Aftk Scene..." );
	Aftk::C_Status readStatus = scene.Read(filename, Aftk::E_FileType_BestGuess, NULL, &aftkProgress );

	if ( !readStatus )
	{
		//LOGERROR("Error importing clip '%s'", filename);
		return AnimationTools::C_Status(false, readStatus.GetErrorString().c_str());
	}

	// Remove Maya Z-flip.
	Aftk::Matrix44 identityWorldMatrix;
	UtMaths::SetIdentity(&identityWorldMatrix);
	scene.GetRootSceneNodePtr()->SetWorldMatrix(identityWorldMatrix);
	
	return AnimationTools::C_Status(true);
}


}

struct CycleResource
{
	CycleResource():
		additive(false)
	{
	}
	
	AnimationTools::C_SceneConfig		sceneConfig;
	AnimationTools::C_CycleParameters	cycleParams;
	
	Util::C_String 					filename;
	Util::C_String 					subtractedFilename;
	Animation::C_Cycle				cycle;
	AnimationTools::C_CycleBuilder	cycleBuilder;
	AnimationTools::C_CycleBuilder	subtractedCycleBuilder;
	U32								purgeCounter;
	U64								timeStamp;
	U64								subTimeStamp;
	bool							additive;
};

struct SkeletonResource
{
	SkeletonResource()
	{
	}
	
	AnimationTools::C_SceneConfig	sceneConfig;
	Util::C_String					filename;
	Animation::C_Skeleton			skeleton;
	AnimationTools::C_ExportConstraints	constraints;
	U32								purgeCounter;
	U64								timeStamp;
};

//
// C_ResourceCache
//

C_ResourceCache::C_ResourceCache():
	m_skeletonResource(NULL),
	m_cycleParameters( new AnimationTools::C_CycleParameters ),
	m_sceneConfig( new AnimationTools::C_SceneConfig ),
	m_skinnedMesh( new C_SkinnedMesh ),
	m_purgeCounter(1)
{
}

C_ResourceCache::~C_ResourceCache()
{
	delete m_skinnedMesh;
	delete m_sceneConfig;
	delete m_cycleParameters;
	delete m_skeletonResource;
}

C_ResourceCache& C_ResourceCache::Instance()
{
	static C_ResourceCache rc;
	return rc;
}

void C_ResourceCache::SetSceneConfig( const AnimationTools::C_SceneConfig& sceneConfig )
{
	*m_sceneConfig = sceneConfig;
}

AnimationTools::C_SceneConfig& C_ResourceCache::GetSceneConfig()
{
	return *m_sceneConfig;
}

const AnimationTools::C_SceneConfig& C_ResourceCache::GetSceneConfig() const
{
	return *m_sceneConfig;
}

void C_ResourceCache::SetCycleParameters( const AnimationTools::C_CycleParameters& cycleParams )
{
	*m_cycleParameters = cycleParams;
}

const AnimationTools::C_CycleParameters& C_ResourceCache::GetCycleParameters() const
{
	return *m_cycleParameters;
}

void C_ResourceCache::SetWindowHandle( void* handle )
{
	m_windowHandle = handle;
}

const CHAR8* C_ResourceCache::GetSkeletonFilename() const
{
	if (m_skeletonResource)
	{
		return m_skeletonResource->filename.Chars();
	}
	return "";
}

const Animation::C_Skeleton* C_ResourceCache::GetSkeleton( const Util::C_String& filename, const AnimationTools::C_SceneConfig& sceneConfig, AnimationTools::C_ExportConstraints* outConstraints, AnimationTools::C_Status* outStatus )
{
	SkeletonResource* sr = NULL;
	if ( m_skeletonResource )
	{
		if ( m_skeletonResource->filename == filename ) // && m_skeletonResource->timeStamp == fileTime )
		{
			sr = m_skeletonResource;
			
			if ( m_skeletonResource->sceneConfig == sceneConfig )
			{
				m_skeletonResource->purgeCounter = m_purgeCounter;
				
				if ( outConstraints != NULL )
					*outConstraints = m_skeletonResource->constraints;
					
	
				if (outStatus != NULL)
				{
					*outStatus = AnimationTools::C_Status(true);
				}					
					
				return &m_skeletonResource->skeleton;
			}
		}
		else
		{
			delete m_skeletonResource;
			m_skeletonResource = NULL;
		}
	}
	
	m_reloadFlag = true;
	
	LoadSkeletonContext c;
	c.cache = this;
	c.filename = filename;
	c.resource = sr;
	c.sceneConfig = sceneConfig;
	
	bool ok = RunLongTask(m_windowHandle, &LoadSkeleton, &c);
	
	m_skeletonResource = c.resource;
	
	if ( ok && outConstraints != NULL )
	{
		*outConstraints = c.resource->constraints;
	}
	
	if (outStatus != NULL)
	{
		*outStatus = c.status;
	}
	
	return ok ? &c.resource->skeleton : NULL;
}


const Animation::C_Cycle* C_ResourceCache::GetCycle( const Util::C_String& filename, const AnimationTools::C_SceneConfig& sceneConfig, const AnimationTools::C_CycleParameters& cycleParams, AnimationTools::C_Status* outStatus )
{
	CycleResource* cr = NULL;
	for ( CycleResourceIterator it(m_cycleResources.Begin()); it != m_cycleResources.End(); ++it )
	{
		if ( (*it)->filename == filename && !(*it)->additive )
		{
//			U32 fileTime = GetSimpleFileTime( filename.Chars() );
		
			// found a match, but does it need reloading?
			cr = *it;
			if ( cr->sceneConfig == sceneConfig && cr->cycleParams == cycleParams ) // && cr->timeStamp == fileTime )
			{
				// No...
				(*it)->purgeCounter = m_purgeCounter;
				return &(*it)->cycle;
			}
			break;
		}
	}
	m_reloadFlag = true;
	Debug::Logger& gLog = Debug::GetLogger("mercury.animation");
	if( cr == NULL )
	{
		LOGINFO( gLog, "*** Loading %s: not in cache ***", filename.Chars() );
	}
	else if ( !(cr->sceneConfig == sceneConfig) )
	{
		LOGINFO( gLog, "*** Loading %s: different scene config ***", filename.Chars() );
	}
	else if ( !(cr->cycleParams == cycleParams) )
	{
		LOGINFO( gLog, "*** Loading %s: different cycle params ***", filename.Chars() );
	}
	
	LoadCycleContext c;
	c.cache = this;
	c.filename = filename;
	c.resource = cr;	
	c.sceneConfig = sceneConfig;
	c.cycleParams = cycleParams;
	bool ok = RunLongTask(m_windowHandle, &LoadCycle, &c);
	
	if (outStatus != NULL)
	{
		*outStatus = c.status;
	}
	
	return ok ? &c.resource->cycle : NULL;
}

const Animation::C_Cycle* C_ResourceCache::GetAdditiveCycle( const Util::C_String& filename, const Util::C_String& subtractedFilename, const AnimationTools::C_SceneConfig& sceneConfig, const AnimationTools::C_CycleParameters& cycleParams, AnimationTools::C_Status* outStatus )
{
	CycleResource* cr = NULL;
	for ( CycleResourceIterator it(m_cycleResources.Begin()); it != m_cycleResources.End(); ++it )
	{
		if ( (*it)->filename == filename && (*it)->subtractedFilename == subtractedFilename && (*it)->additive )
		{

			// found a match, but does it need reloading?
			cr = *it;
			if ( cr->sceneConfig == sceneConfig && cr->cycleParams == cycleParams ) // && cr->timeStamp == fileTime )
			{
				// No...
				(*it)->purgeCounter = m_purgeCounter;
				return &(*it)->cycle;
			}
			break;
		}
	}
	m_reloadFlag = true;
	
	Debug::Logger& gLog = Debug::GetLogger("mercury.animation");
	if( cr == NULL )
	{
		LOGINFO( gLog, "*** Loading %s: not in cache ***", filename.Chars() );
	}
	else if ( !(cr->sceneConfig == sceneConfig) )
	{
		LOGINFO( gLog, "*** Loading %s: different scene config ***", filename.Chars() );
	}
	else if ( !(cr->cycleParams == cycleParams) )
	{
		LOGINFO( gLog, "*** Loading %s: different cycle params ***", filename.Chars() );
	}
	
	LoadCycleContext c;
	c.cache = this;
	c.filename = filename;
	c.subtractedFilename = subtractedFilename;
	c.resource = cr;	
	c.sceneConfig = sceneConfig;
	c.cycleParams = cycleParams;
	bool ok = RunLongTask(m_windowHandle, &LoadCycle, &c);
	
	if (outStatus != NULL)
	{
		*outStatus = c.status;
	}
	
	return ok ? &c.resource->cycle : NULL;

/*


	CycleResource* cr = NULL;

	for ( CycleResourceIterator it(m_cycleResources.Begin()); it != m_cycleResources.End(); ++it )
	{
		if ( (*it)->filename == filename && (*it)->subtractedFilename == subtractedFilename && (*it)->additive )
		{
			// found a match, but does it need reloading?
			//U32 fileTime = GetSimpleFileTime( filename.Chars() );
			cr = *it;
			if ( cr->sceneConfig == sceneConfig && cr->cycleParams == cycleParams ) // && cr->timeStamp == fileTime )
			{
				(*it)->purgeCounter = m_purgeCounter;
				return &(*it)->cycle;
			}
			break;
		}
	}
	
	Aftk::C_Scene aftkScene1;
	Aftk::C_Scene aftkScene2;
	
	if ( !LoadScene( aftkScene1, filename ) )
	{
		return false;
	}
	
	if ( !LoadScene( aftkScene2, subtractedFilename ) )
	{
		return false;
	}
		
	AnimationTools::C_Scene scene1;
	AnimationTools::C_Scene scene2;
	if ( !scene1.Import( aftkScene1, sceneConfig ) )
	{
		return false;
	}
	
	if ( !scene2.Import( aftkScene2, sceneConfig ) )
	{
		return false;
	}
	
	bool madeNew = false;
	if ( NULL == cr )
	{
		cr = new CycleResource;
		madeNew = true;
	}
	
	cr->filename = filename;
	cr->subtractedFilename = subtractedFilename;
	cr->sceneConfig = sceneConfig;
	cr->cycleParams = cycleParams;
	cr->purgeCounter = m_purgeCounter;
	cr->timeStamp = GetSimpleFileTime( filename.Chars() );
	cr->subTimeStamp = GetSimpleFileTime( subtractedFilename.Chars() ):
	cr->additive = true;
		
	bool cycleOk = cr->cycleBuilder.BuildAdditiveCycle(
		scene1,
		scene2,
		cycleParams
	);
	
	if ( !cycleOk )
	{
		if ( madeNew )
			delete cr;
		return NULL;
	}	
	
	BuildGameCycle( cr->cycleBuilder, cr->cycle );
	
	Util::C_String dir;
	Util::C_String fn;
	Util::C_String ext;
	Util::DeconstructFileString( filename, dir, fn, ext );
	cr->cycle.SetName( fn );
	
	if ( madeNew )
	{
		m_cycleResources.PushBack( cr );
	}
	
	return &cr->cycle;
*/
}

void C_ResourceCache::ReloadAll()
{
	for ( CycleResourceIterator it(m_cycleResources.Begin()); it != m_cycleResources.End(); ++it )
	{
		Util::C_String path = (*it)->filename;
		
		if ( (*it)->additive )
		{
			m_reloadFlag = true;
			LoadCycleContext c;
			c.cache = this;
			c.filename = (*it)->filename;
			c.subtractedFilename = (*it)->subtractedFilename;
			c.resource = *it;
			c.sceneConfig = (*it)->sceneConfig;
			c.cycleParams = (*it)->cycleParams;
			bool ok = RunLongTask(m_windowHandle, &LoadCycle, &c);
		}
		else
		{
			m_reloadFlag = true;
			LoadCycleContext c;
			c.cache = this;
			c.filename = (*it)->filename;
			c.resource = *it;
			c.sceneConfig = (*it)->sceneConfig;
			c.cycleParams = (*it)->cycleParams;
			bool ok = RunLongTask(m_windowHandle, &LoadCycle, &c);
		}
	}	
	
	// Dangerous - caller needs to call GetSkeleton() to update constraints list.
	if ( m_skeletonResource )
	{
		m_reloadFlag = true;
		LoadSkeletonContext c;
		c.cache = this;
		c.filename = m_skeletonResource->filename;
		c.resource = m_skeletonResource;
		c.sceneConfig = m_skeletonResource->sceneConfig;
		
		bool ok = RunLongTask(m_windowHandle, &LoadSkeleton, &c);
		
		m_skeletonResource = c.resource;
	}
}

void C_ResourceCache::ReloadChangedFiles()
{
	for ( CycleResourceIterator it(m_cycleResources.Begin()); it != m_cycleResources.End(); ++it )
	{
		Util::C_String path = (*it)->filename;
		
		if ( (*it)->additive )
		{
			U64 oldFileTime = (*it)->timeStamp;
			U64 newFileTime = GetSimpleFileTime( path.Chars() );
			U64 oldSubFileTime = (*it)->subTimeStamp;
			U64 newSubFileTime = GetSimpleFileTime( (*it)->subtractedFilename.Chars() );
			
			if ( newFileTime != oldFileTime || newSubFileTime != oldSubFileTime )
			{
				m_reloadFlag = true;
				LoadCycleContext c;
				c.cache = this;
				c.filename = (*it)->filename;
				c.subtractedFilename = (*it)->subtractedFilename;
				c.resource = *it;
				c.sceneConfig = (*it)->sceneConfig;
				c.cycleParams = (*it)->cycleParams;
				bool ok = RunLongTask(m_windowHandle, &LoadCycle, &c);
			}
		}
		else
		{
			U64 oldFileTime = (*it)->timeStamp;
			U64 newFileTime = GetSimpleFileTime( path.Chars() );
			
			if ( newFileTime != oldFileTime )
			{
				m_reloadFlag = true;
				LoadCycleContext c;
				c.cache = this;
				c.filename = (*it)->filename;
				c.resource = *it;
				c.sceneConfig = (*it)->sceneConfig;
				c.cycleParams = (*it)->cycleParams;
				bool ok = RunLongTask(m_windowHandle, &LoadCycle, &c);
			}
		}
	}	
	
	if ( m_skeletonResource )
	{
		U64 fileTime = GetSimpleFileTime( m_skeletonResource->filename.Chars() );
	
		if ( m_skeletonResource->timeStamp != fileTime )
		{
			m_reloadFlag = true;
			LoadSkeletonContext c;
			c.cache = this;
			c.filename = m_skeletonResource->filename;
			c.resource = m_skeletonResource;
			c.sceneConfig = m_skeletonResource->sceneConfig;
			
			bool ok = RunLongTask(m_windowHandle, &LoadSkeleton, &c);
			
			m_skeletonResource = c.resource;
		}
	}
}

C_SkinnedMesh* C_ResourceCache::GetSkinnedMesh() const
{
	return m_skinnedMesh;
}

void C_ResourceCache::Purge( bool forceAll )
{
	// Delete non-referenced resources.
	CycleResourceIterator it(m_cycleResources.Begin());
	while ( it != m_cycleResources.End() )
	{
		if ( (*it)->purgeCounter != m_purgeCounter || forceAll )
		{
			LOGINFO( Debug::GetLogger("motionworks"), "Purged '%s'", (*it)->filename.Chars() );
			delete *it;
			*it = NULL;
			it = m_cycleResources.Erase( it );
			m_reloadFlag = true; 
		}
		else
		{
			++it;
		}
	}
	
	++m_purgeCounter;
}

// Not pretty, but by relying on the fact that version numbers are less than 65536, it's currently
// possible to auto-detect the endian-ness of a cycle file.
static bool NeedToEndianSwapCycleFile(Sys::I_File* pFile)
{
	U32 test = 0;

	{
		Sys::BinaryDeserialiser< Ut::NoEndianSwap > ar(*pFile);
		ar >> test;
	}
	
	pFile->SetPosition(0);
	
	if (0 != (test & 0x0000ffff))
	{
		// Non-zero low part means that there is no need to endian swap.
		return false;
	}
	
	return true;
}

// Even less pretty for skeleton files as they have 16-bit version numbers, so we check
// to see which byte if non-zero... won't work if version goes above 255 :)
static bool NeedToEndianSwapSkeletonFile(Sys::I_File* pFile)
{
	U16 test = 0;

	{
		Sys::BinaryDeserialiser< Ut::NoEndianSwap > ar(*pFile);
		ar >> test;
	}
	
	pFile->SetPosition(0);
	
	if (0 != (test & 0x00ff))
	{
		// Non-zero low part means that there is no need to endian swap.
		return false;
	}
	
	return true;
}

bool IsGameCycleFilename(const Util::C_String& filename)
{
	return Util::ExtractFileExtension(filename).Lower() == "cycle";
}

bool IsGameSkeletonFilename(const Util::C_String& filename)
{
	return Util::ExtractFileExtension(filename).Lower() == "skeleton";
}

bool C_ResourceCache::LoadCycle(I_Progress* progress, LoadCycleContext* context)
{
	C_ResourceCache* self = context->cache;
	
	CycleResource* cr = context->resource;
	
	bool isGameCycle = IsGameCycleFilename(context->filename);
	
	if (isGameCycle)
	{
		// Load the cycle file.
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(context->filename);

		if( !pFile )
		{
			progress->ErrorMessage( "Error opening cycle file" );
			return false;
		}
		
		if ( NULL == cr )
		{
			cr = new CycleResource;
		}

		cr->filename = context->filename;
		cr->subtractedFilename.Clear();
		cr->additive = false;
		cr->purgeCounter = self->m_purgeCounter;
		cr->sceneConfig = context->sceneConfig;
		cr->cycleParams = context->cycleParams;
		cr->timeStamp = GetSimpleFileTime( context->filename.Chars() );

		// Read data.
		bool success = true;
		{
			pFile->SetEndianSwapFlag(NeedToEndianSwapCycleFile(pFile));
			Sys::BinaryDeserialiserAutoEndian ar(*pFile);
			success = cr->cycle.Deserialise(ar);
		}

		delete pFile;
		pFile = NULL;
		
		if (!success)
		{
			progress->ErrorMessage("Error reading game cycle file");
			return false;
		}
	}
	else
	{
		// Cycle is being loaded from an Aftk readable scene.
	
		Aftk::C_Scene aftkScene1;
		Aftk::C_Scene aftkScene2;
		
		progress->SetTitle( wxString::Format( "Loading cycle '%s'", Util::ExtractFilename(context->filename).Chars() ) );
		//progress->SetMessage( wxString::Format("Loading scene '%s'", context->filename.Chars()).c_str() );
		
		context->status = LoadScene( aftkScene1, context->filename.Chars(), progress );
		
		if ( !context->status )
		{
			progress->ErrorMessage( "Error loading scene" );
			//context->resource = NULL;
			return false;
		}
		
		const bool additive = context->subtractedFilename.Length() > 0;
		
		if ( additive )
		{
			context->status = LoadScene( aftkScene2, context->subtractedFilename.Chars(), progress );
			if ( !context->status )
			{
				progress->ErrorMessage( "Error loading scene" );
				return false;
			}
		}
		
		AnimationTools::C_Scene scene1;
		AnimationTools::C_Scene scene2;
		context->status = scene1.Import( aftkScene1, context->sceneConfig );
		if ( !context->status )
		{
			progress->ErrorMessage( context->status.GetErrorMessage().Chars() );
			return false;
		}
		
		if ( additive )
		{
			context->status = scene2.Import( aftkScene2, context->sceneConfig );
			if ( !context->status )
			{
				progress->ErrorMessage( context->status.GetErrorMessage().Chars() );
				return false;
			}
		}
		
				
		if ( NULL == cr )
		{
			cr = new CycleResource;
		}

		cr->filename = context->filename;
		cr->subtractedFilename = context->subtractedFilename;
		cr->additive = additive;
		cr->purgeCounter = self->m_purgeCounter;
		cr->sceneConfig = context->sceneConfig;
		cr->cycleParams = context->cycleParams;
		cr->timeStamp = GetSimpleFileTime( context->filename.Chars() );
		if ( additive )
		{
			cr->subTimeStamp = GetSimpleFileTime( context->subtractedFilename.Chars() );
		}
		
		if ( additive )
		{
			context->status = cr->cycleBuilder.BuildAdditiveCycle( scene1, scene2, context->cycleParams, progress );
		}
		else
		{
			context->status = cr->cycleBuilder.BuildCycle( scene1, context->cycleParams, progress );
		}
		
		if ( !context->status )
		{
			if ( context->resource == NULL )
			{
				delete cr;
			}
			progress->ErrorMessage( context->status.GetErrorMessage().Chars() );
			return false;
		}
		
		BuildGameCycle( cr->cycleBuilder, cr->cycle );
	}
	
	if ( context->resource == NULL )
	{
		self->m_cycleResources.PushBack( cr );
	}
	
	Util::C_String dir;
	Util::C_String fn;
	Util::C_String ext;
	Util::DeconstructFileString( context->filename, dir, fn, ext );
	cr->cycle.SetName( fn );
	
	context->resource = cr;
	
	return true;
}

bool C_ResourceCache::LoadSkeleton(I_Progress* progress, LoadSkeletonContext* context)
{
	context->resource = NULL;
	
	progress->SetTitle( Util::C_String::Format( "Loading Skeleton '%s'", Util::ExtractFilename( context->filename ).Chars() ) );
	
	SkeletonResource* s = context->resource;
	if ( s == NULL )
	{
		s = new SkeletonResource;
	}
	
	C_ResourceCache* self = context->cache;
	
	s->constraints.Clear();
	
	bool isGameSkeleton = IsGameSkeletonFilename(context->filename);
	
	if (isGameSkeleton)
	{
		// Load the skeleton file.
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(context->filename);

		if( !pFile )
		{
			progress->ErrorMessage( "Error opening skeleton file" );
			return false;
		}
		

		// Read data.
		bool success = true;
		{
			pFile->SetEndianSwapFlag(NeedToEndianSwapSkeletonFile(pFile));
			Sys::BinaryDeserialiserAutoEndian ar(*pFile);
			success = s->skeleton.Deserialise(ar);
		}

		delete pFile;
		pFile = NULL;
		
		if (!success)
		{
			progress->ErrorMessage("Error reading game skeleton file");
			return false;
		}
		
		context->status = AnimationTools::C_Status(true);
	}
	else
	{
		Aftk::C_Scene aftkScene;
		AnimationTools::C_Scene scene;
		context->status = LoadSceneAndBuildSkeleton( aftkScene, context->filename, s->skeleton, scene, *self->m_sceneConfig, progress );
		
		if ( context->status )
		{
			// Create skinned mesh from skeleton.
			self->m_skinnedMesh->Create( aftkScene, scene, s->skeleton, progress );
		}
		else
		{
			if ( context->resource == NULL )
			{
				delete s;
				s = NULL;
			}
			return false;
		}
		
		s->constraints = scene.GetConstraints();
	}
	
	context->resource = s;
	s->filename = context->filename;
	s->purgeCounter = self->m_purgeCounter;
	s->sceneConfig = context->sceneConfig;
	s->timeStamp = GetSimpleFileTime( context->filename.Chars() );
	
	Util::C_String dir;
	Util::C_String fn;
	Util::C_String ext;
	Util::DeconstructFileString( context->filename, dir, fn, ext );
	s->skeleton.SetName( fn );
		
	return true;
}
