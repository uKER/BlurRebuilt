#include "Stdafx.h"
#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/sharedphysics/Select.h"
#include "gravity/narrowphase/PersistentContactManifold.h"
#include "gravity/narrowphase/PrimitiveTest.h"

namespace Collision
{
	
	void C_PersistentContactManifold::CalculateQuadrilateralArea(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d, Maths::HF32& sqArea)
	{
		using namespace Maths;

		//a and b is the point to keep, therefore, if ab is the diagonal vector, which mean a is inside triangle bcd or b is inside triange acd

		/*if(PointInsideTriangle(a, b, c, d))
		{
			sqArea = Constants<HF32>::Zero();
		}
		else
		{*/

			const HVec3 ab(b - a);
			const HVec3 ac(c - a);
			const HVec3 ad(d - a);
			const C_Normal nab(ab);
			const C_Normal nac(ac);
			const C_Normal nad(ad);

			const HF32 cosTheta1(nab|nac);
			const HF32 cosTheta2(nab|nad);
			const HF32 cosTheta3(nac|nad);


			const HVec3 abc(ab^ac);
			const HVec3 abd(ab^ad);
			const HVec3 acd(ac^ad);

			const HF32 triAbc(abc|abc);
			const HF32 triAbd(abd|abd);
			const HF32 triAcd(acd|acd);

			//use the bigest angle to determind which vector is the diagonal vector
			Scalar_Bool con0(cosTheta1.LT(cosTheta2) & cosTheta1.LT(cosTheta3));
			Scalar_Bool con1(cosTheta2.GT(cosTheta3));
			sqArea = Select(con0, triAbd+triAcd, Select(con1, triAbc+triAcd, triAbc+triAbd));

			/*Scalar_Bool con2(triAbc.GT(triAbd) & triAbc.GT(triAcd));
			Scalar_Bool con3(triAbd.GT(triAcd));
			sqArea = Select(con2, triAbc, Select(con3, triAbd, triAcd));*/


			//if(cosTheta1.LT(cosTheta2) & cosTheta1.LT(cosTheta3))
			//{
			//	//The angle between ab and ac is the bigest angle, which make ad the diagonal vector
			//	//Calculte the area of triangle abd and triangle acd
			//	const HVec3 abd(ab^ad);
			//	const HVec3 acd(ac^ad);
			//	sqArea = (abd|abd) + (acd|acd);
			//}
			//else if(cosTheta2.GT(cosTheta3))
			//{
			//	//The angle between ab and ad is the bigest angle, which make ac the diagonal vector
			//	//Calculte the area of triangle abc and triangle adc
			//	const HVec3 abc(ab^ac);
			//	const HVec3 adc(ad^ac);
			//	sqArea = (abc|abc) + (adc|adc);
			//}
			//else
			//{
			//	//The angle between ac and ad is the bigest angle, which make ab the diagonal vector
			//	//Calculte the area of triangle acb and triangle adb
			//	const HVec3 acb(ac^ab);
			//	const HVec3 adb(ad^ab);
			//	sqArea =(acb|acb) + (adb|adb);

			//}

	//	}
	}

	//void C_PersistentContactManifold::ReduceManifoldContacts(C_ManifoldContact &contact)
	//{
	//	using namespace Maths;

	//	F32 maxPen(-Constants<F32>::Max());
	//	S32 index = -1;
	//	//Keep the deepest penetration point
	//	for(U32 i=0; i< m_NumContacts; ++i)
	//	{
	//		if(m_Manifold[i].m_PenetrationDepth > maxPen)
	//		{
	//			index= i;
	//			maxPen = m_Manifold[i].m_PenetrationDepth;
	//		}
	//	}

	//	//Keep  the configuration that creates the biggest area

	//	using namespace Maths;
	//	HF32 zero = Constants<HF32>::Zero();
	//	HF32 sqArea0(zero), sqArea1(zero), sqArea2(zero);

	//	U32 indices[3];
	//	U32 count=0;
	//	for(U32 i=0; i<m_NumContacts; ++i)
	//	{
	//		if(i != index)
	//		{
	//			indices[count++] = i;
	//		}
	//	}

	//	CalculateQuadrilateralArea(contact.m_LocalContactA, m_Manifold[index].m_LocalContactA, m_Manifold[indices[0]].m_LocalContactA, m_Manifold[indices[1]].m_LocalContactA, sqArea0);
	//	CalculateQuadrilateralArea(contact.m_LocalContactA, m_Manifold[index].m_LocalContactA, m_Manifold[indices[0]].m_LocalContactA, m_Manifold[indices[2]].m_LocalContactA, sqArea1);
	//	CalculateQuadrilateralArea(contact.m_LocalContactA, m_Manifold[index].m_LocalContactA, m_Manifold[indices[1]].m_LocalContactA, m_Manifold[indices[2]].m_LocalContactA, sqArea2);
	//	
	//	
	//	S32 replaceIndex = -1;
	//	bool con0(sqArea0.GT(sqArea1) & sqArea0.GT(sqArea2));
	//	bool con1(sqArea1.GT(sqArea2));

	//	replaceIndex = Select(con0, indices[2], Select(con1, indices[1], indices[0]));
	//	BIZ_ASSERT(index != replaceIndex);
	//	m_Manifold[replaceIndex] = contact;
	//	
	//}


	void C_PersistentContactManifold::ReduceManifoldContacts(C_ManifoldContact &contact)
	{
		using namespace Maths;
		
		F32 maxPen(-Constants<F32>::Max());
		S32 index = -1;
		//F32 averagePen = 0.f;
		
		//Keep the deepest penetration point
		for(U32 i=0; i< m_NumContacts; ++i)
		{
			if(m_Manifold[i].m_PenetrationDepth > maxPen)
			{
				index= i;
				maxPen = m_Manifold[i].m_PenetrationDepth;
			}
		}

		U32 indices[3];
		indices[0] = (index + 1)&3;
		indices[1] = (index + 2)&3;
		indices[2] = (index + 3)&3;

		//Keep  the configuration that creates the biggest area
		const HVec3 ab(m_Manifold[indices[0]].m_LocalContactA - contact.m_LocalContactA);
		const HVec3 ac(m_Manifold[indices[1]].m_LocalContactA - contact.m_LocalContactA);
		const HVec3 ad(m_Manifold[indices[2]].m_LocalContactA - contact.m_LocalContactA);

		const HVec3 abc(ab^ac);
		const HVec3 abd(ab^ad);
		const HVec3 acd(ac^ac);

		const HF32 areaABC(abc|abc);
		const HF32 areaABD(abd|abd);
		const HF32 areaACD(acd|acd);

		bool con0(areaABC.GT(areaABD) & areaABC.GT(areaACD));
		bool con1(areaABD.GT(areaACD));

		S32 replaceIndex = Select(con0, indices[2], Select(con1, indices[1], indices[0]));
		BIZ_ASSERT(index != replaceIndex && replaceIndex <MaxManifoldContacts);
		/*averagePen = averagePen - m_Manifold[replaceIndex].m_PenetrationDepth + contact.m_PenetrationDepth;
		averagePen = averagePen/4.f;*/

		m_Manifold[replaceIndex] = contact;

		/*for(U32 i=0; i<m_NumContacts; ++i)
		{
			m_Manifold[i].m_PenetrationDepth = averagePen;
		}*/



	/*	CalculateQuadrilateralArea(contact.m_LocalContactA, m_Manifold[index].m_LocalContactA, m_Manifold[indices[0]].m_LocalContactA, m_Manifold[indices[1]].m_LocalContactA, sqArea0);
		CalculateQuadrilateralArea(contact.m_LocalContactA, m_Manifold[index].m_LocalContactA, m_Manifold[indices[0]].m_LocalContactA, m_Manifold[indices[2]].m_LocalContactA, sqArea1);
		CalculateQuadrilateralArea(contact.m_LocalContactA, m_Manifold[index].m_LocalContactA, m_Manifold[indices[1]].m_LocalContactA, m_Manifold[indices[2]].m_LocalContactA, sqArea2);*/
		
		
		/*S32 replaceIndex = -1;
		bool con0(sqArea0.GT(sqArea1) & sqArea0.GT(sqArea2));
		bool con1(sqArea1.GT(sqArea2));

		replaceIndex = Select(con0, indices[2], Select(con1, indices[1], indices[0]));
		BIZ_ASSERT(index != replaceIndex);
		m_Manifold[replaceIndex] = contact;*/
		
	}

	void C_PersistentContactManifold::AddManifoldContact(C_ManifoldContact& contact)
	{
		using namespace Maths;

	//	contact.m_NewPoint = true;

		/*for(U32 a = 0; a < m_NumContacts; ++a)
		{
			m_Manifold[a].m_NewPoint = false;
		}*/

		//contact.m_AverageNormal = contact.m_Normal;
		contact.m_OriginalPenetration = contact.m_PenetrationDepth;
		//Loop through all the contacts and find out if we've got one which is identical to this point.
		for(U32 a = 0; a < m_NumContacts; ++a)
		{
			const HVec3 dif = contact.m_LocalContactA - m_Manifold[a].m_LocalContactA;
			if((dif|dif).LE(m_SqContactBreakingThreshold))
			{
				//if the point already in the manifold, replace this point with the new contact point(same point but might be has different penetration)		
				m_Manifold[a] = m_Manifold[0];
				goto AverageNormal;
			}
		}

		if(m_NumContacts< MaxManifoldContacts)
			m_Manifold[m_NumContacts++] = contact;
		else
			ReduceManifoldContacts(contact);

AverageNormal:
		
		/*HVec3 sumNormal(Constants<HVec3>::Zero());
		for(U32 i=0; i<m_NumContacts; ++i)
		{
			sumNormal += m_Manifold[i].m_Normal;
		}

		const C_Normal averageNormal = C_Normal(sumNormal);

		for(U32 i=0; i<m_NumContacts; ++i)
		{
			m_Manifold[i].m_AverageNormal = averageNormal;
		}*/
		return;
	}


	void C_PersistentContactManifold::RemoveManifoldContact(const U32 index)
	{
		//for(U32 j = index; j < m_NumContacts; ++j)
		//Avoid the loop, just remove the point and replace it with the last point!
		BIZ_ASSERT(index < m_NumContacts);
		BIZ_ASSERT(m_NumContacts <= MaxManifoldContacts);
		U32 replaceIndex = m_NumContacts - 1;
		{
			m_Manifold[index] = m_Manifold[replaceIndex];
		}
		m_NumContacts--;

	}

	/*
		This method is called after each intergration step
	*/

	void C_PersistentContactManifold::RefreshManifoldContacts(const Maths::HMat43 PCREF transA, const Maths::HMat43 PCREF transB)
	{
		using namespace Maths;

		BIZ_ASSERT(m_NumContacts <= MaxManifoldContacts);
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 hundredth(SharedPhysics::Constants::Hundredth());
		
		for(U32 i=0; i< m_NumContacts; ++i)
		{
			C_ManifoldContact& point = m_Manifold[i];
			const HF32 originalPenetration(Maths::Load<Maths::HF32>(&point.m_OriginalPenetration));
			const HVec3 worldContactA( point.m_LocalContactA * transA);
			const HVec3 worldContactB(point.m_LocalContactB * transB);

			const HVec3 v(worldContactB - worldContactA);

			const HF32 projDist(v|point.m_Normal);
			
			const HF32 newPen(originalPenetration - Maths::Max(projDist, zero));

			const HVec3 rv(v - projDist * point.m_Normal);
			const HF32 contactABDist(rv|rv);
			//const HF32 contactABDist(v|v);

			Maths::Store(Maths::Max(newPen - hundredth, zero), &point.m_PenetrationDepth);
			if(newPen.LT(zero) |contactABDist.GT(m_SqContactBreakingThreshold))// | cosWorldAngleA.LT(closeAngle) | cosWorldAngleB.LT(closeAngle))
			{
				RemoveManifoldContact(i);
				--i;
			}
			//else
			//{
			//	//point.m_LocalContactB = transB.Rotation() * (worldContactA - transB.Translation());
			//	//point.m_Normal = C_Normal(-v);
			//	//point.m_PenetrationDepth = v.Length();
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(worldContactB, 0.01f, Maths::RGBA(0, 255, 0));
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(worldContactA, 0.01f, Maths::RGBA(0, 255, 255));
			//	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(worldContactA, worldContactB, Maths::RGBA(0, 255, 0));
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(worldContactA, point.m_Normal, point.m_PenetrationDepth, 0.05f, Maths::RGBA(255, 255, 0));
			//	
			//} 
		}
	}


}
