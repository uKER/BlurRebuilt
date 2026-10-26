#ifndef COMPOSITE_DESC_H
#define COMPOSITE_DESC_H

#include "gravity/sdk/BodyHelper.h"
#include "gravity/sdk/ShareDeserialiser.h"
#include "gravity/SharedPhysics/SortedArray.h"

namespace Collision
{
	class C_CollisionFilter;
	class C_ShapeWrapper;
}

namespace Gravity
{

class C_InstanceMeshResourceManager;

class C_ShapeDescWrapper
{
public:
	C_ShapeDesc* m_pShapeDesc;
	Maths::HMat33 m_IneriaTensor;
	Maths::HMat43 m_RelativeTransformation;
	Maths::HF32 m_Mass;
	Collision::C_CollisionFilter* m_pFilter;

	Collision::C_ShapeWrapper Serialise(Deserialiser& serial, Collision::C_CollisionFilter* pFilter);

	Collision::C_ShapeWrapper Shape(const Collision::C_CollisionFilter* pFilter);

	Collision::C_Shape* Shape(const Collision::C_CollisionFilter* pFilter, void* pAddress);
	

	bool Import(Deserialiser& deserial, SharedPhysics::SortedArray<C_ShapeDesc* >& ownedShapes, Gravity::C_InstanceMeshResourceManager* pInstMeshManager, bool bShareData);

	template <typename Serialiser>
	void Export(Serialiser& serial);
	
	C_ShapeDescWrapper() : m_IneriaTensor(Maths::Constants<Maths::HMat33>::Zero()), m_RelativeTransformation(Maths::Constants<Maths::HMat43>::Identity()),
		m_Mass(Maths::Constants<Maths::HF32>::Zero())
	{
		m_pShapeDesc = 0;
		m_pFilter = 0;
	}
};


class C_CompositeDesc : public C_ShapeDesc
{
public:

	SharedPhysics::SortedArray<C_ShapeDescWrapper > shapes;
	SharedPhysics::SortedArray<C_ShapeDesc*> m_OwnedShapes;
	Gravity::C_InstanceMeshResourceManager* m_instanceMeshManager;
	bool m_DecomposeSingleComposite;
	bool m_ShareData;
	

	C_CompositeDesc(Gravity::C_InstanceMeshResourceManager* instanceMeshManager = NULL, const S32 hintNumShapes = 4) : C_ShapeDesc(Collision::E_Composite_Shape, Collision::E_ImportComposite_Shape),
																			shapes(hintNumShapes), 
																			m_instanceMeshManager(instanceMeshManager),
																			m_DecomposeSingleComposite(true)
	{
	}


	virtual ~C_CompositeDesc()
	{ 
		for(S32 a = 0; a < m_OwnedShapes.m_numElements; ++a)
		{
			if(m_OwnedShapes[a])
				delete m_OwnedShapes[a];
			m_OwnedShapes[a]=NULL;
		}
	}

	virtual void SetPhysicsMaterial(const U32 material)
	{
		m_PhysicsMaterialIndex = material;
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			shapes[a].m_pShapeDesc->SetPhysicsMaterial(material);
		}
	}
	
	virtual C_CompositeDesc* GetComposite()
	{
		return this;
	}

	virtual U32 GetSize() const;

	U32 GetDepth()
	{
		U32 depth = 0;
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			if(shapes[a].m_pShapeDesc->m_Type == Collision::E_Composite_Shape)
			{
				depth = Maths::Max(depth, ((C_CompositeDesc*)shapes[a].m_pShapeDesc)->GetDepth());
			}
		}
		return depth + 1;
	}

	U32 GetCompositeSize() const
	{
		U32 size = 0;
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			if(shapes[a].m_pShapeDesc->ValidShapeDescriptor())
			{
				size += shapes[a].m_pShapeDesc->GetSize();
			}
		}
		return size;
	}


	void SetMeshManager(Gravity::C_InstanceMeshResourceManager* instanceMeshManager)
	{
		m_instanceMeshManager=instanceMeshManager;
	}

	virtual Maths::HF32 CalculateVolume();

	virtual void RemapShape();

	virtual void SetRemapped();

	virtual Maths::HVec3 CalculateCentreOfMass();

	virtual bool ValidShapeDescriptor()
	{
		return GetNumValidShapes() > 0;
	}

	virtual U32 GetNumValidShapes()
	{
		U32 numValidShapes = 0;
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			numValidShapes += ((U32)shapes[a].m_pShapeDesc->ValidShapeDescriptor());
		}
		return numValidShapes;
	}

	void AddShape(C_ShapeDesc* pDesc, const Maths::HMat43 PCREF relativeOrientation, const Maths::HF32 PCREF mass,
		Collision::C_CollisionFilter* pFilter);

	
	void AddShape(C_ShapeDesc* pDesc, const Maths::HMat43 PCREF relativeOrientation, const Maths::HF32 PCREF mass);

	virtual void ApplyStaticRotation(const Maths::HMat33 PCREF rot);
	
	/*
		This is used in the exporter
	*/
	void AddOwnShape(C_ShapeDesc* pDesc, const Maths::HMat43 PCREF relativeOrientation, const Maths::HMat33 PCREF inertiaTensor, const Maths::HF32 PCREF mass);

	//void AddShape(C_ShapeDesc* pDesc, const Maths::HMat43 PCREF relativeOrientation);
	

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual void CalculateNormalisedMasses(const Maths::HF32 PCREF totalMass);

	void Decompose()
	{
		SharedPhysics::SortedArray<C_ShapeDescWrapper > newShapes;
		Maths::HMat43 relTransform;
		relTransform.SetIdentity();
		Decomposite(newShapes, relTransform);
	
		shapes.m_numElements=0;

		for(S32 i=0; i<newShapes.m_numElements; ++i)
		{
			shapes.AddToEnd(&newShapes[i]);
		}
	}

	

	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		U32 numElems;
		if(deserial.m_stream.GetVersionNumber() < 6)
			deserial >> numElems;
		else
			deserial >> numElems >> m_ConstraintBodyID;
		shapes.Resize(numElems);
		shapes.m_numElements = numElems;

	
		bool succeeded = true;
		for(S32 i=0; i<shapes.m_numElements; ++i)
		{
			if(!shapes[i].Import(deserial, m_OwnedShapes, m_instanceMeshManager, m_ShareData))
			{
				succeeded = false;
			}
		}
		return succeeded;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		serial<< shapes.m_numElements << m_ConstraintBodyID;

		for(S32 i=0; i<shapes.m_numElements; ++i)
		{
			shapes[i].Export<Serialiser>(serial);
		}
	}
private:
	void Decomposite(SharedPhysics::SortedArray<C_ShapeDescWrapper >& newShapes, Maths::HMat43 PCREF relTransform)
	{
		for(S32 i=0; i< shapes.m_numElements; ++i)
		{
			C_ShapeDescWrapper& s=shapes[i];
			Maths::HMat43 newTrans(s.m_RelativeTransformation * relTransform);
			Gravity::C_ShapeDesc* desc=s.m_pShapeDesc;
			if(desc->m_Type == Collision::E_Composite_Shape)
			{
				Gravity::C_CompositeDesc* cShape=(Gravity::C_CompositeDesc*)desc;
				cShape->Decomposite(newShapes, newTrans);	
			}
			else
			{
				Gravity::C_ShapeDescWrapper newWrapper = s;
				newWrapper.m_RelativeTransformation = newTrans;
				newShapes.AddToEnd(&newWrapper);
			}

		}
	}
};

}

#endif
