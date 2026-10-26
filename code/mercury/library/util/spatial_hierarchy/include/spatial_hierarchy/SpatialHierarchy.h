#ifndef SPATIAL_HIERACHY_H
#define SPATIAL_HIERACHY_H

#include "spatial_hierarchy/AlphaTextureInfo.h"
#include "spatial_hierarchy/Kd-Tree.h"
#include "spatial_hierarchy/Object.h"

typedef Ut::Vector<KdTree> KdTreeVector;
typedef Ut::Vector<ObjectPrim> ObjectVector;

namespace Maths
{
class HMat44;
};

namespace SD
{
class I_AlphaTextureManager;
class UserDataIterator;

/// A class for raytracing against an Aftk::C_Scene object.
/** Before use it needs to be initialized by calling the 'Build' function. **/
class SpatialHierarchy : protected KdTreeTemplate<ObjectPrim>
{
public:
	friend struct TriAccel;
	friend class UserDataIterator;

	typedef enum
	{
		AlwaysOn,
		AlwaysOff,
		Variable,
	} LayerMode;

	typedef bool SpatialHierarchyCallbackFunctionBool(Aftk::C_SceneNode *);
	typedef int SpatialHierarchyCallbackFunctionInt(Aftk::C_SceneNode *, void *pContext, int iTriangle);

			/// Default constructor
			SpatialHierarchy();
			/// Default destructor
			~SpatialHierarchy();


	/// Build a spatial hierarchy from a given Aftk::C_Scene
	/** pLayerModes specifies for each layer whether it is always on, always off, or switchable using SetLayer. **/
	void	Build(Aftk::C_Scene *pAftkScene, LayerMode *pLayerModes = NULL);
	
	/// Trace a single ray through the spatial hierarchy, result is returned in 'result'.
	void	RayTrace(const Ray& ray, RayTraceResult& result) const;

	/// Trace a batch of rays through the spatial hierarchy using SSE instructions. results are returned inside 'batch'.
	int		RayTrace(RayBatch &batch) const;




	/// Set a list of textures which will have their alpha channel checked.
	/** Rays will pass through polygons where one of these textures is applied, and the alpha of the texture at that position <0.5 **/
	void	SetAlphaTextureManager(I_AlphaTextureManager *pList);

			// functions for turning collision on and off
	/// Turn collision on or off for all objects in the specified layer.
	bool	SetLayer(const std::string &layerName, bool bOn);

	/// Turn collision on or off for all objects in the specified layer. (0 = all, 1 = layer 0, 2 = layer 1, etc.)
	bool	SetLayer(int i, bool bOn);

	/// Turn collision on or off for all layers using a bitfield
	void	SetLayerBitArray(const Ut::BitArray128 &iLayerData);

	/// set raytracing for a specific lod level
	/// set -1 or 0 to enable the highest lod. set 1 or 2 to disable the high-lod and raytrace against specified lower lods
	void	SetLodLevel(int iLodLevel);

	/// Turn collision on or off for a specific scene node (this only works on instances)
	int		GetInstanceFromAftkID(int iNodeInstance, int iHint);
	void	EnableInstance(int iID, bool bOn);

	/// If 'bIgnore' is set to true, instances will not be collided against.
	void	SetIgnoreInstances(bool bIgnore) const;

	/// Sets the epsilon value for the ray-triangle intersection code.
	void	SetEpsilon(float fEpsilon) const;

	/// Gets the epsilon value for the ray-triangle intersection code.
	float	GetEpsilon() const;

	/// Set callback function which is called by the spatial hierarchy 
	/** The callback function will be called by the Spatial Hierarchy class to determine whether a particular object
	    needs to be included in the collision tree. The callback function must be set before called 'Build' **/
	void	SetCallbackFunction(SpatialHierarchyCallbackFunctionBool *pCallbackFunction);

	/// Set callback function which is called by the spatial hierarchy 
	/** The callback function will be called by the Spatial Hierarchy class to determine whether a base instance
	    needs to be included in the collision tree. it is possible to include all it's instances without including the base **/
	void	SetBaseInstanceCallbackFunction(SpatialHierarchyCallbackFunctionBool *pCallbackFunction);

	/// Set callback function which is called by the spatial hierarchy 
	/** The callback function will be called by the Spatial Hierarchy class to determine whether a triangle
	    needs to be included in the collision tree. **/
	void	SetTriangleCallbackFunction(SpatialHierarchyCallbackFunctionInt *pCallbackFunction, void *pContext);

	/// The callback function is called for each triangle in the hierarchy. The returned int is stored in the hierarchy.
	void	SetPrimitiveUserData(SpatialHierarchyCallbackFunctionInt *pCallbackFunction, void *pContext);

	/// If a polygon's polyflag bits & _iPolyflagMask_ == TRUE, it will not be added to the collision tree.
	void	SetNoCollisionPolyflagMask(int iPolyflagMask);

	/// Get the bounding box for the entire scene.
   	const Box3	*GetBoundingBox() const { return &m_bbox; };

	/// Returns the number of rays that have been cast since the creation of the spatial hierarchy.
	int		GetNumRaysCast() const;

	/// Serialize a spatial hierarchy to an ost::Engine class for loading/saving.
	template<typename Archive>
	void	write(Archive &ar);
	template<typename Archive>
	void	read(Archive &ar);

	/// Change the unique ID of an object in the tree. This is needed when converting instances to remapped instances
	bool	ChangeUniqueID(int iNodeID, int iNewID);

private:
	int		GetAlphaTextureIndexFromName(const std::string &s);
	int		CollectPolygonsFromScene(Aftk::C_SceneNode *pAftkSceneNode, TriPrimVector &primitives, Box3 *psceneBBox, KdTree *pSH, bool bTransform, bool bCount);
	int		CheckIfPolyAssociatedWithMaterialNode(const Aftk::C_MaterialNode *pMat, KdTree *pSH);
	int		GetKdTreePtrForLayerIndex(S32 iLayer, int iLodLevel);
	bool	DoesNodeNeedProcessing(Aftk::C_SceneNode *pNode);
	bool	DoesBaseInstanceNeedProcessing(Aftk::C_SceneNode *pNode);
	bool	AddToHierarchyAsInstance(Aftk::C_SceneNode *pAftkSceneNode);
	/// update the active layer list using m_iCurLodLevel and m_CurActiveLayers
	void	UpdateActiveKdTrees();

public:
	// class to generate the alpha texture info
    I_AlphaTextureManager *m_pAlphaTextureMgr;
	// alpha texture info list
	Ut::Vector<AlphaTextureInfo> *m_pAlphaTextureList;

	KdTreeVector		m_MainKdTrees;			// a list of merged kd-trees, one per layer
	KdTreeVector		m_KdTrees;				// contains a list of kd-tree representations of instanced mesh
	ObjectVector		m_Objects;				// contains an index into m_KdTrees and a world matrix

	int					*m_piAlphaTex;			// temporary list of aplha tex indices (one per material)
	Ut::Vector<int>		m_iActiveMainKdTrees;	// list of indices of active main kd-trees
	Ut::Vector<bool>	m_pbLayer;				// on/off switch per scene layer index
	int					m_iCurLodLevel;			// current lod level to raytrace against
	int					m_nLayers;
	int					m_iGlobalPrimOffset;

	Aftk::C_Scene		*m_pAftkScene;
	mutable bool		m_bIgnoreInstances;

	int					m_iNoCollisionPolyflag	;

	bool m_bAllocatedBySerialization;
	SpatialHierarchyCallbackFunctionBool *m_pCallbackFunction;
	SpatialHierarchyCallbackFunctionBool *m_pBaseInstanceCallbackFunction;
	SpatialHierarchyCallbackFunctionInt  *m_pTriCallbackFunction;
	void *m_pTriContext;
	mutable int	 m_nRaysCast;

	mutable float				m_fEpsilon;
};

}

int GetModelLodLevel(Aftk::C_SceneNode *pNode);

#include "SpatialHierarchy.inl"

#endif
