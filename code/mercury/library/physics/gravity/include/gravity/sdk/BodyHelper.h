#ifndef BODYHELPER_H
#define BODYHELPER_H
#include <gravity/sdk/ShareDeserialiser.h>
#include <gravity/narrowphase/Shape.h>

#include "gravity/SharedPhysics/GraphNode.h"

//A helper class to fill in all the details required in a C_Body from the data in a shape descriptor 
//coupled with mass and orientation/position!

namespace Collision
{
	class C_Shape;
};

namespace Gravity
{


class C_Body;
class C_CompositeDesc;
//template <class Deserialiser>
class C_ShapeDesc
{
public:

	Maths::HVec3 m_RenderOffset;
	Maths::HF32 m_Margin;
	Maths::HF32 m_Epsilon;
	U32 m_PhysicsMaterialIndex;
	U32 m_EffectMaterialIndex;
	U32 m_CollisionFlag;
	U32 m_UserDataFlag;
	F32 m_Mass;
	S32 m_RemapExportID;
	S32 m_OriginalExportID;
	S32 m_ConstraintBodyID;
	mutable U32 m_NodeFlag; //static, active, sleeping, dynamic
	Collision::E_ShapeType m_Type; //sphere, box, capsule, convexhull, mesh, cylinder.... 
	Collision::E_ImportShapeType m_ImportType;
	U32 m_Remapped;
	bool m_HasCentreOfMass;
	bool m_HasVolume;

	bool m_HasCollision;

	C_ShapeDesc(const Collision::E_ShapeType type, const Collision::E_ImportShapeType importType) : m_Epsilon(Maths::Constants<Maths::HF32>::Epsilon2()), m_Margin(Maths::Constants<Maths::HF32>::Zero()),
		m_RenderOffset(Maths::Constants<Maths::HVec3>::Zero()),	m_PhysicsMaterialIndex(0), m_EffectMaterialIndex(0), m_CollisionFlag(0), m_NodeFlag(SharedPhysics::C_GraphNode::E_Type_Active), m_Type(type), m_ImportType(importType),m_Mass(1.f),
		m_Remapped(false), m_HasVolume(true), m_RemapExportID(-1), m_OriginalExportID(-1), m_ConstraintBodyID(-1), m_HasCentreOfMass(true),
		m_UserDataFlag(0), m_HasCollision(true)
	{
	}

	C_ShapeDesc(): m_Epsilon(Maths::Constants<Maths::HF32>::Epsilon2()), m_Margin(Maths::Constants<Maths::HF32>::Zero()),
		m_RenderOffset(Maths::Constants<Maths::HVec3>::Zero()), m_PhysicsMaterialIndex(0), m_EffectMaterialIndex(0), m_CollisionFlag(0), m_NodeFlag(SharedPhysics::C_GraphNode::E_Type_Active), m_Mass(1.f),
		m_Remapped(false), m_HasVolume(true), m_RemapExportID(-1), m_OriginalExportID(-1), m_ConstraintBodyID(-1),
		m_UserDataFlag(0), m_HasCollision(true)
	{
	}

	virtual ~C_ShapeDesc()
	{
	}

	virtual U32 GetSize() const = 0;

	//Method which normally does nothing, but will for convex hulls and composite shapes remap
	//the structure of the object so that it is centred around the calculated centre of mass 
	//for the shape

	void EnableCollisions(bool isEnabled)
	{
		m_HasCollision = isEnabled;
	}
	
	virtual C_CompositeDesc* GetComposite()
	{
		return NULL;
	}
	
	virtual void RemapShape()
	{
		//m_Remapped = true;
	}

	virtual U32 GetNumValidShapes()
	{
		if(ValidShapeDescriptor())
			return 1;
		return 0;
	}

	virtual void SetRemapped()
	{
		m_Remapped = true;
	}

	virtual void SetHasBuoyancy(bool bHasBuoyancy)
	{
		m_HasVolume = bHasBuoyancy;
	}

	virtual Maths::HVec3 CalculateCentreOfMass()
	{
		return Maths::Constants<Maths::HVec3>::Zero();
	}

	virtual bool HasCentreOfMass()
	{
		return m_HasCentreOfMass;
	}

	virtual void ApplyStaticRotation(const Maths::HMat33 PCREF rot)
	{
	}

	virtual bool ValidShapeDescriptor() const
	{
		return true;
	}

	virtual Maths::HF32 CalculateVolume()
	{
		return Maths::Constants<Maths::HF32>::Zero();
	}

	virtual void SetPhysicsMaterial(const U32 material)
	{
		m_PhysicsMaterialIndex = material;
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const =0 ;

	//Create shape in place!!!
	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const =0 ;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter) =0;

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const=0;
	virtual Collision::C_CRAxisBox GetBoundingBox() const=0;

	virtual Maths::HF32 GetSlop() const
	{
		return Maths::HF32(0.000005f);
	}

	virtual bool Import(Deserialiser& deserial)=0;	

	void ImportBasic(Deserialiser& deserial)
	{
		deserial >> m_Margin >> m_Epsilon >> m_PhysicsMaterialIndex;
		if(deserial.m_stream.GetVersionNumber() >= 2)
			deserial >> m_EffectMaterialIndex;
		else
			m_EffectMaterialIndex = m_PhysicsMaterialIndex;
		deserial >> m_CollisionFlag >> m_NodeFlag >> m_Mass >> m_OriginalExportID >>m_RenderOffset >> m_Remapped;

		if(deserial.m_stream.GetVersionNumber() >= 7)
			deserial >> m_UserDataFlag;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial) const
	{
		serial << U32(m_ImportType) << m_Margin << m_Epsilon << m_PhysicsMaterialIndex << m_EffectMaterialIndex << m_CollisionFlag << m_NodeFlag << m_Mass << m_OriginalExportID << m_RenderOffset << m_Remapped << m_UserDataFlag;
	}

	U32 GetNodeFlags() const
	{
		return m_NodeFlag;
	}

	U32 GetCollisionFlags() const
	{
		return m_CollisionFlag;
	}

	F32 GetMass() const
	{
		return m_Mass;
	}

	virtual void CalculateNormalisedMasses(const Maths::HF32 PCREF totalMass)
	{
		m_Mass = totalMass;
	}

};

//template <class Deserialiser>
class C_ShapeInstance
{
public:
	C_ShapeInstance() : m_Position(Maths::UNDEFINED)
	{
	}

	~C_ShapeInstance()
	{
	}
	
	Maths::HVec3 m_Position;
	//Maths::HMat33 m_Rotation;
	MathsStorage::Matrix33 m_Rotation;
	MathsStorage::Matrix33 m_IneriaTensor;

	//Maths::HMat33 m_IneriaTensor;
	Gravity::C_ShapeDesc* m_pDesc;

	void Import(Deserialiser& deserial)
	{
		//m_pDesc->Import(deserial);
		deserial >> m_Position >> m_Rotation >> m_IneriaTensor;
		
	}

	template <typename Serialiser>
	void Export(Serialiser& serial) const
	{
		//m_pDesc->Export(serial);
		serial << m_Position << m_Rotation << m_IneriaTensor;
	}

};

//template <class Serial>
class C_BodyHelper
{
	C_ShapeDesc* m_pShapeDesc;
public:
	C_BodyHelper()
	{
	}

	C_BodyHelper(C_ShapeDesc* pShapeDesc) : m_pShapeDesc(pShapeDesc)
	{
	}

	void UpdateBodyShape(const Maths::HF32 PCREF mass, C_Body* pBody, const Collision::C_CollisionFilter* pFilter);

	//The position and orientation is passed from the seriliser
	void CreateBody(const Maths::HF32 PCREF mass, C_Body* pBody,  Deserialiser& serial, const Collision::C_CollisionFilter* pFilter, const U16 physicsMaterial, const U16 effectMaterial);

	void CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, Deserialiser& serial, const Collision::C_CollisionFilter* pFilter, const U16 physicsMaterial, const U16 effectMaterial);
	

	//The position is passed by the user
	void CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, const Collision::C_CollisionFilter* pFilter, const U16 physicsMaterial, const U16 effectMaterial);

	void CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, const Collision::C_CollisionFilter* pFilter);

	/*
		The methods below are used for importing the intermediate layer into the scene
	*/
	//The method is for triangle mesh
	void CreateBody(C_Body* pBody, const Collision::C_CollisionFilter* pFilter);

	//This method is for normal shapes
	void CreateBody(const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, C_Body* pBody, const Collision::C_CollisionFilter* pFilter);
	
	void CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HMat33 PCREF ineriaTensor, 
		C_Body* pBody, const Collision::C_CollisionFilter* pFilter);
	
	void CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags);

	void CreateBody(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, 
		const Maths::HMat33 PCREF orientation, const Maths::HMat33 PCREF ineriaTensor, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags);

	void CreateBody(const Maths::HF32 PCREF mass, const Gravity::C_ShapeInstance* pInst, C_Body* pBody, Collision::C_Shape* pShape, const U32 nodeFlags);

	void MakeFloat(const Maths::HF32 PCREF fluidDensity, const Maths::HF32 PCREF scale, const Maths::HF32 PCREF maxBuoyancy, C_Body* pBody,
	const Maths::HF32 PCREF linearDrag, const Maths::HF32 PCREF angularDrag);
	
};

};

#endif
