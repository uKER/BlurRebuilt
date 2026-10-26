#ifndef PERSISTENTCONTACTMANIFOLD_H
#define PERSISTENTCONTACTMANIFOLD_H


namespace Collision
{

	class C_ManifoldContact
	{
	public:


		Maths::HVec3 m_LocalContactA; // won't be changed
		Maths::HVec3 m_LocalContactB; // won't be changed
		Maths::HVec3 m_Normal; 
		SharedPhysics::C_ConstraintPoint* m_pConstraint;
		U32 m_HashValue;
		U32 m_PhysicsMaterialIndices;
		F32 m_PenetrationDepth;
		F32 m_OriginalPenetration;
		F32 m_Baumgarte;
		//bool m_NewPoint;

		C_ManifoldContact() : m_LocalContactA(Maths::UNDEFINED), m_LocalContactB(Maths::UNDEFINED),
			m_Normal(Maths::UNDEFINED)
		{
		}


		C_ManifoldContact& operator = (const C_ManifoldContact& contact)
		{
			m_LocalContactA = contact.m_LocalContactA;
			m_LocalContactB = contact.m_LocalContactB;
			m_Normal = contact.m_Normal;
			//m_AverageNormal = contact.m_AverageNormal;
			m_pConstraint = contact.m_pConstraint;
			m_HashValue = contact.m_HashValue;
			m_PhysicsMaterialIndices = contact.m_PhysicsMaterialIndices;
			m_PenetrationDepth = contact.m_PenetrationDepth;
			m_OriginalPenetration = contact.m_OriginalPenetration;
			m_Baumgarte = contact.m_Baumgarte;
			//m_NewPoint = contact.m_NewPoint;
			return *this;
		}

	};

	const static U32 MaxManifoldContacts = 4;

	class C_PersistentContactManifold
	{
	public:
		C_PersistentContactManifold() :  m_NumContacts(0), m_SqContactBreakingThreshold(0.02f * 0.02f),
			m_SqContactBreakingThreshold2(0.05f * 0.05f)
		{
		}

		C_PersistentContactManifold(const C_PersistentContactManifold& manifold) : m_SqContactBreakingThreshold(Maths::UNDEFINED),
			m_SqContactBreakingThreshold2(Maths::UNDEFINED)
		{
			SetManifold(manifold);
		}

		~C_PersistentContactManifold()
		{
		}

		C_PersistentContactManifold& operator = (const C_PersistentContactManifold& manifold)
		{
			SetManifold(manifold);
			return *this;
		}

		void FlipNormals()
		{
			for(U32 a = 0; a < m_NumContacts; ++a)
			{
				m_Manifold[a].m_Normal = -m_Manifold[a].m_Normal;
				//m_Manifold[a].m_AverageNormal = -m_Manifold[a].m_AverageNormal;
				const Maths::HVec3 localA = m_Manifold[a].m_LocalContactA;
				m_Manifold[a].m_LocalContactA = m_Manifold[a].m_LocalContactB;
				m_Manifold[a].m_LocalContactB = localA;
			}
		}

		void SetManifold(const C_PersistentContactManifold& manifold)
		{
			BIZ_ASSERT(manifold.m_NumContacts <= MaxManifoldContacts);
			m_NumContacts = manifold.m_NumContacts;
			m_SqContactBreakingThreshold2 = manifold.m_SqContactBreakingThreshold2;
			m_SqContactBreakingThreshold = manifold.m_SqContactBreakingThreshold;
			m_pResult = manifold.m_pResult;
			for(U32 a =0 ; a < m_NumContacts; ++a)
			{
				m_Manifold[a] = manifold.m_Manifold[a];
			}
		}


		void CalculateQuadrilateralArea(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d, Maths::HF32& sqArea);

		void SetContactBreakingThreshold(const Maths::HF32 PCREF threshold, const Maths::HF32 PCREF threshold2)
		{
			m_SqContactBreakingThreshold2 = threshold2 * threshold2;
			m_SqContactBreakingThreshold = threshold * threshold;
		}

		void Reset()
		{
			//m_MaxNumContacts = MaxManifoldContacts;
			m_NumContacts = 0;
		}
		
		//S32 GetEntryIndex(C_ManifoldContact& contact);
		void AddManifoldContact( C_ManifoldContact& contact);
		void ReduceManifoldContacts(C_ManifoldContact& contact);
		void RefreshManifoldContacts(const Maths::HMat43 PCREF transA, const Maths::HMat43 PCREF transB);
		void RemoveManifoldContact(const U32 index);


		C_ManifoldContact m_Manifold[MaxManifoldContacts];
		SharedPhysics::C_ContactStreamResult* m_pResult;
		Maths::HF32 m_SqContactBreakingThreshold2;
		Maths::HF32 m_SqContactBreakingThreshold;
		//U32 m_MaxNumContacts;
		U32 m_NumContacts;
	};
}

#endif
