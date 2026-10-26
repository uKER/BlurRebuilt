#ifndef RESOURCECACHE_H
#define RESOURCECACHE_H

#include <btl/containers/vector.h>
#include <animation/tools/Scene.h>
#include <animation/tools/CycleBuilder.h>

struct CycleResource;
struct SkeletonResource;
class C_SkinnedMesh;

namespace Animation
{
	class C_Skeleton;
	class C_Cycle;
}

namespace AnimationTools
{
	class C_SceneConfig;
	class C_CycleParameters;
	class C_ExportConstraints;
}

class I_Progress;

class C_ResourceCache
{
private:
	C_ResourceCache();
	~C_ResourceCache();
public:
	
	static C_ResourceCache& Instance();
	
	void SetSceneConfig( const AnimationTools::C_SceneConfig& sceneConfig );
	AnimationTools::C_SceneConfig& GetSceneConfig();
	const AnimationTools::C_SceneConfig& GetSceneConfig() const;
	void SetCycleParameters( const AnimationTools::C_CycleParameters& cycleParams );
	const AnimationTools::C_CycleParameters& GetCycleParameters() const;
	
	void SetWindowHandle( void* handle );

	const CHAR8* GetSkeletonFilename() const;

	const Animation::C_Skeleton* GetSkeleton( const Util::C_String& filename, const AnimationTools::C_SceneConfig& sceneConfig, AnimationTools::C_ExportConstraints* outConstraints = NULL, AnimationTools::C_Status* outStatus = NULL );
	
	const Animation::C_Cycle* GetCycle( const Util::C_String& filename, const AnimationTools::C_SceneConfig& sceneConfig, const AnimationTools::C_CycleParameters& cycleParameters, AnimationTools::C_Status* outStatus = NULL );
	
	const Animation::C_Cycle* GetAdditiveCycle( const Util::C_String& filename, const Util::C_String& subtractedFilename, const AnimationTools::C_SceneConfig& sceneConfig, const AnimationTools::C_CycleParameters& cycleParameters, AnimationTools::C_Status* outStatus = NULL );
	
	void ReloadAll();
	void ReloadChangedFiles();
	
	void ClearReloadFlag()			{ m_reloadFlag = false; }
	bool GetReloadFlag() const		{ return m_reloadFlag; }
	
	bool SkeletonLoaded() const		{ return m_skeletonResource != NULL; }
	
	
	C_SkinnedMesh* GetSkinnedMesh() const;
		
	void Purge( bool forceAll = false );

private:

	struct LoadCycleContext
	{
		Util::C_String filename;
		Util::C_String subtractedFilename;
		CycleResource* resource;
		C_ResourceCache* cache;
		AnimationTools::C_SceneConfig sceneConfig;
		AnimationTools::C_CycleParameters cycleParams;
		AnimationTools::C_Status status;
	};

	struct LoadSkeletonContext
	{
		Util::C_String filename;
		SkeletonResource* resource;
		C_ResourceCache* cache;
		AnimationTools::C_SceneConfig sceneConfig;
		AnimationTools::C_Status status;
	};

	static bool LoadSkeleton(I_Progress* progress, LoadSkeletonContext* context);
	static bool LoadCycle(I_Progress* progress, LoadCycleContext* context);

	template <typename T>
	struct AutoDeleteVector: public Btl::Vector<T>
	{
		~AutoDeleteVector()
		{
			for ( Iterator it(Begin()); it != End(); ++it )
			{
				delete *it;
			}
		}
	};
	
	AutoDeleteVector<CycleResource*> m_cycleResources;
	typedef AutoDeleteVector<CycleResource*>::Iterator CycleResourceIterator;

	SkeletonResource* m_skeletonResource;
	AnimationTools::C_CycleParameters*		m_cycleParameters;
	AnimationTools::C_SceneConfig*			m_sceneConfig;
	C_SkinnedMesh* m_skinnedMesh;
	U32 m_purgeCounter;
		
	void* m_windowHandle;
	
	bool m_reloadFlag;
};

bool IsGameCycleFilename(const Util::C_String& filename);
bool IsGameSkeletonFilename(const Util::C_String& filename);



#endif
