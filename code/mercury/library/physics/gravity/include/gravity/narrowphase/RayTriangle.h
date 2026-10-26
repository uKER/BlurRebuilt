#ifndef RAYTRIANGLE_H
#define RAYTRIANGLE_H

#include "gravity/narrowphase/Ray.h"

namespace Collision
{
//template <class Triangle>
//bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high);

template <class Triangle>
bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
							const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high);


template <class Triangle>
bool CollideFatRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
							   const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high);

bool IntersectRayPlane(Collision::C_Ray& ray, Collision::C_Shape* pShape);

template <class Triangle>
bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
							const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
							const Maths::HVec3& low, const Maths::HVec3& high);
template <class Triangle>
bool CollideFatRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
							   const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
							   const Maths::HVec3& low, const Maths::HVec3& high);

template <class Triangle>
bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
							const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
							const Maths::HVec3& low, const Maths::HVec3& high, const Maths::HMat43& matrix);
template <class Triangle>
bool CollideFatRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
							   const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
							   const Maths::HVec3& low, const Maths::HVec3& high, const Maths::HMat43& matrix);

bool IntersectRayPlane(Collision::C_TransparentRay& ray, Collision::C_Shape* pShape);

};

#endif
