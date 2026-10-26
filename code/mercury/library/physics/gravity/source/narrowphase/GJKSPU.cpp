#include "stdafx.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/SPUManager.h"


namespace Collision
{
#ifdef PLATFORM_SPU

bool _GJKCPair(C_Convex* a, C_Convex* b)
{
	using namespace Maths;

	SPUManager manager;
	U32 size=1;
	HVec3 origin(Constants<HVec3>::Zero());				//(0.0f, 0.0f, 0.0f)

	const Maths::HVec3 assumedNormDir(a->Centre() - b->Centre());
	const Maths::HVec3 initialSearchDir(assumedNormDir);
	const Maths::HVec3 initialSupportA(manager.Support(a, initialSearchDir));
	const Maths::HVec3 initialSupportB(manager.Support(b, initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	

	//HVec3 upper(Maths::Constants<HVec3>::Max());
	HVec3 closestPt(Q[0]);
	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB);
	//const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	const HF32 epsilon(Maths::Min(a->Epsilon(), b->Epsilon()));
	HVec3 closA(initialSupportA), closB(initialSupportB);
	HF32 sDist(closestPt|closestPt);

	S32 maxIterations = 10;

	bool result = true;
	//while(!(closestPt.All_EQ(origin)) && (maxIterations --))
	while( sDist.GT(epsilon) && (maxIterations --))
	{
		
		supportA=manager.Support(a, -closestPt);//aSup(a, -closestPt);
		supportB=manager.Support(b, closestPt);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);
		
		const HF32 sup_val(sDist-(support|closestPt));
		
		//const HVec3 supDelta(oldSupport - support);
		//const HF32 dif(supDelta | supDelta);
		//const Scalar_Bool b(sup_val.LE(epsilon) | dif.LE(epsilon));
		if(sup_val.LE(epsilon))
		{
			return false;
		}
	
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull
		GJKCPairDoSimplex(Q, A, B, size, closA, closB);

		closestPt = (closA - closB);
		sDist = closestPt|closestPt;
	}

	if(maxIterations == -1)
		return false;
	
	return true; //intersect, closest point is origin
	
}


bool _GJKCPair(C_Convex* a, C_Convex* b, Maths::HF32& dis, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	SPUManager manager;
	U32 size=1;
	HVec3 origin(Constants<HVec3>::Zero());				//(0.0f, 0.0f, 0.0f)

	const Maths::HVec3 initialSearchDir(a->Centre() - b->Centre());
	const Maths::HVec3 initialSupportA(manager.Support(a, initialSearchDir));
	const Maths::HVec3 initialSupportB(manager.Support(b, initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	

	//HVec3 upper(Maths::Constants<HVec3>::Max());
	HVec3 closestPt(Q[0]);
	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB);
	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	HVec3 closA(initialSupportA), closB(initialSupportB);
	HF32 sDist(closestPt|closestPt);

	S32 maxIterations = 10;

	bool result = true;
	//while(!(closestPt.All_EQ(origin)) && (maxIterations --))
	while( sDist.GT(epsilon) && (maxIterations --))
	{
		
		supportA=manager.Support(a, -closestPt);//aSup(a, -closestPt);
		supportB=manager.Support(b, closestPt);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);
		
		const HF32 sup_val((closestPt|closestPt)-(support|closestPt));
		
		//const HVec3 supDelta(oldSupport - support);
		//const HF32 dif(supDelta | supDelta);
		//const Scalar_Bool b(sup_val.LE(epsilon) | dif.LE(epsilon));
		if(sup_val.LE(epsilon))
		{
			result = false;
			break;
		}
	
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull
		GJKCPairDoSimplex(Q, A, B, size, closA, closB);

		closestPt = (closA - closB);
		sDist = closestPt|closestPt;
	}

	if(maxIterations == -1)
		result = false;
	dis = closestPt.Length();
	closestA = closA;
	closestB = closB;
	/*a.wStart=supportA;
	b.wStart=supportB;*/
	
	return result; //intersect, closest point is origin
	
}

bool _GJKCPair(C_Convex* a, C_Convex* b, Maths::HVec3& contact, Maths::HVec3& normal, Maths::HF32& penetration, EPA& epa)
{
	using namespace Maths;
	SPUManager manager;
	//const HF32 zero(Maths::Constants<HF32>::Epsilon());//*Maths::Constants<Maths::HF32>::Epsilon2());
	U32 size=1;
	//HVec3 origin(Maths::Constants<HVec3>::Zero());				//(0.0f, 0.0f, 0.0f)
	const Maths::HF32 _zero(Maths::Constants<Maths::HF32>::Zero());

	const Maths::HVec3 assumedNormDir(a->Centre() - b->Centre());
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	
	const Maths::HVec3 initialSearchDir(assumedNormDir);
	const Maths::HVec3 initialSupportA(manager.Support(a, initialSearchDir));
	const Maths::HVec3 initialSupportB(manager.Support(b, initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	

	//HF32 upper(Maths::Constants<HF32>::Epsilon());
	HVec3 closestPt(Q[0]);
	
	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	const HF32 zero(epsilon*epsilon);

	HF32 sDist(closestPt|closestPt);
	
	GJKResultType result = E_DEEP_INTERSECTION;

	Maths::HVec3 closestA(initialSupportA), closestB(initialSupportB);

	S32 maxIterations = 10;
	
	while(U32(sDist > zero) && (maxIterations--))//(!(closestPt.All_EQ(origin)))
	{	
		
		const Maths::HVec3 supportA=manager.Support(a, -closestPt);//aSup(a, -closestPt);
		const Maths::HVec3 supportB=manager.Support(b, closestPt);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);

		const HF32 sup_val(sDist-(support|closestPt));
		
		if(sup_val.LE(epsilon))// | dif.LE(zero)) //the objects don't intersect with each other, but they might intersect in the margin
			return false;	

		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull, the size of simplex will be reseted in this method
		//Maths::Scalar_Bool bResult(GJKCPairDoSimplex(Q, A, B, Q2, A2, B2, size,closA, closB));
		GJKCPairDoSimplex(Q, A, B, size,closestA, closestB);
		
		closestPt = (closestA - closestB);
		sDist=closestPt|closestPt;
	}

	//Which mean the support point is jump between two points, we just simple return no intersection
	if(maxIterations == -1)
	{
		return false;
	}
	epa.PenetrationDepth(*a, *b, Q, A, B, size, closestA, closestB);

	const HVec3 dir = closestB - closestA;
	normal = C_Normal(dir);
	contact = closestA;
	penetration = dir.Length();
	return true;
	
}



bool _GJKCPairMargin(C_Convex* a, C_Convex* b, Maths::HVec3& contact, Maths::HVec3& normal, Maths::HF32& penetration, EPA& epa)
{
	using namespace Maths;
	SPUManager manager;
	U32 size=1;

	const HF32 _zero(Constants<HF32>::Zero());
	/*const HVec3 assumedNormDir(a->WarmStart() - b->WarmStart());
	BIZ_ASSERT((assumedNormDir.X().EQ(_zero) & assumedNormDir.Y().EQ(_zero) & assumedNormDir.Z().EQ(_zero))!=1);*/


	bool bQuadratic = manager.QuadraticShape(a) | manager.QuadraticShape(b);
	const HF32 minMargin(Min(a->Margin(), b->Margin()));

	const HF32 marginA(bQuadratic ? a->Margin() : minMargin);
	const HF32 marginB(bQuadratic ? b->Margin() : minMargin);
	const HF32 margin(marginA + marginB);
	const HF32 sqMargin(margin*margin);

	const HF32 epsilon(Min(a->Epsilon(), b->Epsilon()));//Maths::Constants<HF32>::Epsilon());
	const HF32 epsilonSq(epsilon*epsilon);

	const HVec3 _initialSearchDir(a->WarmStart() - b->WarmStart());
	const HVec3 initialSearchDir(Maths::Select(_initialSearchDir.Length2().GT(epsilon), _initialSearchDir, Maths::Constants<Maths::HVec3>::UnitAxisPositiveY()));

	HVec3 supportA(manager.SupportMargin(a, -initialSearchDir, marginA));
	HVec3 supportB(manager.SupportMargin(b,  initialSearchDir, marginB));
	

	
	HVec3 Q[4] = {supportA - supportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {supportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {supportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	
	HVec3 closestPt(Q[0]);
	HF32 sqDist(closestPt|closestPt);
	
	GJKResultType result = E_DEEP_INTERSECTION;

	HVec3 closestA(supportA), closestB(supportB);

	S32 maxIterations = 10;
	
	do
	{	
		supportA=manager.SupportMargin(a, -closestPt, marginA);//aSup(a, -closestPt);
		supportB=manager.SupportMargin(b,  closestPt, marginB);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);
		const HF32 temp(support|closestPt);
		const HF32 con2(sqDist-temp);
		const HF32 sTemp(temp*temp);
		const HF32 con1(sTemp/sqDist);

		//The search direction is -closestPt, therefore, temp should be large than zero to guarantee it is the closest point 
		if(temp.GT(epsilon) & con1.GT(sqMargin))
		{
			a->WarmStart(closestA);
			b->WarmStart(closestB);
			//we don't need the exact closest point
			return false;
		}

		if(con2.LT(epsilon) & sqDist.LE(sqMargin))
		{
			result = E_MARGIN_INTERSECTION;
			break;
		}

		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull, the size of simplex will be reseted in this method
		//Maths::Scalar_Bool bResult(GJKCPairDoSimplex(Q, A, B, Q2, A2, B2, size,closA, closB));
		GJKCPairDoSimplex(Q, A, B, size,closestA, closestB);
		
		closestPt = closestA - closestB;
		sqDist=closestPt|closestPt;
	}
	while(U32(sqDist > epsilonSq) && (maxIterations--));//(!(closestPt.All_EQ(origin)))

	//Which mean the support point is jump between two points, we just simple return no intersection
	if(maxIterations == -1)
	{
		return false;
	}


	if(result == E_MARGIN_INTERSECTION)
	{
		const C_Normalised normalised(closestPt.Normalised());
		const HF32 dis(normalised.Length());
		normal = -normalised.Normal();
		contact = closestA + normal*marginA;
		penetration = margin - dis;

		a->WarmStart(closestA);
		b->WarmStart(closestB);

		return true;
	}
	else if(result == E_DEEP_INTERSECTION)
	{
 		bool bResult = epa.PenetrationDepthMargin(*a, *b, Q, A, B, size, closestA, closestB, marginA, marginB);
		const HVec3 cAcB(closestA - closestB);
		const C_Normalised normalised(cAcB.Normalised());
		const HF32 len(normalised.Length());
		normal = normalised.Normal();
		contact = closestA + normal * marginA;
		penetration = len  + margin;
		a->WarmStart(supportA);
		b->WarmStart(supportB);
		return bResult;
	
	}

	return false;
}

bool _GJKRayCast(C_Convex*a, C_Convex* b, const Maths::HVec3 PCREF r, const Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, Maths::HVec3& normal, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	//DebugBreak();

	using namespace Maths;

	U32 size=1;

	SPUManager manager;
	
	//const HVec3 assumedNormDir(a.Centre() - b.Centre());
	const HVec3 initialSearchDir(a->Centre() - b->Centre());
	const HVec3 initialSupportA(manager.Support(a, initialSearchDir));
	const HVec3 initialSupportB(manager.Support(b, -initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	
	const HF32 _zero(Constants<HF32>::Zero());
	const HF32 _one(Constants<HF32>::One());
	const HVec3 zeroV(Constants<HVec3>::Zero());

	//Initialize the ray
	HVec3 x(zeroV);//the clipping point of the ray
	HF32 _lambda(_zero);
	HVec3 v(x - Q[0]);

	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB);
	
	const HF32 epsilon(Min(a->Epsilon(), b->Epsilon()));
	HVec3 closA(initialSupportA), closB(initialSupportB);
	HF32 sDist(v|v);

	S32 maxIterations = 10;

	do
	{
		
		supportA=manager.Support(a, v);//aSup(a, -closestPt);
		supportB=manager.Support(b, -v);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);

		const HVec3 w(x - support);

		const HF32 vw(v|w);
		const HF32 vr(v|r);
		
		if(vw.GT(_zero))
		{
			//The ray point away from the objects. no interception , reject
			if(vr.GE(_zero))
			{
				//DebugBreak();
				return false;
			}
			else
			{
				_lambda = _lambda - (vw/vr);

				if(_lambda.GT(maxLambda) || _lambda.LT(_zero))
					return false; // the ray hit C but not in next frame
				x = _lambda*r;
				size = 0;
				maxIterations = 10;
			}
		}
		
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;
		

		//calculate the closest point between two convex hull
		GJKCPairDoSimplex(x, Q, A, B, size, closA, closB);
		if(size == 4)
			break;

		v = x - (closA - closB);
		sDist = v|v;

	}
	while(sDist.GT(epsilon) && (maxIterations --));

	lambda = _lambda;
	normal = Maths::Normalise(v);
	closestA = closA;
	closestB = closB;
	/*a.wStart=supportA;
	b.wStart=supportB;*/

	return (maxIterations != -1); 
	
}

#endif
}