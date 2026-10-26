#include "stdafx.h"
#include "gravity/cloths/HighFieldFluid.h"
#include "gravity/cloths/HighFieldFluidCollision.h"
#include "gravity/narrowphase/shape.h"
#include "gravity/sdk/Body.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/sharedphysics/RigidBody.inl"
#include "gravity/narrowphase/CollisionResult.h"
#include "system/Debug/I_DebugRender.h"

namespace Collision
{


	//void C_Fluid::Verlet(const Maths::HF32 PCREF time)
	//{
	//	using namespace Maths;
	//	const HF32 constants=m_SquareSpeed/m_SquareWidth;
	//	const HF32 four=Constants<HF32>::Four();
	//	const HF32 recipFour(0.25f);
	//	const HF32 _time=time*time;
	//	const HF32 maxOffset=m_MaxSlop*time;
	//	const HF32 nmaxOffset(-maxOffset);
	//	const HF32 damping(1.f);
	//	const HF32 zero=Constants<HF32>::Zero();


	//	for(U32 i=1; i<=m_Dimension; ++i)
	//	{
	//		for(U32 j=1; j<=m_Dimension; ++j)
	//		{
	//			C_GridCell& cell=m_Grid[i][j];

	//			HF32& curHeight = cell.m_CurHeight;
	//			HF32& preHeight = cell.m_PreHeight;

	//			const HF32 p1= cell.GetNeighbourAverageHeight();
	//			const HF32 p2=curHeight*four;
	//			const HF32 dif = p1 - p2;
	//			const HF32 f=dif*constants+ cell.m_ExternalForce.Y(); //a=p1/h, f=ma
	//			preHeight = curHeight + (curHeight - preHeight)*damping + f*_time;

	//			//Clamp the wave height so the difference between this column and the neighbours are inside certain range
	//			const HF32 offset =dif*recipFour;
	//			const HF32 v1=offset - maxOffset;
	//			const HF32 v2=offset + maxOffset;

	//			const Maths::Scalar_Bool con1(offset.GT(maxOffset));
	//			const Maths::Scalar_Bool con2(offset.LT(nmaxOffset));

	//			preHeight=Select(con1, preHeight + v1, Select(con2, preHeight + v2, preHeight));

	//			//Clamp the wave height so that its height within a range

	//			const HF32 gap=preHeight - m_Height;

	//			const Maths::Scalar_Bool con3(gap.GT(maxOffset));
	//			const Maths::Scalar_Bool con4(gap.LT(nmaxOffset));
	//			preHeight=Select(con3, m_Height + maxOffset, Select(con4, m_Height + nmaxOffset, preHeight));


	//		}
	//	}

	//	//update the position and calculate the bounding box for the fluid
	//	HF32 height = zero;
	//	for(U32 i=1; i<=m_Dimension; ++i)
	//	{
	//		for(U32 j=1; j<=m_Dimension; ++j)
	//		{
	//			C_GridCell& cell=m_Grid[i][j];
	//			const HF32 temp= cell.m_CurHeight;
	//			cell.m_CurHeight =  cell.m_PreHeight;
	//			cell.m_PreHeight=temp;
	//			
	//			if(height.LT(cell.m_CurHeight))
	//				height =cell.m_CurHeight;

	//		}
	//	}
	//	m_Box.m_max.Y(m_Box.m_min.Y() + height);


	//	U32 s = m_Dimension + 1;
	//	//update the dummy node

	//	for(U32 k=1; k<=m_Dimension; ++k)
	//	{
	//		m_Grid[0][k].m_CurHeight= m_Grid[1][k].m_CurHeight;
	//		m_Grid[k][0].m_CurHeight = m_Grid[k][1].m_CurHeight;
	//		m_Grid[s][k].m_CurHeight= m_Grid[m_Dimension][k].m_CurHeight;
	//		m_Grid[k][s].m_CurHeight= m_Grid[k][m_Dimension].m_CurHeight;
	//	}
	//}


	void C_Fluid::SatisfyConstraint(const Maths::HF32 PCREF time)
	{
		using namespace Maths;
		const HF32 recipFour(0.25f);
		const HF32 zero = Constants<HF32>::Zero();
		const HF32 maxOffset=m_MaxSlop*time;
		const HF32 nmaxOffset(-maxOffset);

		for(U32 k=0; k<2 ; ++k)
		{
			for(U32 i=1; i<=m_Dimension; ++i)
			{
				for(U32 j=1; j<=m_Dimension; ++j)
				{
					C_GridCell& cell=m_Grid[i][j];
					HF32& curHeight=cell.m_CurHeight;

					

					//Clamp the wave height so the difference between this column and the neighbours are inside certain range
					const HF32 offset =cell.GetNeighbourAverageHeight()*recipFour - curHeight;
					const HF32 v1=offset - maxOffset;
					const HF32 v2=offset + maxOffset;

					const Maths::Scalar_Bool con1(offset.GT(maxOffset));
					const Maths::Scalar_Bool con2(offset.LT(nmaxOffset));

					curHeight=Select(con1, curHeight + v1, Select(con2, curHeight + v2, curHeight));

					//Clamp the wave height so that its height within a range
					const HF32 gap=curHeight - m_Height;
					const Scalar_Bool con3(gap.GT(maxOffset));
					const Scalar_Bool con4(gap.LT(nmaxOffset));
					curHeight=Select(con3, m_Height + maxOffset, Select(con4, m_Height + nmaxOffset, curHeight));

			/*		const Scalar_Bool con3((curHeight - m_Height).LT(zero));
					const Scalar_Bool con4((curHeight - m_MaxHeight).GT(zero));

					curHeight=Select(con3, m_Height, Select(con4, m_MaxHeight, curHeight));*/

					
				}
			}
		}

		U32 s = m_Dimension + 1;
		//update the dummy node

		for(U32 k=1; k<=m_Dimension; ++k)
		{
			m_Grid[0][k].m_CurHeight= m_Grid[1][k].m_CurHeight;
			m_Grid[k][0].m_CurHeight = m_Grid[k][1].m_CurHeight;
			m_Grid[s][k].m_CurHeight= m_Grid[m_Dimension][k].m_CurHeight;
			m_Grid[k][s].m_CurHeight= m_Grid[k][m_Dimension].m_CurHeight;
		}

	}

	void C_Fluid::Verlet(const Maths::HF32 PCREF time)
	{
		using namespace Maths;
		const HF32 constants=m_SquareSpeed/m_SquareWidth;
		const HF32 four=Constants<HF32>::Four();
		const HF32 _time=time*time;
		const HF32 damping(0.93f);
		const HF32 zero=Constants<HF32>::Zero();
		HF32 height = zero;


		for(U32 i=1; i<=m_Dimension; ++i)
		{
			for(U32 j=1; j<=m_Dimension; ++j)
			{
				C_GridCell& cell=m_Grid[i][j];

				HF32& curHeight = cell.m_CurHeight;
				const HF32 temp = curHeight;
				HF32& preHeight = cell.m_PreHeight;

				const HF32 dif = cell.GetNeighbourAverageHeight() - curHeight*four;
				const HF32 f=dif*constants + cell.m_ExternalForce.Y(); //a=p1/h, f=ma
				curHeight = curHeight + ((curHeight - preHeight) + f*_time)*damping;
				preHeight = temp;

				if(height.LT(curHeight))
					height = curHeight;

			}
		}


		m_Box.m_max.Y(m_Box.m_min.Y() + height);


		U32 s = m_Dimension + 1;
		//update the dummy node

		for(U32 k=1; k<=m_Dimension; ++k)
		{
			m_Grid[0][k].m_CurHeight= m_Grid[1][k].m_CurHeight;
			m_Grid[k][0].m_CurHeight = m_Grid[k][1].m_CurHeight;
			m_Grid[s][k].m_CurHeight= m_Grid[m_Dimension][k].m_CurHeight;
			m_Grid[k][s].m_CurHeight= m_Grid[k][m_Dimension].m_CurHeight;
		}

		SatisfyConstraint(time);
	}

	void C_Fluid::Initilise(const Maths::HVec3 PCREF position, const Maths::HF32 PCREF height)
	{
		using namespace Maths;

		m_Height = height;

		const HVec3 zeroV=Constants<HVec3>::Zero();
		const HF32 dif(1.f);
		//const HVec3 dif(0.f, 0.2f, 0.f);
		const HF32 x0(position.X());
		const HF32 y0(position.Y());
		const HF32 z0(position.Z());
		m_Position = position;
		const HF32 half=Constants<HF32>::Half();
		const HF32 one=Constants<HF32>::One();
		const HF32 zero=Constants<HF32>::Zero();
		const HF32 ext=m_Width*half;
		Maths::HVec3 initialHeight(Constants<HVec3>::Zero());

		U32 s = m_Dimension + 1;
		U32 a, b;

		for(U32 i=0; i< s; ++i)
		{
			const HF32 temp = z0 + ext*(i*2); 
			for(U32 j=0; j< s; ++j)
			{
				a=i+1;
				b=j+1;
				C_GridCell& cell=m_Grid[i+1][j+1];

				cell.m_Left = &m_Grid[a-1][b];
				cell.m_Right= &m_Grid[a+1][b];
				cell.m_Top  = &m_Grid[a][b-1];
				cell.m_Bottom = &m_Grid[a][b+1];

				const HF32 x=x0 + ext*(j*2);
				cell.m_WorldPosition = 	HVec3(x, y0, temp);
				const HF32 div=dif*j;

				//cell.m_OriHeight = height;
				
				if(i%2 == 0)
					cell.m_CurHeight =cell.m_PreHeight = height;
				else
					cell.m_CurHeight = cell.m_PreHeight = height*1.05f;

				//cell.m_MaxSlop = m_MaxSlop + dif;
				
				cell.m_Velocity  = cell.m_ExternalForce = zeroV;
		//		cell.m_CurOffSetHeight=zero;
			}
		}

		m_Box.m_min=m_Grid[1][1].m_WorldPosition;
		m_Box.m_min.X(m_Box.m_min.X() - m_Width);
		m_Box.m_min.Z(m_Box.m_min.Z() - m_Width);
		m_Box.m_max=m_Grid[m_Dimension+1][m_Dimension+1].m_WorldPosition;
		m_Box.m_max.Y(m_Grid[m_Dimension][m_Dimension].m_WorldPosition.Y() + m_Height);
		
		//update the dummy node

		for(U32 k=1; k<=m_Dimension; ++k)
		{
			m_Grid[0][k].m_CurHeight=m_Grid[1][k].m_CurHeight;
			m_Grid[k][0].m_CurHeight =m_Grid[k][1].m_CurHeight;
			m_Grid[s][k].m_CurHeight =m_Grid[m_Dimension][k].m_CurHeight;
			m_Grid[k][s].m_CurHeight =m_Grid[k][m_Dimension].m_CurHeight;
		}

	}



	void C_Fluid::EulerIntergation(const Maths::HF32 PCREF time)
	{

		using namespace Maths;

		const HF32 four=Constants<HF32>::Four();
		const HF32 recipFour(0.25f);
		const HF32 zero=Constants<HF32>::Zero();
		const HF32 ratio(0.999f);
		const HF32 maxOffset=m_MaxSlop*time;
		const HF32 nmaxOffset(-maxOffset);
		const HF32 damping(0.99f);


		const HF32 constants=m_SquareSpeed/m_SquareWidth;
		

		for(U32 i=1; i<= m_Dimension; ++i)
		{
			for(U32 j=1; j<=m_Dimension; ++j)
			{
				C_GridCell& cell=m_Grid[i][j];
				HF32& preHeight = cell.m_PreHeight;
				
				const HF32 p1=cell.GetNeighbourAverageHeight();
				const HF32 p2=cell.m_CurHeight*four;
				const HF32 dif = p1 - p2;
				const HF32 a=dif*constants; //a=p1/h, f=ma
				const HF32 v = cell.m_Velocity.Y() + a*time;
				preHeight = cell.m_CurHeight + v*time;
				
				//cell.m_Velocity.Y(v);

				const HF32 offset =dif*recipFour;
				const HF32 v1=offset - maxOffset;
				const HF32 v2=offset + maxOffset;

				const Maths::Scalar_Bool con1(offset.GT(maxOffset));
				const Maths::Scalar_Bool con2(offset.LT(nmaxOffset));

				preHeight=Select(con1, preHeight + v1, Select(con2, preHeight + v2, preHeight));

				//Clamp the wave height so that its height within a range

				const HF32 gap=preHeight - m_Height;

				const Maths::Scalar_Bool con3(gap.GT(maxOffset));
				const Maths::Scalar_Bool con4(gap.LT(nmaxOffset));
				preHeight=Select(con3, m_Height + maxOffset, Select(con4, m_Height + nmaxOffset, preHeight));

			}
		}

		//update the position and calculate the bounding box for the fluid
		HF32 height = zero;
		for(U32 i=1; i<=m_Dimension; ++i)
		{
			for(U32 j=1; j<=m_Dimension; ++j)
			{
				C_GridCell& cell=m_Grid[i][j];
				HF32& curHeight = cell.m_CurHeight;
				HF32& preHeight = cell.m_PreHeight;
				const HF32 temp= curHeight;
				curHeight = preHeight;
				preHeight=temp;

				cell.m_Velocity.Y((curHeight - preHeight)/time);
				
				if(height.LT(curHeight))
					height = curHeight;

			}
		}
		m_Box.m_max.Y(m_Box.m_min.Y() + height);

		const U32 s=m_Dimension + 1;

		//update the position
		for(U32 k=1; k<=m_Dimension; ++k)
		{
			m_Grid[0][k].m_CurHeight=m_Grid[1][k].m_CurHeight;
			m_Grid[k][0].m_CurHeight =m_Grid[k][1].m_CurHeight;
			m_Grid[s][k].m_CurHeight =m_Grid[m_Dimension][k].m_CurHeight;
			m_Grid[k][s].m_CurHeight =m_Grid[k][m_Dimension].m_CurHeight;
		}

	}




	//void C_Fluid::ApplyForce(Collision::C_CollisionObject* pObject)
	//{
	//	using namespace Maths;
	//	SharedPhysics::C_RigidBody* pRigidBody= pObject->m_pNodePtr->m_pRigidBody;
	//	
	//	Collision::C_CRAxisBox& box=pObject->m_TransformedBox;

	//	const HVec3 _min = box.centre - box.extents;
	//	const HVec3 _max = box.centre + box.extents;
	//	HMat43 mat;
	//	mat.SetIdentity();
	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, RGBA(255, 0, 0, 255));

	//	const HVec3 dimension = _max - _min;
	//	const HF32 volume = dimension.X() * dimension.Y() * dimension.Z();

	//	HF32 fluidHeight=m_Position.Y() + m_Height;
	//	HF32 ratio;
	//	HVec3 centroid;
	//	

	//	if(fluidHeight.GT(_max.Y()))
	//	{
	//		fluidHeight=_max.Y();
	//		ratio = Constants<HF32>::One();
	//		centroid = box.centre;
	//	}
	//	else
	//	{
	//		const HF32 submergedHeight =fluidHeight - _min.Y(); 
	//		ratio = submergedHeight / dimension.Y();
	//		HVec3 subMax = _max;
	//		subMax.Y(subMax.Y() - submergedHeight);
	//		const HF32 half = Constants<HF32>::Half();
	//		centroid=(subMax + _min)*half;
	//	}

	//	
	//	const HF32 submergedVolume = volume*ratio;
	//	
	//	const HVec3 buoyancyForce= -(submergedVolume * m_Density * m_Gravity);

	//	//Calculate the centroid of the submerged volume
	//	

	//	const HF32 partialMass = pRigidBody->m_Mass * ratio;
	//	const HVec3 rc = centroid - box.centre;
	//	const HVec3 vc = pRigidBody->m_LinearVelocity + rc^pRigidBody->m_AngularVelocity ;
	//	const HVec3 dragForce2 = (m_Velocity - vc)*(partialMass* m_LinearDrag);
	//	const HVec3 dragForce = (m_Velocity - pRigidBody->m_LinearVelocity)*(partialMass* m_LinearDrag);

	//	const HVec3 totalForce = buoyancyForce + dragForce;

	//	pRigidBody->AddForce(totalForce);
	//	pRigidBody->AddTorque(totalForce ^ rc);

	//	
	//	const HF32 length2 = dimension.Y() * dimension.Y();
	//	const HVec3 dragTorque = (-partialMass*m_AngularDrag*length2)*pRigidBody->m_AngularVelocity;
	//	pRigidBody->AddTorque(dragTorque);
	//
	//	/*const HF32 energy = pRigidBody->m_KineticEnergy | pRigidBody->m_KineticEnergy;
	//	if(energy.LT(0.05f))
	//	{
	//		pRigidBody->m_LinearMomentum = Constants<HVec3>::Zero();
	//	}*/
	//
	//	if(pObject->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	{
	//		//if(!sleepCondition)
	//		{
	//			pObject->m_pNodePtr->m_Deactivator.Activate();
	//		}
	//	}
	//	
	//}

	void C_Fluid::ApplyForce(Collision::C_CollisionObject* pObject, const Maths::HF32 PCREF averageHeight)
	{
		using namespace Maths;
		SharedPhysics::C_RigidBody* pRigidBody= pObject->m_pNodePtr->m_pRigidBody;
		C_Shape* pShape=pObject->m_pNodePtr->m_pBody->GetShape();

		/*Collision::C_CRAxisBox box;
		pShape->TransformBox(box);
		const HVec3 _min = box.centre - box.extents;
		const HVec3 _max = box.centre + box.extents;*/


		/*HMat43 mat;
		mat.SetIdentity();*/
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, RGBA(255, 255, 0, 0));

		//HVec3 _mid=_min;
		//_mid.Y(m_Position.Y() + averageHeight);

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_mid, _max, mat, RGBA(0, 255, 0, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(_mid, 0.25f, RGBA(0, 255, 255, 255));



		//HVec3 p(m_Position.X(), m_Position.Y() + m_Height, m_Position.Z());

		HVec3 p(m_Position.X(), m_Position.Y() + averageHeight, m_Position.Z());
		const C_Normal normal(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		
		HVec3 centroid(UNDEFINED);
		HF32 ratio(UNDEFINED);
		HF32 subVolume = pShape->GetSubVolume(p, normal, centroid, ratio);

		const Maths::HF32 mass(pRigidBody->Mass());
		const Maths::HF32 buoyancyMagnitude(Maths::Min(m_Density*subVolume, m_MaxBuoyancy * mass));
		const Maths::HVec3 buoyancyForce = buoyancyMagnitude*Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();

		const Maths::HF32 partialMass = mass * ratio;
		const Maths::HVec3 vc(pRigidBody->PointVelocity(centroid));
		const Maths::HF32 dragStrength(m_LinearDrag);
		const Maths::HVec3 vel(m_Velocity - vc);

		const Maths::HVec3 dragForce = (partialMass*dragStrength)*(vel);

		pRigidBody->ApplyForceWorldSpace((buoyancyForce * pRigidBody->BuoyancyCoefficient() + dragForce * pRigidBody->FluidLinearDragCoefficient()), centroid);
		
		const Maths::HF32 length2 = pObject->m_TransformedBox.extents | pObject->m_TransformedBox.extents;
		const Maths::HVec3 dragTorque = (-partialMass*m_AngularDrag*length2)*pRigidBody->m_AngularVelocity;
		pRigidBody->AddTorque(dragTorque * pRigidBody->FluidAngularDragCoefficient());

	//	HF32 referenceArea = pShape->GetReferenceArea();

	//	const HF32 half=Constants<HF32>::Half();
	//	const HF32 two=Constants<HF32>::Two();
	//
	//	//HVec3 buoyancyForce= -(subVolume * m_Density * m_Gravity * pRigidBody->m_BuoyancyCoefficient);
	//	HVec3 buoyancyForce= -(subVolume * m_Density * pRigidBody->m_BuoyancyCoefficient);
	//
	//	const HF32 partialMass = pRigidBody->m_Mass * ratio;
	//	const HVec3 rc = centroid - pRigidBody->m_Position;
	//	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(centroid, 0.25f, RGBA(0, 255, 0, 255));

	//	const HVec3 angularVelocity = rc ^ pRigidBody->m_AngularVelocity;

	//	const HVec3 vc = pRigidBody->m_LinearVelocity + angularVelocity;
	//	//const HVec3 dragForce2 = (m_Velocity - vc)*(partialMass* m_LinearDrag);
	//	//const HVec3 dragForce = (m_Velocity - pRigidBody->m_LinearVelocity)*(partialMass* m_LinearDrag);

	//	const HVec3 dragForce = half*m_Density*(m_Velocity - pRigidBody->m_LinearVelocity)*referenceArea*m_LinearDrag*partialMass;

	//	//const HVec3 dragForce = half*m_Density*(m_Velocity - vc)*referenceArea*m_LinearDrag*partialMass;
	//	//pRigidBody->m_AngularMomentum = Constants<HVec3>::Zero();



	//	HVec3 totalForce = buoyancyForce + dragForce;

	//	pRigidBody->AddForce(totalForce);

	//	pRigidBody->AddTorque(totalForce^rc);

	//	
	//	const HF32 x= pShape->m_LocalBoundingBox.extents.X()*two;
	//	const HF32 z= pShape->m_LocalBoundingBox.extents.Z()*two;
	//	//const HF32 length2 =(x*x+z*z);


	//	const HVec3 a=_max - _min;
	//	const HF32 length2 = a.Length()*half;
	////	const HVec3 dragTorque = (-partialMass*m_AngularDrag*length2)*pRigidBody->m_AngularVelocity*m_Density*half;

	//	const HVec3 dragTorque = (-partialMass*m_AngularDrag*length2)*pRigidBody->m_AngularVelocity;
	//	pRigidBody->AddTorque(dragTorque);
	
		if(pObject->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			if(pRigidBody->m_Force.Length().GE(Constants<HF32>::Zero()))
			{
				pObject->m_pNodePtr->m_Deactivator.Activate();
			}
		}
		
	}

	


	
};
