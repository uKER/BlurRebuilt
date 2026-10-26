#pragma once

#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/SpatialHierarchy.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/Kd-Tree.h"

// some forward declarations
struct SIMDRay;
struct SIMDResult;
class KdBuilder;
namespace LUX
{
class Light;
}
#define NUM_LIGHT_SETS 9

// maximum recursive depth of the kd-tree tracing code
#define KDNODE_STACK_SIZE 128

// some macros to save us typing so much
#define KDNODE_ISLEAF(n)	(n->iFlagDimAndOffset & (unsigned int)(1<<31))
#define KDNODE_DIMENSION(n) (n->iFlagDimAndOffset & 0x3)
#define KDNODE_OFFSET(n)	(n->iFlagDimAndOffset & (0x7FFFFFFC))
#define KDPTR_OFFSET(X,I)	((KdNode*)( ((char*)(X)) + (I)))

typedef Ut::Vector<int> IntVector;
typedef Ut::Vector<LUX::Light*> LightPtrVector;

struct LightKdStack
{
	KdNode	*pNode;
	Box3	bbox;
};

class LUX::Light;

class LightPrim : public Primitive
{
public:
					LightPrim(int iLight, const Box3 &bbox);
	bool			Intersect(const SD::Ray &,SD::RayTraceResult &);
	virtual Box3	&WorldBound() { return m_bbox; }

	Box3	m_bbox;
	int		m_iLightSet;
	int		m_iLight;
};

class LightTree
{
public:
				LightTree();
				~LightTree();

	void		AddLight(LUX::Light *pLight, const Box3 &bbox);
	void		Build();
	void		GetLights(const Box3 &bbox);
	void		AddLeafNode(int iPrimitive);
	void		Trace(const Box3 &bbox);

	int					m_nNodes;
	KdNode				*m_pNodes;
	Box3				m_bbox;
	mutable int			m_iCurMailBox;

	KdBuilder			*m_pBuilder;
	PrimVector			m_Primitives;
	MailboxVector		*m_pMailboxes;
	int					*m_pRealMailBoxes;

	LightPtrVector		m_LocalLights;	// master list of lights referenced by LightPrim by index

	// 'real' light list
	LightPtrVector		m_LightPtrVec;	// current list of local lights
};

bool TestIntersectHemisphereVsBBox(const Float3 &vPos, float fRadius, const Float3 &vNormal, Box3 &bbox);
