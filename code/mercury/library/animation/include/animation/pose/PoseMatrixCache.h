#ifndef ANIMATION_POSEMATRIXCACHE_H
#define ANIMATION_POSEMATRIXCACHE_H

namespace Animation
{

class C_Pose;
class C_Skeleton;
class C_JointTransform;

/**
Caches model-space matrices derived from the state of a C_Pose.
*/
class ALIGN16 C_PoseMatrixCache
{
public:
	/// Construct an empty C_PoseMatrixCache that can be initialised later using Create().
	C_PoseMatrixCache();
	
	/// Construct a C_PoseMatrixCache for @a pose or any other pose that shares the same C_PoseFormat.
	C_PoseMatrixCache(const C_Pose& pose);
	
	/// Destructor.
	~C_PoseMatrixCache(); 
	
	//
	// Initialisation.
	//
		
	/// Create a C_PoseMatrixCache for @a pose or any other pose that shares the same C_PoseFormat.
	void Create(const C_Pose& pose);
	
	/// Destroy the matrix cache, releasing all allocated memory.
	void Destroy();
	
	//
	// SPU DMA.
	//
	void SpuFetch();
	
	//
	// Update.
	//
	
	/// Update the matrix cache from the current state in @a pose, optionally using the specified skeleton
	/// if @a pose has a local-space joint hierarchy.
	/// On SPU, this must be preceded by a call to SpuAlloc() and followed by a call to SpuStore() or
	/// SpuStoreAsync() to write the updated matrices back to main memory.
	void Update(const C_Pose& pose, const C_Skeleton* skeleton);

	//
	// Read-only access.
	//
	
	/// Returns a pointer to the first skinning matrix.
	const Maths::Mat43*		GetSkinningMatrices() const;
	
	
private:
	void Resize(U32 numMatrices);
	void BuildModelSpaceSkinningMatrices(const C_Pose& pose, const C_Skeleton* skeleton, Maths::Mat43* destMatrices);

	U32								m_maxMatrices;
	U32								m_numMatrices;
	Maths::Mat43*					m_matrices;
	Animation::C_JointTransform*	m_tempJointTransforms;
	U32								m_numSkinningMatrices;
};

} // end Animation

#endif
