#ifndef GRAVITYPARTICLE_H
#define GRAVITYPARTICLE_H

#include "gravity/sharedphysics/safestack.h"
#include "gravity/sharedPhysics/GraphNode.h"

namespace Gravity
{
	struct C_ExternalConstants;
	class C_Body;
}

namespace Collision
{

	const U32 SoftBodyContactSize = 64;

	enum C_ParticleSystemType
	{
		E_ClothSystem,
		E_RopeSystem,
		E_DeformableSystem
	};

	class C_GravityParticle
	{
	public:
		C_GravityParticle() : m_Active(true), m_CurPosition(Maths::Constants<Maths::HVec3>::Zero()),
			m_PrePosition(Maths::Constants<Maths::HVec3>::Zero()), m_PreConstraintPosition(Maths::Constants<Maths::HVec3>::Zero()),
			m_ExternalForce(Maths::Constants<Maths::HVec3>::Zero()), m_AverageNormal(Maths::Constants<Maths::HVec3>::Zero()),
			m_NormalCount(Maths::Constants<Maths::HF32>::Zero()),m_Mass(Maths::Constants<Maths::HF32>::One()),
			m_MassInverse(Maths::Constants<Maths::HF32>::One())
		{
		}

		virtual ~C_GravityParticle()
		{
		}

	
		void SetExternalForce(const Maths::HVec3 PCREF force)
		{
			m_ExternalForce = force;
		}

		void AddExternalForce(const Maths::HVec3 PCREF force)
		{
			m_ExternalForce += force;
		}

		void ClearExternalForce()
		{
			m_ExternalForce=Maths::Constants<Maths::HVec3>::Zero();
		}

		void MakeStatic()
		{
			m_Mass = Maths::Constants<Maths::HF32>::Zero();
			m_MassInverse = m_Mass;
		}

		void SetMass(const Maths::HF32 PCREF mass)
		{
			m_Mass = mass;
			m_MassInverse = mass.Reciprocal();
		}

		template <typename Serial>
		void Serialise(Serial& s) const
		{
			s << m_CurPosition << m_Mass << m_MassInverse;
		}

		template <typename Serial>
		void Deserialise(Serial& s)
		{
			s >> m_CurPosition >> m_Mass >> m_MassInverse;
		}

		const Maths::HVec3 Velocity(const Maths::HF32 PCREF recipTime)
		{
			return (m_CurPosition - m_PrePosition) * recipTime;
		}


		//Update data in particle
		Maths::HVec3 m_CurPosition;
		Maths::HVec3 m_PrePosition;
		Maths::HVec3 m_PreConstraintPosition;
		Maths::HVec3 m_ExternalForce;
		//Particle normal data (for cloth triangles only)
		Maths::HVec3 m_AverageNormal;
		Maths::HF32 m_NormalCount;
		//Particle mass data
		Maths::HF32 m_Mass;
		Maths::HF32 m_MassInverse;
		bool m_Active;
		
	};


	class C_ParticleSystem
	{
	public:
		C_ParticleSystem(const U32 type) :  m_numParticles(0), m_pParticles(NULL),m_pExternalConstants(NULL), 
			m_Type(type), m_Thickness(Maths::Constants<Maths::HF32>::Zero()), m_Damping(Maths::Constants<Maths::HF32>::One()),
			m_MaxSpeedSq(Maths::Constants<Maths::HF32>::Zero())
		{
		}

		virtual ~C_ParticleSystem()
		{
			if(m_pParticles != NULL)
			{
				delete[] m_pParticles;
			}
		}

		void Initialise()
		{
			for(U32 i=0; i<m_numParticles; ++i)
			{
				m_pParticles[i].m_PrePosition = m_pParticles[i].m_CurPosition;
			}
		}

		void SetExternalConstants(Gravity::C_ExternalConstants* pConstants)
		{
			m_pExternalConstants = pConstants;
		}

		void SetThickness(const Maths::HF32 PCREF thickness)
		{
			m_Thickness = thickness;
		}

		void SetDampingCoefficent(const Maths::HF32 PCREF damping)
		{
			m_Damping = damping;
		}

		void SetParticleMass(const Maths::HF32 PCREF mass)
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			for(U32 i=0; i<m_numParticles; ++i)
			{
				if(m_pParticles[i].m_Mass.GT(zero))
					m_pParticles[i].SetMass(mass);
			}
		}

		//This method accumulate all the external force for each particle
		void AccumulateForces();

		virtual void Verlet(const Maths::HF32 PCREF time);

		
		C_GravityParticle* m_pParticles;
		Gravity::C_ExternalConstants* m_pExternalConstants;

		Maths::HF32 m_MaxSpeedSq;
		Maths::HF32 m_Thickness;//what is the radius of the particles
		Maths::HF32 m_Damping; // for the verlet intergation

		U32 m_numParticles;
		U32 m_Type;//what type of particle system is it
		
	};

	struct C_ConstrainedParticleEdge
	{
		U32 m_IndexA, m_IndexB;
		F32 m_RestLength;
		bool m_Active;
		C_ConstrainedParticleEdge() : m_Active(true)
		{
		}
	};

	//A class which makes a particle follow a specific body's local space...
	//NB. Any particle which has this applied, must have infinite mass!!!
	struct C_ParticleJoint
	{
		Maths::HVec3 m_RelativePosition;
		Gravity::C_Body* m_pBody;
		U32 m_ParticleIndex;

		C_ParticleJoint() : m_RelativePosition(Maths::UNDEFINED)
		{
		}
	};

	class C_ConstrainedParticleSystem : public C_ParticleSystem
	{
	public:
		//Maths::HF32 m_StretchCoefficient; //should not more than 10% of the rest length
		//Maths::HF32 m_CompressCoefficient;//should not less than 10% of the rest length
		//Maths::HF32 m_TearThreshold;	  //A scale of the length, at which point an edge will tear
		C_ConstrainedParticleEdge* m_pEdges;
		C_ParticleJoint* m_pJoints;

		//Particle header gets the number of edges which are associated with a particle
		U32* m_pParticleHeader;
		//Particle edge table tells you the indices of the edges which connect to a specific triangle
		S32* m_pParticleEdgeTable;

		Maths::HF32 m_StretchCoefficient; 
		Maths::HF32 m_CompressCoefficient;

		U32 m_numEdges;
		U32 m_numJoints;
		U32 m_maxJoints;
		U32 m_numIterations;


		C_ConstrainedParticleSystem(U32 type, U32 numIteration = 5) : C_ParticleSystem(type),m_pEdges(NULL), m_pJoints(NULL),
			m_numEdges(0), m_numJoints(0), m_maxJoints(0), m_numIterations(numIteration),
			m_pParticleHeader(NULL), m_pParticleEdgeTable(NULL), m_StretchCoefficient(1.05f),m_CompressCoefficient(0.95f)
		{
		}

		virtual ~C_ConstrainedParticleSystem()
		{
			if(m_pEdges)
				delete [] m_pEdges;
			if(m_pJoints)
				delete [] m_pJoints;
			if(m_pParticleHeader)
				delete [] m_pParticleHeader;
			if(m_pParticleEdgeTable)
				delete [] m_pParticleEdgeTable;
			m_pEdges = NULL;
			m_pJoints = NULL;
			m_pParticleHeader = NULL;
			m_pParticleEdgeTable = NULL;
		}

		void SetNumberJoints(const U32 numJoints)
		{
			if(m_pJoints)
				delete [] m_pJoints;
			m_maxJoints = numJoints;
			m_pJoints = new C_ParticleJoint[numJoints];
		}

		void ConnectParticle(U32 particleIndex, Gravity::C_Body* pBody, const Maths::HVec3 PCREF relativePosition);


	/*	void SetStretchCoefficient(const Maths::HF32 PCREF stretch)
		{
			m_StretchCoefficient = stretch;
		}

		void SetCompressCoefficient(const Maths::HF32 PCREF compress)
		{
			m_CompressCoefficient = compress;
		}

		void SetTearThreshold(const Maths::HF32 PCREF tearThreshold)
		{
			m_TearThreshold = tearThreshold;
		}*/

		void CalculateParticleEdgeData();

		void RemoveEdge(U32 edgeIndex);

		void SatisfyConstraints();

		//void SatisfyConstraints();
	};

	class C_ParticleContact
	{
	public:
		enum E_ParticleConstraintType
		{
			E_TriangleConstraint,
			E_VertexConstraint
		};
		SharedPhysics::C_GraphNode* m_pParticlePtr;
		SharedPhysics::C_GraphNode* m_pNodePtr;
		Maths::HVec3 m_Contact;
		//Maths::HVec3 m_RemapContact;
		Maths::C_Normal m_EscapeVector;
		Maths::C_Normal m_TriangleNormal;
		Maths::HF32 m_Penetration;
		//Maths::HF32 m_RemapPenetration;
		Maths::HF32 m_Stiffness;
		U32 m_Index;
		U32 m_Type;


		//Maths::HVec3 m_NormalVelocity;

		C_ParticleContact() : m_EscapeVector(Maths::UNDEFINED), m_TriangleNormal(Maths::UNDEFINED), m_Penetration(Maths::UNDEFINED), 
			m_Stiffness(Maths::UNDEFINED), m_Type(E_VertexConstraint), m_Contact(Maths::UNDEFINED)
		{
		}
	};

	template <U32 size>
	class C_ParticleContactBuffer
	{
	public:
		C_ParticleContact m_Contacts[size];
		U32 m_NumContacts;

		C_ParticleContactBuffer() : m_NumContacts(0)
		{
		}

		void AddContact(SharedPhysics::C_GraphNode* pParticleNode, SharedPhysics::C_GraphNode* pNodePtr, const Maths::HVec3 PCREF contact,
			const Maths::C_Normal PCREF escapeVector, const Maths::C_Normal PCREF triangleNormal, const Maths::HF32 PCREF penetration,
			const Maths::HF32 PCREF stiffness, U32 triangleIndex, C_ParticleContact::E_ParticleConstraintType contactType)
		{
			if(m_NumContacts == size)
				return;

			Collision::C_ParticleContact& constraint = m_Contacts[m_NumContacts++];
			constraint.m_Type = C_ParticleContact::E_TriangleConstraint;
			constraint.m_Contact = contact;
			constraint.m_EscapeVector = escapeVector;
			constraint.m_TriangleNormal = triangleNormal;
			constraint.m_Index = triangleIndex;
			constraint.m_Penetration = penetration;
			constraint.m_pNodePtr = pNodePtr;
			constraint.m_pParticlePtr = pParticleNode;
			constraint.m_Stiffness = stiffness;
		}
	};


}

#endif
