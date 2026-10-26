#pragma once

#include "geometry.h"
#include "rigidbody.h"

typedef float CONSTRAINT_MATRIX_FLOAT;

namespace RacingPhysics
{

class ConstraintSolver
{
public:
	// using these to avoid allocating memory in the game loop
	const static int MAX_CONSTRAINTS_PER_BODY = 16;
	const static int MAX_CONSTRAINTS = 28;
	const static int MAX_BODIES = 6;

	ConstraintSolver();
	~ConstraintSolver();

	void Initialise(int nBodies, int nConstraints);
	void Clear();

	// Add body to be constained.
	void AddRigidBody(const RigidBody* p);
	void AddRotor(const Rotor* p);

	// Set up constraints using global coordinates.
	void SetDefaultRigidBodies(int rigidBodyIndex0, int rigidBodyIndex1);

	void SetVelocityCorrection(float timeStep) {m_velocityCorrection = true; m_timeStep = timeStep;}

	void ConstrainPositions(const Normal3R& direction, const Point3R& position0, const Point3R& position1,
							int rigidBodyIndex0 = -1, int rigidBodyIndex1 = -1);

	void ConstrainAxis(const Normal3R& direction, const Normal3R& axis0, const Normal3R& axis1, 
							int rigidBodyIndex0 = -1, int rigidBodyIndex1 = -1);

	void ConstrainAxis(const Normal3R& direction, int rigidBodyIndex0 = -1, int rigidBodyIndex1 = -1);

	void AddConstraintTorque(const Normal3R& direction, float relativeVelocity = 0.f, int rigidBodyIndex0 = -1, int rigidBodyIndex1 = -1);

	void AddConstraintForce(const Normal3R& direction, 
							const Point3R& position, 
							float relativeVelocity = 0.f, 
							int rigidBodyIndex0 = -1, int rigidBodyIndex1 = -1);

	// Max and min values for the constraint impulse.
	void SetConstraintMinMax(float min, float max);

	// Solve for the constraints.
	bool Solve();

	// Get the magnitude of constraint i;
	enum ConstraintType
	{
		CONSTRAINT_TYPE_FORCE,
		CONSTRAINT_TYPE_COUPLE,
	};

	int GetNumConstraints() const {return m_constraints.Size();}
	FloatVR GetConstraintSize(int i) const 
	{
		RP_ASSERT(i >= 0 && i < m_constraints.Size());
		const CONSTRAINT_MATRIX_FLOAT f = m_pResult[i];
		RP_ASSERT(f >= m_constraints[i].min && f <= m_constraints[i].max);
		return FloatVR(m_pResult[i]);
	}
	Vector3R GetConstraint(int i) const {return m_constraints[i].direction * GetConstraintSize(i);}
	Point3R GetConstraintPosition(int i) {return m_constraints[i].position;}
	ConstraintType GetConstraintType(int i) {return m_constraints[i].type;}
	int GetConstraintBodyIndex0(int i) {return m_constraints[i].rigidBodyIndex0;}
	int GetConstraintBodyIndex1(int i) {return m_constraints[i].rigidBodyIndex1;}

private:
	void AddConstraint(const Normal3R& direction, float relativeVelocity, int rigidBodyIndex0, int rigidBodyIndex1);
	void BuildMatrix();
	void EliminateConstraint(int i, float f);

	float& A(int i, int j);
	float& b(int j);

	float* __restrict m_pMatrixData;
	CONSTRAINT_MATRIX_FLOAT* __restrict m_pResult;
	CONSTRAINT_MATRIX_FLOAT* __restrict m_pTempMatrixData;
	int* __restrict m_pActive;
	int m_nConstraints;

	int m_rigidBodyIndex0;
	int m_rigidBodyIndex1;
	float m_timeStep;
	bool m_velocityCorrection;
	bool m_limits;

	struct Constraint
	{
		Normal3R direction;
		Point3R position;
		float relativeVelocity;
		int rigidBodyIndex0;
		int rigidBodyIndex1;
		ConstraintType type;
		float min;
		float max;

		// solver
		bool active;
		float fixedValue;
	};

	struct BodyInfo
	{
		const RigidBody* pBody;
		const Rotor* pRotor;

		Btl::Vector<int, Btl::VectorParameters_Static<MAX_CONSTRAINTS_PER_BODY> > m_constraints;
		Btl::Vector<float, Btl::VectorParameters_Static<MAX_CONSTRAINTS_PER_BODY> > m_constraintSign;
	};

	Btl::Vector<BodyInfo, Btl::VectorParameters_Static<MAX_BODIES> > m_rigidBodyInfo;
	Btl::Vector<Constraint, Btl::VectorParameters_Static<MAX_CONSTRAINTS> > m_constraints;
};

}
