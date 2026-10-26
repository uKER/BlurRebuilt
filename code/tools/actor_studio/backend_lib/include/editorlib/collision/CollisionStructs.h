#ifndef EDITOR_LIB_COLLISION_STRUCTS_H
#define EDITOR_LIB_COLLISION_STRUCTS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Misc/Definitions.h>

namespace EditorLib
{
	static U32 kCollisionSourceType_MaskGeometry	= 0x0000000f;
	static U32 kCollisionSourceType_MaskFaces		= 0xc0000000;
	static U32 kCollisionSourceType_MaskAll			= 0xffffffff;

	enum E_CollisionSourceType
	{
		E_CollisionSourceType_Mesh		= 0x00000001,
		E_CollisionSourceType_Spline	= 0x00000002,
		E_CollisionSourceType_Proxy		= 0x00000010
	};

	enum E_CollisionSourceFaces
	{
		E_CollisionSourceFace_Front	= 0x40000000,
		E_CollisionSourceFace_Back	= 0x80000000
	};

	enum E_CollisionQueryType
	{
		E_CollisionQueryType_Complex,
		E_CollisionQueryType_Bounds
	};
	
	enum E_CollisionInstanceType
	{
		E_CollisionInstanceType_Internal,
		E_CollisionInstanceType_AftkExternal
	};

	struct C_CollisionEdge
	{
		Maths::HVec3 begin;
		Maths::HVec3 end;
	};

	struct C_CollisionOBB
	{
		Maths::HMat43 transform;
		Maths::HVec3 extents;
	};

#ifndef SWIG

	struct I_CollisionSource
	{
	public:
		virtual ~I_CollisionSource() {}
		virtual E_CollisionSourceType GetType() const = 0;
	};

	struct C_CollisionMesh : public I_CollisionSource
	{
		virtual ~C_CollisionMesh()
		{
			//No constructor, so up to user to ensure initialisation.
			delete[] pVertices;
			delete[] pTriIndices;
		}

		virtual E_CollisionSourceType GetType() const { return E_CollisionSourceType_Mesh; }

		Vector3* pVertices;
		U32 nVertices;
		U32* pTriIndices;
		U32 nTris;

		C_CollisionOBB obb;
	};

	struct C_CollisionSpline : public I_CollisionSource
	{
		struct C_Tangents
		{
			Vector3 tangentIn;
			Vector3 tangentOut;
		};

		virtual ~C_CollisionSpline()
		{
			//No constructor, so up to user to ensure initialisation.
			delete[] pVertices;
			delete[] pTangents;
		}

		virtual E_CollisionSourceType GetType() const { return E_CollisionSourceType_Spline; }

		const Vector3* pVertices;
		C_Tangents* pTangents;
		U32 nVertices;

		C_CollisionOBB obb;
	};

	struct C_CollisionProxy : public I_CollisionSource
	{
		virtual ~C_CollisionProxy() {}
		virtual E_CollisionSourceType GetType() const { return E_CollisionSourceType_Proxy; }

		//Bounding sphere.
		Maths::C_Sphere boundingSphere;
	};

	struct I_CollisionInstance
	{
		I_CollisionInstance() {}
		I_CollisionInstance(I_CollisionInstance& rhs) {	*this = rhs; }
		virtual ~I_CollisionInstance() {}
		virtual E_CollisionInstanceType GetType() const = 0;

		const I_CollisionInstance& operator=(I_CollisionInstance& rhs)
		{
			pSource = rhs.pSource;
			toObjectSpace = rhs.toObjectSpace;
			return *this;
		}

		//live source pointer, this should always be valid as our instance source and this associated 
		//instance are loaded and unloaded as one.
		I_CollisionSource* pSource;

		//Used to transform from world space into object space.
		Matrix44 toObjectSpace;
		Matrix44 toWorldSpace;
	};


	//An internal collision instance is an instance of a collidable shape that's within the
	//same collision scene as its source
	struct C_InternalCollisionInstance : public I_CollisionInstance
	{
		virtual ~C_InternalCollisionInstance() {}
		virtual E_CollisionInstanceType GetType() const { return E_CollisionInstanceType_Internal; }
	};

	//An external collision instance is an instance of an aftk node. Specifically it is used where
	//the instance is in a different aftk scene (and this different collision scene) to the source.
	//It is also used when our instance doesn't originate from an aftk scene at all. For instance
	//if we've an actor that has some mesh proxy associated with it for rendering.
	struct C_ExternalAftkCollisionInstance : public I_CollisionInstance
	{
		C_ExternalAftkCollisionInstance() {}
		C_ExternalAftkCollisionInstance(C_ExternalAftkCollisionInstance& rhs) {	*this = rhs; }
		virtual ~C_ExternalAftkCollisionInstance() {}
		const C_ExternalAftkCollisionInstance& operator=(C_ExternalAftkCollisionInstance& rhs)
		{
			__super::operator=(*this);
			originalFilePath = rhs.originalFilePath;
			aftkNodePath = rhs.aftkNodePath;
		}

		virtual E_CollisionInstanceType GetType() const { return E_CollisionInstanceType_AftkExternal; }

		//A live source pointer - this will be null if the external source is not in memory.
		I_CollisionSource* pSource;
		Matrix44 worldTransform;

		Btl::String originalFilePath;
		Btl::String aftkNodePath;
	};

#endif

}

#endif
