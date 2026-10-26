#include "stdafx.h"
#include "racingphysics/physics/constraints.h"

//using namespace Maths;

/////////////////////////////////////////////////////

//#pragma optimize("", off)

namespace RacingPhysics
{

ConstraintSolver::ConstraintSolver()
{
	m_pMatrixData = 0;
	m_pResult = 0;
	m_timeStep = 0.f;
	m_velocityCorrection = false;
	m_limits = false;

#if !PLATFORM_SPU

	m_pMatrixData = new float[MAX_CONSTRAINTS * (MAX_CONSTRAINTS + 1)];
	m_pResult = new CONSTRAINT_MATRIX_FLOAT[MAX_CONSTRAINTS];

	m_pTempMatrixData = new CONSTRAINT_MATRIX_FLOAT[MAX_CONSTRAINTS * (MAX_CONSTRAINTS + 1)];
	m_pActive = new int[MAX_CONSTRAINTS + 1];

#else

	static float matrixData[MAX_CONSTRAINTS * (MAX_CONSTRAINTS + 1)];
	static CONSTRAINT_MATRIX_FLOAT result[MAX_CONSTRAINTS];

	static CONSTRAINT_MATRIX_FLOAT tempMatrixData[MAX_CONSTRAINTS * (MAX_CONSTRAINTS + 1)];
	static int active[MAX_CONSTRAINTS + 1];

	m_pMatrixData = matrixData;
	m_pResult = result;

	m_pTempMatrixData = tempMatrixData;
	m_pActive = active;

#endif
}

ConstraintSolver::~ConstraintSolver() 
{
#if !PLATFORM_SPU
	delete m_pMatrixData;
	delete m_pResult;
	delete m_pTempMatrixData;
	delete m_pActive;
#endif
}

void ConstraintSolver::Initialise(int nBodies, int nConstraints)
{
	RP_ASSERT(m_rigidBodyInfo.Size() == 0 && m_constraints.Size() == 0);
	m_rigidBodyInfo.Reserve(nBodies);
	m_constraints.Reserve(nConstraints);

	m_timeStep = 0.f;
	m_velocityCorrection = false;
	m_limits = false;
}

void ConstraintSolver::Clear()
{
	m_rigidBodyInfo.Clear();
	m_constraints.Clear();
}

void ConstraintSolver::AddRigidBody(const RigidBody* p)
{
	m_rigidBodyInfo.PushBack();
	BodyInfo& bodyInfo(m_rigidBodyInfo.Back());

	bodyInfo.pBody = p;
	bodyInfo.pRotor = 0;
}

void ConstraintSolver::AddRotor(const Rotor* p)
{
	m_rigidBodyInfo.PushBack();
	BodyInfo& bodyInfo(m_rigidBodyInfo.Back());

	bodyInfo.pBody = 0;
	bodyInfo.pRotor = p;
}

void ConstraintSolver::SetDefaultRigidBodies(int rigidBodyIndex0, int rigidBodyIndex1)
{
	m_rigidBodyIndex0 = rigidBodyIndex0;
	m_rigidBodyIndex1 = rigidBodyIndex1;
}

void ConstraintSolver::ConstrainPositions(const Normal3R& direction, 
										  const Point3R& position0, const Point3R& position1, 
										  int rigidBodyIndex0, int rigidBodyIndex1)
{
	float dp = 0;
	
	// set the final velocity to correct errors in the position
	if (m_velocityCorrection)
		dp = ((position1 - position0) | direction) / m_timeStep;

	AddConstraintForce(direction, position0, dp, rigidBodyIndex0, rigidBodyIndex1);
}

void ConstraintSolver::ConstrainAxis(const Normal3R& direction, 
										  const Normal3R& axis0, const Normal3R& axis1, 
										  int rigidBodyIndex0, int rigidBodyIndex1)
{
	float da = 0;
	
	// set the final velocity to correct errors in the position
	if (m_velocityCorrection)
		da = ((axis0 ^ axis1) | direction) / m_timeStep;

	AddConstraintTorque(direction, da, rigidBodyIndex0, rigidBodyIndex1);
}

void ConstraintSolver::ConstrainAxis(const Normal3R& direction, int rigidBodyIndex0, int rigidBodyIndex1)
{
	if (!m_velocityCorrection)
		AddConstraintTorque(direction, 0.f, rigidBodyIndex0, rigidBodyIndex1);
}

void ConstraintSolver::AddConstraint(const Normal3R& direction, float relativeVelocity, int rigidBodyIndex0, int rigidBodyIndex1)
{
	m_constraints.PushBack();

	Constraint& constraint(m_constraints.Back());

	constraint.direction = direction;
	constraint.relativeVelocity = relativeVelocity;

	if (rigidBodyIndex0 == -1) // use defaults
	{
		rigidBodyIndex0 = m_rigidBodyIndex0;
		rigidBodyIndex1 = m_rigidBodyIndex1;
	}

	constraint.rigidBodyIndex0 = rigidBodyIndex0;
	constraint.rigidBodyIndex1 = rigidBodyIndex1;

	constraint.active = true;
	constraint.min = -FLT_MAX;
	constraint.max = FLT_MAX;

	{
		BodyInfo& bodyInfo = m_rigidBodyInfo[constraint.rigidBodyIndex0];
		bodyInfo.m_constraints.PushBack(m_constraints.Size() - 1);
		bodyInfo.m_constraintSign.PushBack(1.f);

		if (m_constraints.Back().rigidBodyIndex1 > -1)
		{
			BodyInfo& bodyInfo = m_rigidBodyInfo[constraint.rigidBodyIndex1];
			bodyInfo.m_constraints.PushBack(m_constraints.Size() - 1);
			bodyInfo.m_constraintSign.PushBack(-1.f);
		}
	}

}

void ConstraintSolver::AddConstraintTorque(const Normal3R& direction, float relativeVelocity, int rigidBodyIndex0, int rigidBodyIndex1)
{
	AddConstraint(direction, relativeVelocity, rigidBodyIndex0, rigidBodyIndex1);
	m_constraints.Back().type = CONSTRAINT_TYPE_COUPLE;
}

void ConstraintSolver::AddConstraintForce(const Normal3R& direction, const Point3R& position, float relativeVelocity, 
										  int rigidBodyIndex0, int rigidBodyIndex1)
{
	AddConstraint(direction, relativeVelocity, rigidBodyIndex0, rigidBodyIndex1);
	m_constraints.Back().position = position;
	m_constraints.Back().type = CONSTRAINT_TYPE_FORCE;
}

void ConstraintSolver::SetConstraintMinMax(float min, float max)
{
	RP_ASSERT(max >= min);
	m_constraints.Back().min = min;
	m_constraints.Back().max = max;
	m_limits = true;
}

typedef CONSTRAINT_MATRIX_FLOAT FLOAT;
bool GESolve(FLOAT* A, FLOAT* x, int nRows);
bool GESolve2(FLOAT* A, FLOAT* x, int nRows, int* active);

bool ConstraintSolver::Solve()
{
	BuildMatrix();

	bool solved = false;
	int size = (m_nConstraints + 1) * m_nConstraints;

	if (!m_limits)
	{
		for (int i = 0; i < size; i++)
			m_pTempMatrixData[i] = m_pMatrixData[i];

		bool valid = GESolve(m_pTempMatrixData, m_pResult, m_nConstraints);
		RP_ASSERT(valid);
	}
	else
	{
		while(!solved && m_nConstraints > 0)
		{
			{
				//memcpy(m_pTempMatrixData, m_pMatrixData, size * sizeof(float));

				for (int i = 0; i < size; i++)
					m_pTempMatrixData[i] = m_pMatrixData[i];

				int nDisabled = 0;
				bool valid = false;

				for (int i = 0; i < m_nConstraints; i++)
					if (!m_constraints[i].active) nDisabled++;

				if (nDisabled == 0)
				{
					valid = GESolve(m_pTempMatrixData, m_pResult, m_nConstraints);
				}
				else
				{
					for (int i = 0; i < m_nConstraints; i++)
						m_pActive[i] = m_constraints[i].active;

					m_pActive[m_nConstraints] = 1;

					valid = GESolve2(m_pTempMatrixData, m_pResult, m_nConstraints, m_pActive);
				}
				
				if (!valid) return false;
				solved = true;

				//delete [] tmpMatrixData;
			}		

			for (int i = 0; i < m_constraints.Size(); i++)
			{
				if (!m_constraints[i].active)
					m_pResult[i] = m_constraints[i].fixedValue;
			}

			// check limits
			for (int i = 0; i < m_constraints.Size(); i++)
			{
				if (m_constraints[i].active)
				{
					if (m_pResult[i] < m_constraints[i].min)
					{
						EliminateConstraint(i, m_constraints[i].min);
						solved = false;
						continue;
					}
					else if (m_pResult[i] > m_constraints[i].max)
					{
						EliminateConstraint(i, m_constraints[i].max);
						solved = false;
						continue;
					}
				}
			}
		}
	}

	return true;
}

void ConstraintSolver::EliminateConstraint(int c, float f)
{
	m_constraints[c].active = false;
	m_constraints[c].fixedValue = f;

	if (f != 0.f)
	{
		for (int i = 0; i < m_constraints.Size(); i++)
			b(i) -= A(c, i) * (m_constraints[c].fixedValue);
	}

	m_pResult[c] = m_constraints[c].fixedValue;
}

float& ConstraintSolver::A(int i, int j) {return m_pMatrixData[i + j * (m_nConstraints + 1)];}
float& ConstraintSolver::b(int j) {return m_pMatrixData[m_nConstraints + j * (m_nConstraints + 1)];}

void ConstraintSolver::BuildMatrix()
{
	m_nConstraints = m_constraints.Size();
	int matrixSize = (m_nConstraints + 1) * m_nConstraints;

	memset(m_pMatrixData, 0, matrixSize * sizeof(float));

	for (int i = 0; i < m_constraints.Size(); i++)
	{
		int i0 = m_constraints[i].rigidBodyIndex0;
		int i1 = m_constraints[i].rigidBodyIndex1;

		const RigidBody* __restrict pb0 = m_rigidBodyInfo[i0].pBody;
		const RigidBody* __restrict pb1 = 0;
		if (i1 > -1) pb1 = m_rigidBodyInfo[i1].pBody;

		const Normal3R& di = m_constraints[i].direction;

		if (m_constraints[i].type == CONSTRAINT_TYPE_FORCE)
		{
			Vector3R r0i = m_constraints[i].position - pb0->CofG();

			// constant term
			FloatVR v = FloatVR(m_constraints[i].relativeVelocity);

			if (!m_velocityCorrection)
			{
				v -= (pb0->Velocity() + (pb0->AngularVelocity() ^ r0i)) | di;
				if (pb1)
				{
					Vector3R r1i = m_constraints[i].position - pb1->CofG();
					v += (pb1->Velocity() + (pb1->AngularVelocity() ^ r1i)) | di;
				}
			}

			Maths::Store(v, &b(i));

			RP_ASSERT(Maths::Finite(b(i)));
		}
		else // torque
		{
			const Rotor* __restrict pg0 = m_rigidBodyInfo[i0].pRotor;

			const Rotor* __restrict pg1 = 0;
			if (i1 > -1) pg1 = m_rigidBodyInfo[i1].pRotor;

			FloatVR v = FloatVR(m_constraints[i].relativeVelocity);

			// constant term
			if (!m_velocityCorrection)
			{
				if (pb0) v -= pb0->AngularVelocity() | di;
				else if (pg0) v -= pg0->AngularVelocity() | di;

				if (pb1) v += pb1->AngularVelocity() | di;
				else if (pg1) v += pg1->AngularVelocity() | di;
			}

			Maths::Store(v, &b(i));

			if (b(i) != 0)
				int breakhere = 0;

			RP_ASSERT(Maths::Finite(b(i)));
		}
	}

	for (int i = 0; i < m_constraints.Size(); i++)
	{
		int i0 = m_constraints[i].rigidBodyIndex0;
		int i1 = m_constraints[i].rigidBodyIndex1;

		const RigidBody* __restrict pb0 = m_rigidBodyInfo[i0].pBody;
		const RigidBody* __restrict pb1 = 0;
		if (i1 > -1) pb1 = m_rigidBodyInfo[i1].pBody;

		const Normal3R& di = m_constraints[i].direction;

		if (m_constraints[i].type == CONSTRAINT_TYPE_FORCE)
		{
			Vector3R r0i = m_constraints[i].position - pb0->CofG();

			for (int j = 0; j < m_rigidBodyInfo[i0].m_constraints.Size(); j++)
			{
				int cj = m_rigidBodyInfo[i0].m_constraints[j];
				FloatVR sj = FloatVR(m_rigidBodyInfo[i0].m_constraintSign[j]);

				const Normal3R& dj = m_constraints[cj].direction;

				if (m_constraints[cj].type == CONSTRAINT_TYPE_FORCE)
				{
					Vector3R r0j = m_constraints[cj].position - pb0->CofG();

					FloatVR f = (di | dj) / pb0->Mass()
							+ (r0i ^ Vector3R(pb0->InverseMIGlobal() * (dj ^ r0j)) | di);

					A(cj, i) += sj * f;
				}
				else
				{
					FloatVR f = r0i ^ Vector3R(pb0->InverseMIGlobal() * dj) | di;
					A(cj, i) -= sj * f;
				}
			}

			if (pb1)
			{
				Vector3R r1i = m_constraints[i].position - pb1->CofG();

				for (int j = 0; j < m_rigidBodyInfo[i1].m_constraints.Size(); j++)
				{
					int cj = m_rigidBodyInfo[i1].m_constraints[j];
					FloatVR sj = FloatVR(m_rigidBodyInfo[i1].m_constraintSign[j]);

					const Normal3R& dj = m_constraints[cj].direction;

					if (m_constraints[cj].type == CONSTRAINT_TYPE_FORCE)
					{
						Vector3R r1j = m_constraints[cj].position - pb1->CofG();

						FloatVR f = (di | dj) / pb1->Mass()
								+ (r1i ^ Vector3R(pb1->InverseMIGlobal() * (dj ^ r1j)) | di);

						A(cj, i) += (-sj) * f;
					}
					else
					{
						FloatVR f = r1i ^ Vector3R(pb1->InverseMIGlobal() * dj) | di;
						A(cj, i) -= (-sj) * f;
					}
				}
			}
		}
		else // torque
		{
			const Rotor* __restrict pg0 = m_rigidBodyInfo[i0].pRotor;

			const Rotor* __restrict pg1 = 0;
			if (i1 > -1) pg1 = m_rigidBodyInfo[i1].pRotor;

			if (pb0)
			{
				for (int j = 0; j < m_rigidBodyInfo[i0].m_constraints.Size(); j++)
				{
					int cj = m_rigidBodyInfo[i0].m_constraints[j];
					FloatVR sj = FloatVR(m_rigidBodyInfo[i0].m_constraintSign[j]);

					const Normal3R& dj = m_constraints[cj].direction;

					if (m_constraints[cj].type == CONSTRAINT_TYPE_FORCE)
					{
						Vector3R r0j = m_constraints[cj].position - pb0->CofG();
						FloatVR f = (pb0->InverseMIGlobal() * (dj ^ r0j)) | di;
						A(cj, i) -= sj * f;
					}
					else
					{
						FloatVR f = (pb0->InverseMIGlobal() * dj) | di;
						A(cj, i) += sj * f;
					}
				}
			}
			else if (pg0)
			{
				for (int j = 0; j < m_rigidBodyInfo[i0].m_constraints.Size(); j++)
				{
					int cj = m_rigidBodyInfo[i0].m_constraints[j];
					FloatVR sj = FloatVR(m_rigidBodyInfo[i0].m_constraintSign[j]);

					const Normal3R& dj = m_constraints[cj].direction;

					FloatVR f = (dj | di) / pg0->MI();
					A(cj, i) += sj * f;
				}
			}

			if (pb1)
			{
				for (int j = 0; j < m_rigidBodyInfo[i1].m_constraints.Size(); j++)
				{
					int cj = m_rigidBodyInfo[i1].m_constraints[j];
					FloatVR sj = FloatVR(m_rigidBodyInfo[i1].m_constraintSign[j]);

					const Normal3R& dj = m_constraints[cj].direction;

					if (m_constraints[cj].type == CONSTRAINT_TYPE_FORCE)
					{
						Vector3R r1j = m_constraints[cj].position - pb1->CofG();
						FloatVR f = (pb1->InverseMIGlobal() * (dj ^ r1j)) | di;
						A(cj, i) -= (-sj) * f;
					}
					else
					{
						FloatVR f = (pb1->InverseMIGlobal() * dj) | di;
						A(cj, i) += (-sj) * f;
					}
				}
			}
			else if (pg1)
			{
				for (int j = 0; j < m_rigidBodyInfo[i1].m_constraints.Size(); j++)
				{
					int cj = m_rigidBodyInfo[i1].m_constraints[j];
					FloatVR sj = FloatVR(m_rigidBodyInfo[i1].m_constraintSign[j]);

					const Normal3R& dj = m_constraints[cj].direction;

					FloatVR f = (dj | di) / pg1->MI();
					A(cj, i) += (-sj) * f;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////

class MatrixHelper
{
public:
	MatrixHelper(FLOAT* p, int nRows, int nColumns) : m_p(p), m_nRows(nRows), m_nColumns(nColumns) {}

	__forceinline FLOAT& operator () (int i, int j) 
	{
		RP_ASSERT(i >= 0 && j >= 0 && i < m_nRows && j < m_nColumns);
		return m_p[j + i * m_nColumns];
	}

	__forceinline const FLOAT& operator () (int i, int j) const 
	{
		RP_ASSERT(i >= 0 && j >= 0 && i < m_nRows && j < m_nColumns);
		return m_p[j + i * m_nColumns];
	}

private:
	FLOAT* __restrict m_p;
	int m_nRows;
	int m_nColumns;
};

// Solve an equation in the form Ax = b.
// Input (N+1)xN matrix [Ab].
// Destroys matrix, returns result in x.
// Returns false if the matrix was singular.
bool GESolve(FLOAT* __restrict A, FLOAT* __restrict x, int nRows)
{
	FLOAT epsilon = 0.00001f;
	int nColumns = nRows + 1;

	MatrixHelper a(A, nRows, nColumns);

	FLOAT fi = 0.f;

	for (int i = 0; i < nRows; i++, fi += 1.f) 
	{
		// Find the row with the largest first value.
		//int maxrowi = i;
		FLOAT maxrowf = fi;
		FLOAT maxval = Abs(a(i, i));
		FLOAT fj = fi + 1.f;

		for (int j = i + 1; j < nRows; j++, fj += 1.f) 
		{
			FLOAT val = Abs(a(j, i));
			maxrowf = (FLOAT) Maths::Select_GE_Zero(val - maxval, fj, maxrowf);
			maxval = (FLOAT) Maths::Select_GE_Zero(val - maxval, val, maxval);

			//if (val > maxval)
			//{
			//	maxrowi = j;
			//	maxval = val;
			//}
		}
			
		int maxrowi = (int) (maxrowf + 0.5f);

		// Swap the maxrow and ith row.
		for (int j = 0; j < nColumns; j++)
		{
			FLOAT tmp = a(i, j);
			a(i, j) = a(maxrowi, j);
			a(maxrowi, j) = tmp;
		}
		
#ifndef CFG_SPEED
		// Singular matrix?
		if (Abs(a(i, i)) < epsilon)
		{
			//DBPrint("GESolve::Singular Matrix, %i", i);
			return false;
		}
#endif

		FLOAT invMax = 1.f / a(i, i);

		// Eliminate the ith element of the jth row.
		for (int j = i + 1; j < nRows; j++)
			for (int k = nRows; k >= i; k--)
				a(j, k) -= a(i, k) * a(j, i) * invMax;
   }

   // Do the back substitution.
   for (int j = nRows - 1; j >= 0; j--) 
   {
		FLOAT tmp = 0.f;

		for (int k = nRows - 1; k > j; k--)
			tmp += a(j, k) * x[k];

		x[j] = (a(j, nRows) - tmp) / a(j, j);
   }

   return true;
}

// Solve an equation in the form Ax = b.
// Input (N+1)xN matrix [Ab].
// Destroys matrix, returns result in x.
// Returns false if the matrix was singular.
// to do: version that doesn't branch on active flag in inner loops
bool GESolve2(FLOAT* A, FLOAT* x, int nRows, int* active)
{
	FLOAT epsilon = 0.00001f;

	int i, j, k;
	FLOAT tmp;

	int nColumns = nRows + 1;

	MatrixHelper a(A, nRows, nColumns);

	for (i = 0; i < nRows; i++) 
	{
		if (active[i])
		{
			// Find the row with the largest first value.
			int maxrow = i;

			for (j = i + 1; j < nRows; j++) 
			{
				if (active[j])
				{
					if (Abs(a(j, i)) > Abs(a(maxrow, i)))
						maxrow = j;
				}
			}

			// Swap the maxrow and ith row.
			for (int j = 0; j < nColumns; j++)
			{
				if (active[j])
				{
					tmp = a(i, j);
					a(i, j) = a(maxrow, j);
					a(maxrow, j) = tmp;
				}
			}
		
			// Singular matrix?
			if (Abs(a(i, i)) < epsilon)
			{
				//DBPrint("GESolve::Singular Matrix, %i", i);
				return false;
			}

			FLOAT invMax = 1.f / a(i, i);

			// Eliminate the ith element of the jth row.
			for (int j = i + 1; j < nRows; j++)
			{
				if (active[j])
				{
					for (k = nRows; k >= i; k--)
					{
						if (active[k])
						{
							a(j, k) -= a(i, k) * a(j, i) * invMax;
						}
					}
				}
			}
		}
   }

   // Do the back substitution.
   for (j = nRows - 1; j >= 0; j--) 
   {
	   if (active[j])
	   {
			tmp = 0;
			for (k = j + 1; k < nRows; k++)
			{
				if (active[k])
					tmp += a(j, k) * x[k];
			}

			x[j] = (a(j, nRows) - tmp) / a(j, j);
	   }
   }

   return true;
}

}
