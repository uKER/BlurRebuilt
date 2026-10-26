#ifndef HIGHFIELDFLUID_H
#define HIGHFIELDFLUID_H

#include "gravity/narrowphase/AxisBox.h"

namespace Collision
{
class C_CollisionObject;
class C_HFFCollisionManager;

class C_GridCell
{
public:
	C_GridCell() : m_PreHeight(Maths::Constants<Maths::HF32>::Zero()), m_CurHeight(Maths::Constants<Maths::HF32>::Zero()),
	m_WorldPosition(Maths::Constants<Maths::HF32>::Zero()), m_Velocity(Maths::Constants<Maths::HF32>::Zero()),
	m_ExternalForce(Maths::Constants<Maths::HF32>::Zero())
	{
	}

	~C_GridCell()
	{
	}

	Maths::HF32 GetNeighbourAverageHeight()
	{
		return m_Left->m_CurHeight + m_Right->m_CurHeight + m_Top->m_CurHeight + m_Bottom->m_CurHeight;
	}

	Maths::HF32 m_PreHeight;
	Maths::HF32 m_CurHeight;;
	//Maths::HF32 m_CurOffSetHeight;

	Maths::HVec3 m_WorldPosition;
	Maths::HVec3 m_Velocity;
	Maths::HVec3 m_ExternalForce;
	bool m_Hit;
	C_GridCell* m_Left;
	C_GridCell* m_Right;
	C_GridCell* m_Top;
	C_GridCell* m_Bottom;
};

class C_Fluid
{
public:
	C_Fluid(C_HFFCollisionManager* manager=NULL):m_CollisionManager(manager), m_Width(Maths::Constants<Maths::HF32>::Zero()),
	m_MaxHeight(Maths::Constants<Maths::HF32>::Zero()), m_Height(Maths::Constants<Maths::HF32>::Zero()), 
	m_SquareSpeed(Maths::Constants<Maths::HF32>::Zero()), m_SquareWidth(Maths::Constants<Maths::HF32>::Zero()),
	m_Density(Maths::Constants<Maths::HF32>::Zero()), m_MaxSlop(Maths::Constants<Maths::HF32>::Zero()), 
	m_MaxBuoyancy(Maths::Constants<Maths::HF32>::Zero()), m_Gravity(Maths::Constants<Maths::HVec3>::Zero()),
	m_Position(Maths::Constants<Maths::HVec3>::Zero()), m_Velocity(Maths::Constants<Maths::HVec3>::Zero()), 
	m_LinearDrag(Maths::Constants<Maths::HF32>::Zero()), m_AngularDrag(Maths::Constants<Maths::HF32>::Zero())
	{
		m_Gravity = Maths::HVec3(0.f, -9.8f, 0.f);
	}

	C_Fluid(const U32 dimension): m_Dimension(dimension), m_CollisionManager(NULL), m_Width(Maths::Constants<Maths::HF32>::Zero()),
	m_MaxHeight(Maths::Constants<Maths::HF32>::Zero()), m_Height(Maths::Constants<Maths::HF32>::Zero()), 
	m_SquareSpeed(Maths::Constants<Maths::HF32>::Zero()), m_SquareWidth(Maths::Constants<Maths::HF32>::Zero()),
	m_Density(Maths::Constants<Maths::HF32>::Zero()), m_MaxSlop(Maths::Constants<Maths::HF32>::Zero()), 
	m_MaxBuoyancy(Maths::Constants<Maths::HF32>::Zero()), m_Gravity(Maths::Constants<Maths::HVec3>::Zero()),
	m_Position(Maths::Constants<Maths::HVec3>::Zero()), m_Velocity(Maths::Constants<Maths::HVec3>::Zero()), 
	m_LinearDrag(Maths::Constants<Maths::HF32>::Zero()), m_AngularDrag(Maths::Constants<Maths::HF32>::Zero())
	{
		m_Gravity = Maths::HVec3(0.f, -9.8f, 0.f);
	}

	~C_Fluid()
	{
		const U32 size = m_Dimension +2;
		for(register U32 i=0; i< size; ++i)
			delete [] m_Grid[i];
		delete [] m_Grid;
	}

	void SetCollisionManager(C_HFFCollisionManager* manager)
	{
		m_CollisionManager = manager;
	}

	void SetGridDimension(const U32 dimension)
	{
		m_Dimension = dimension;
		const U32 size=dimension + 2;
		m_Grid = new C_GridCell*[size];
		for(U32 i=0; i< size; ++i)
			m_Grid[i] = new C_GridCell[size];
	}

	void SetMaxSlop(const Maths::HF32 PCREF maxSlop)
	{
		m_MaxSlop = maxSlop;
	}

	void SetWaveSpeed(const Maths::HF32 PCREF speed)
	{
		m_SquareSpeed=speed*speed;
	}

	void SetWaveWidth(const Maths::HF32 PCREF width)
	{
		m_Width = width;
		m_SquareWidth = width * width;
	}

	void SetDensity(const Maths::HF32 PCREF density)
	{
		m_Density = density;
	}

	void Initilise(const Maths::HVec3 PCREF position, const Maths::HF32 PCREF height);

	/*void AccumulateForces()
	{

		for(U32 i=1; i<=m_Dimension; ++i)
		{
			for(U32 j=1; j<=m_Dimension; ++j)
			{
				C_GridCell& cell=m_Grid[i][j];
				cell.m_ExternalForce = -cell.m_Dif * m_SquareWidth*m_Density*m_Gravity;
			}
		}
	}*/

	void ClearFlag()
	{
		for(U32 i=1; i<=m_Dimension; ++i)
		{
			for(U32 j=1; j<=m_Dimension; ++j)
			{
				C_GridCell& cell=m_Grid[i][j];
				cell.m_Hit = false;
			}
		}

	}


	void ApplyForce(Collision::C_CollisionObject* pObject, const Maths::HF32 PCREF averageHeight);
	

	void EulerIntergation(const Maths::HF32 PCREF time);

	void Verlet(const Maths::HF32 PCREF time);

	void SatisfyConstraint(const Maths::HF32 PCREF time);

	void TimeStep(const Maths::HF32 PCREF time)
	{
	//	AccumulateForces();
		Verlet(time);
	}

	
	U32 m_Dimension;
	C_GridCell** m_Grid;
	Maths::HF32 m_Width;
	Maths::HF32 m_MaxHeight;
	Maths::HF32 m_Height;
	Maths::HF32 m_SquareSpeed;
	Maths::HF32 m_SquareWidth;
	Maths::HF32 m_Density;
	Maths::HF32 m_MaxSlop;
	Maths::HF32 m_MaxBuoyancy;
	Maths::HVec3 m_Gravity;
	Maths::HVec3 m_Position;
	Maths::HVec3 m_Velocity;
	Maths::HF32 m_LinearDrag;
	Maths::HF32 m_AngularDrag;
	Collision::C_AxisBoxMinMax m_Box;
	C_HFFCollisionManager* m_CollisionManager;


};

}

#endif
