#include "StdAfx.h"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/collisionmanager.h"
#include "gravity/cloths/SoftBody.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/shapes.h"
#include "gravity/narrowphase/OBBRay.inl"
#include "gravity/sharedphysics/ContactStream.inl"
#include "gravity/narrowphase/SphereSphere.h"
#include "gravity/narrowphase/OBBSphere.h"
#include "gravity/narrowphase/CapsuleSphere.h"


namespace Collision
{

template <U32 ContactSize>
void EncodeContacts(C_ParticleContactBuffer<ContactSize>& contacts, SharedPhysics::C_ContactStream& stream)
{
	if(contacts.m_NumContacts == 0)
		return;

	Spu::MemBaseMarker marker;
	const U32 size = (sizeof(SharedPhysics::C_ParticleContactPair) + sizeof(SharedPhysics::C_ParticleContactStorage) * ContactSize) + 64;
#ifndef PLATFORM_SPU
	ALIGN16 U8 tempData[size];
#else
	U8* tempData = (U8*)Spu::Alloc(size);
#endif

	U8* dataStream(tempData);
	const U32 dataSize = sizeof(SharedPhysics::C_ParticleContactPair) + (sizeof(SharedPhysics::C_ParticleContactStorage) * contacts.m_NumContacts);
	U8* writeData = stream.LockMemory(dataSize);

	BIZ_ASSERT((writeData + dataSize) < stream.m_pStreamEnd);

	SharedPhysics::C_ParticleContactPair* pPair = (SharedPhysics::C_ParticleContactPair*)dataStream;
	dataStream += sizeof(SharedPhysics::C_ParticleContactPair);
	pPair->m_NumContacts = contacts.m_NumContacts;
	pPair->m_ValidData = SharedPhysics::E_ValidData;
	pPair->m_pNodePtr = contacts.m_Contacts[0].m_pNodePtr;
	pPair->m_pParticlePtr = contacts.m_Contacts[0].m_pParticlePtr;

	for(U32 a = 0; a < contacts.m_NumContacts; ++a)
	{
		C_ParticleContact& contact = contacts.m_Contacts[a];
		SharedPhysics::C_ParticleContactStorage* pResult((SharedPhysics::C_ParticleContactStorage*)dataStream);
		dataStream += sizeof(SharedPhysics::C_ParticleContactStorage);
		pResult->m_ValidData = SharedPhysics::E_ValidData;
		//pResult->m_pClothPtr = contact.m_pClothPtr;
		//pResult->m_pNodePtr = contact.m_pNodePtr;
		Maths::Store(contact.m_Contact, &pResult->m_Contact);
		Maths::Store(contact.m_Penetration, &pResult->m_Penetration);
		Maths::Store(contact.m_Stiffness, &pResult->m_Stiffness);
		Maths::Store(contact.m_EscapeVector, &pResult->m_EscapeVector);
		Maths::Store(contact.m_TriangleNormal, &pResult->m_TriangleNormal);
		pResult->m_Type = contact.m_Type;
		pResult->m_Index = contact.m_Index;
	}

	Spu::Store(writeData, tempData, dataSize);
	
}



	

	Maths::HF32 PenDepth(const Collision::C_OBB& obb, const Maths::HVec3 PCREF point, const Maths::C_Normal PCREF normal)
	{
		const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HVec3 e(obb.Extent());
		const Maths::HVec3 end(point + normal * (e.X() + e.Y() + e.Z()) * two);
		Maths::HVec3 minContact(Maths::UNDEFINED), maxContact(Maths::UNDEFINED);
		Collision::IntersectSegmentBox(point, end, e, minContact, maxContact);
		const Maths::HF32 pen((maxContact - point) | normal);
		return pen;
	}


	bool C_CollisionManager::CollideClothDynamic(C_CollisionObject* obj, SharedPhysics::C_GraphNode* pClothNode,
		C_ClothShape* pCloth, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_ContactStream& clothContactStream)
	{
		C_ParticleContactBuffer<SoftBodyContactSize> contacts;
		//This will eventually be a switch table of all different collision types!
		if(obj->m_pShape->GetShape() == Collision::E_Sphere_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, SphereClothTriangle))
			if(pCloth->m_Cloth.CollideParticles(obj, pClothNode, stack, contacts, SphereSoftBodyTriangle))
			{
				//Encode contacts...
				EncodeContacts<SoftBodyContactSize>(contacts, clothContactStream);
				return true;
			}
			//return pCloth->m_Cloth.CollideAndResolve(obj, stack, SphereClothTriangle);
		}
		else if (obj->m_pShape->GetShape() == Collision::E_Box_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, BoxClothTriangle))
			if(pCloth->m_Cloth.CollideParticles(obj, pClothNode, stack, contacts, BoxSoftBodyTriangle))
			{
				EncodeContacts<SoftBodyContactSize>(contacts, clothContactStream);
				return true;
			}
		}
		else if (obj->m_pShape->GetShape() == Collision::E_Capsule_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, CapsuleClothTriangle))
			if(pCloth->m_Cloth.CollideParticles(obj, pClothNode, stack, contacts, CapsuleSoftBodyTriangle))
			{
				EncodeContacts<SoftBodyContactSize>(contacts, clothContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_Cylinder_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, CylinderClothTriangle))
			if(pCloth->m_Cloth.CollideParticles(obj, pClothNode, stack, contacts, CylinderSoftBodyTriangle))
			{
				EncodeContacts<SoftBodyContactSize>(contacts, clothContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_Cone_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, ConeClothTriangle))
			if(pCloth->m_Cloth.CollideParticles(obj, pClothNode, stack, contacts, ConeSoftBodyTriangle))
			{
				EncodeContacts<SoftBodyContactSize>(contacts, clothContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_ConvexHull_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, ConvexHullClothTriangle))
			if(pCloth->m_Cloth.CollideParticles(obj, pClothNode, stack, contacts, ConvexHullSoftBodyTriangle))
			{
				EncodeContacts<SoftBodyContactSize>(contacts, clothContactStream);
				return true;
			}
		}
		else if(obj->m_pShape->GetShape() == Collision::E_Composite_Shape)
		{
			//if(pCloth->m_Cloth.Collide(obj, pClothNode, stack, contacts, CompositeClothTriangle))
			if(pCloth->m_Cloth.CollideParticles(obj, pClothNode, stack, contacts, CompositeSoftBodyTriangle))
			{
				EncodeContacts<SoftBodyContactSize>(contacts, clothContactStream);
				return true;
			}
		}
		return false;
	}
}
