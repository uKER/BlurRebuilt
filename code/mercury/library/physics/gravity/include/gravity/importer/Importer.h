#ifndef IMPORTER_H
#define IMPORTER_H

#include "gravity/narrowphase/Shape.h"
#include "gravity/sharedphysics/GenericStackAllocator.h"
#include "gravity/sdk/BodyHelper.h"
#include "gravity/sdk/ConstraintDescriptor.h"
#include "gravity/importer/InstanceMeshResourceManager.h"
#include "gravity/sdk/BodyHelper.h"


namespace Gravity
{
class C_Scene;
class C_Body;
struct C_ExternalConstants;

class C_ImportBodyWrapper
{
	C_Body* m_pBody;
	S32 m_ImportIndex;

public:
	C_ImportBodyWrapper(C_Body* pBody, U32 importIndex) : m_pBody(pBody), m_ImportIndex(importIndex)
	{
	}

	C_ImportBodyWrapper() : m_pBody(NULL), m_ImportIndex(-1)
	{
	}

	C_Body* GetBody()
	{
		return m_pBody;
	}

	S32 GetImportIndex()
	{
		return m_ImportIndex;
	}

	bool operator < (const C_ImportBodyWrapper& other) const;
	bool operator <= (const C_ImportBodyWrapper& other) const;
	bool operator > (const C_ImportBodyWrapper& other) const;
	bool operator >= (const C_ImportBodyWrapper& other) const;
	bool operator == (const C_ImportBodyWrapper& other) const;
	bool operator != (const C_ImportBodyWrapper& other) const;
};

//! C_ImportShape
class C_ImportShape
{
	//template <class Deserialiser> friend class C_ImportScene_T;

       // We have wrapped the C_ShapeDesc<> with this class so we can add extra information in here later such as the intertia tensor, mass etc.

public:
	C_ImportShape(): m_pShapeDesc(NULL)
	{
	}

	~C_ImportShape()
	{
		if(m_pShapeDesc)
			delete m_pShapeDesc;
		m_pShapeDesc=NULL;
	}

	/*C_ShapeDesc* GetConstraintDesc() const
	{
		return m_pConstraintDesc;
	}*/

	/*void SetConstraintDesc(C_ShapeDesc* pShapeDesc)
	{
		m_pConstraintDesc = pShapeDesc;
	}*/

	C_ShapeDesc* GetShapeDesc() const
	{
	   return m_pShapeDesc;
	}

	void SetShapeDesc(C_ShapeDesc* pShape)
	{
		m_pShapeDesc = pShape;
	}

	void SetImportPosition(const Maths::HVec3 PCREF position)
	{
		m_ImportPosition = position;
	}

	void SetImportRotation(const Maths::HMat33 PCREF rotation)
	{
		m_ImportRotation = rotation;
	}

	void SetImportIneria(const Maths::HMat33 PCREF ineria)
	{
		m_ImportIneria = ineria;
	}

	Maths::HMat33 GetImportIneriaTensor() const
	{
		return m_ImportIneria;
	}

	Maths::HMat43 GetImportMatrix() const
	{
		return Maths::HMat43(m_ImportRotation, m_ImportPosition + m_pShapeDesc->m_RenderOffset);
	}

	Maths::HVec3 GetImportPosition() const
	{
		return m_ImportPosition + m_pShapeDesc->m_RenderOffset;
	}

	Maths::HMat33 GetImportRotation() const
	{
		return m_ImportRotation;
	}



private:
	//C_ShapeDesc* m_pConstraintDesc;
	C_ShapeDesc* m_pShapeDesc;
	Maths::HVec3 m_ImportPosition;
	Maths::HMat33 m_ImportRotation;
	Maths::HMat33 m_ImportIneria;

};



//! C_ImportScene

class C_ImportSceneStaticInstance;

class C_ImportScene
{
	friend class C_Scene;
	friend class C_PhysicsSDK;
private:
	C_ImportScene();
	~C_ImportScene();
public:

	bool Import( Sys::I_BinaryStream* filePtr );
	void RemoveStaticShapes();

	Gravity::C_ShapeDesc* GetShapeDesc(U32 m_Type);
	
	U32 GetImportShapeCount() const;

	U32 GetNumConstraintCount() const;
	
	/*
		This method get out the C_ImportShape_T of the dynamic objects
	*/
	C_ImportShape* GetImportShape( const U32 index ) const;

	/*
		Get out the subshape descriptor of the dynamic(compound) objects
	*/
	Gravity::C_ShapeDesc* GetSubDynamicShape( Gravity::C_CompositeDesc & comp, const U32 subIndex ) const;


	//which is called by the model file
	C_ConstraintDescriptors* GetConstraint( const U32 id) const;

	SharedPhysics::SortedArray<C_ImportShape* > m_DynamicShapes; //deal with the dynamic compound objects
	SharedPhysics::SortedArray<C_ImportShape*> m_ClothShapes; //deal with the dynamic cloth
	SharedPhysics::SortedArray<C_ImportShape*> m_DeformableShapes; //deal with the dynamic cloth
	SharedPhysics::SortedArray<C_ImportShape*> m_FluidMeshShapes;
	SharedPhysics::SortedArray<C_ShapeDesc* > m_StaticShapesDesc;
	SharedPhysics::SortedArray<C_ShapeInstance* > m_StaticInstance;
	SharedPhysics::SortedArray<C_ConstraintDescriptors* > m_ConstraintsDesc;
	Gravity::C_InstanceMeshResourceManager m_StaticInstanceManager; //deal with the static base instances and their instances
	C_ImportSceneStaticInstance* m_pStaticScene;
	bool m_ShareData;


};

//! C_ImportSceneStaticInstance

class C_ImportSceneStaticInstance
{
public:
	// This object is created after an import scene has been loaded.
	// When this constructor is called, it will add all the static data from the C_ImportScene into the C_Scene.
	// The static data that is created is everything that was exported with the C_Converter::AddStaticNodes() method.
	C_ImportSceneStaticInstance( C_Scene* pScene, C_ImportScene* pImportScene, const Maths::HVec3 PCREF desiredSize = Maths::HVec3(20.f, 20.f, 20.f),
		const Maths::HVec3 PCREF maxCellSize = Maths::HVec3(100.f, 100.f, 100.f), 
		Maths::HF32 PCREF fluidDepth = Maths::HF32(100.f));

	C_ImportSceneStaticInstance( C_Scene* pScene, C_ImportScene* pImportScene, const Maths::HVec3 PCREF desiredSize,
		const Maths::HVec3 PCREF maxCellSize, const Collision::C_CollisionFilter* pFilter, const Collision::C_CollisionFilter* pFluidFilter, Maths::HF32 PCREF fluidDepth = Maths::HF32(100.f));

	// When this object is destroyed, the static parts that were added to the C_Scene in the constructor will be removed from the C_Scene.
	~C_ImportSceneStaticInstance();

	void Import();


	friend class C_ImportScene;
private:
	C_ImportScene* m_pImportScene;
	SharedPhysics::SortedArray< C_Body* > m_pBodies;
	C_Scene* m_pScene;
	Collision::C_CollisionFilter m_Filter, m_FluidFilter;
	Gravity::C_ExternalConstants* m_ExternalConstants;
	Maths::HVec3 m_DesiredSize;
	Maths::HVec3 m_MaxCellSize;
	Maths::HF32 m_FluidDepth;
	void* m_pUserData;

	
};



}


#endif
