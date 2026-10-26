#ifndef NEWCONTACTSTREAMITERATOR_H
#define NEWCONTACTSTREAMITERATOR_H
#include "gravity/sharedphysics/ContactStream.h"
//#include "gravity/solver2/ConstraintGraphManager.h"

//An iterator which allows us to traverse over a contact stream, manipulate, add and remove contacts
//The structure of the contact stream is as follows:
//CollisionResult
//   -- CollisionPatch
//      -- ContactPoint
//A collision Result stores all contacts between a pair of bodies. These contacts are broken up into
//patches with common normals. These patches can then be further subdivided into contact points
//Results, patches and points can be invalidated or manipulated from within the contact stream iterator. 
//This is useful because it allows for game-side contact manipulation as well as reading, to allow game-side
//logic to affect the way in which the physics engine behaves. For example, invalidating a contact will
//cause the contact not to be responded to. This may allow us to override the physics engine's default contact
//behaviour.
//It is not permitted to insert new contact points or patches into an existing collision result as this would require
//very expensive memory shuffling!

namespace Solver2
{
	class C_ConstraintGraphManager;
}

namespace SharedPhysics
{
class C_ContactStream;
class C_ConstContactStreamPtr;
struct C_ContactStreamResult;
struct C_ContactStreamPatch;
struct C_ContactStreamPoint;
struct C_ClothContactStorage;
enum C_ContactType
{
	SEPERATING,
	RESTING,
	COLLIDING,
	SLIDING
};
class C_ContactStreamIterator
{
public:
	C_ContactStreamIterator(C_ContactStream* pStream, Solver2::C_ConstraintGraphManager* pManager, const Maths::HF32 PCREF timeStep);

	C_ContactStreamIterator(C_ConstContactStreamPtr* pStream, Solver2::C_ConstraintGraphManager* pManager, const Maths::HF32 PCREF timeStep);

	C_ContactStreamIterator() : m_Dt(Maths::UNDEFINED)
	{
	}

	C_ContactStreamIterator(const C_ContactStreamIterator& iter);

	C_ContactStreamIterator& operator = (const C_ContactStreamIterator& iter);

	//This code works by detecting the continuous collision between the two
	//bodies in contact. It works using their bounding boxes and using these
	//to generate new contact points/normals. It will discard the existing 
	//contacts generated in collision detection. NB. This is quite expensive, so 
	//avoid using when dealing with unimportant collisions or low-speed collisions.
	void HandleTunnelEventDynamicBodies();

public:
	bool hasNextContact();
	C_ContactStreamResult* nextContact();
	bool hasNextPatch() ;
	C_ContactStreamPatch*  nextPatch();
	bool hasNextPoint();
	C_ContactStreamPoint* nextPoint() ;
	C_ContactStreamPoint* currentPoint() ;

	C_ContactStream* GetStream() {return m_pStream;}

	void DampBodies(const Maths::HF32 PCREF linearDamp, const Maths::HF32 PCREF angularDamp);

	void GetImpulseAndFriction(Maths::HF32& impluse, Maths::HVec3& friction, bool forceBodyBActive = false);
	C_ContactType GetContactType();
	
	//This function normalises the relative mass of the objects. If relative mass differential 
	//greater than maxRelativeMass, then the objects are scaled accordingly. If mass is over 
	//assumedInfiniteRelativeMass, then the mass of the larger object is assumed to be infinite
	void NormaliseContacts(const Maths::HF32 PCREF maxRelativeMass, 
		const Maths::HF32 PCREF assumedInfiniteRelativeMass);

	void NormaliseContacts(const Maths::HF32 PCREF assumedInfiniteRelativeMass);

	void AbsorbEnergy(const Maths::HF32 PCREF damping);

	void SetMassRatioA(const F32 massRatio);
	void SetMassRatioB(const F32 massRatio);
	void SetInertiaRatioA(const F32 ratio );
	void SetInertiaRatioB(const F32 ratio );

	Maths::HF32 timeStep()
	{
		return m_Dt;
	}
	
private:
	C_ContactStream* m_pStream;
	U8* m_pCurrentPointer;
	C_ContactStreamResult* m_pCurrentResult;
	C_ContactStreamPatch* m_pCurrentPatch;
	C_ContactStreamPoint* m_pCurrentPoint;

	C_ContactStreamPatch* m_pNextPatch;
	C_ContactStreamPoint* m_pNextPoint;
	Solver2::C_ConstraintGraphManager* m_pManager;
	S32 m_PatchCount, m_PointCount;
	Maths::HF32 m_Dt;
};

class C_SingleContactIterator
{
	public:
	C_SingleContactIterator(C_ConstContactStreamPtr* pStream);

public:
	__forceinline bool validResult() 
	{
		return m_pCurrentResult != NULL && m_pCurrentResult->m_ValidData == E_ValidData;
	}
	C_ContactStreamResult*result();
	bool hasNextPatch() ;
	C_ContactStreamPatch* nextPatch();
	bool hasNextPoint();
	C_ContactStreamPoint* nextPoint();
	C_ContactStreamPoint* currentPoint();
	
private:
	C_ContactStream* m_pStream;
	U8* m_pCurrentPointer;
	C_ContactStreamResult* m_pCurrentResult;
	C_ContactStreamPatch* m_pCurrentPatch;
	C_ContactStreamPoint* m_pCurrentPoint;
	S32 m_PatchCount, m_PointCount;
};

class C_ParticleContactStreamIterator
{
	C_ContactStream* m_pStream;
	U8* m_pCurrentPointer;
	C_ParticleContactPair* m_pCurrentPair;
	C_ParticleContactStorage* m_pCurrentContact;
	C_ParticleContactStorage* m_pNextContact;
	S32 m_ContactCount;
public:
	C_ParticleContactStreamIterator(C_ContactStream* pStream);

	C_ParticleContactStreamIterator(C_ConstContactStreamPtr* pStream);

	bool HasNextResult();
	C_ParticleContactPair* NextResult();
	bool HasNextContact();
	C_ParticleContactStorage* NextContact();
};

};

#include "ContactStreamIterator.inl"

#endif

