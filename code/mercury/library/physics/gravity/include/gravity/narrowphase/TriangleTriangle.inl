#ifndef __GRAVITY_TRIANGLE_TRIANGLE_INL__
#define __GRAVITY_TRIANGLE_TRIANGLE_INL__

//using namespace Maths;


namespace Collision
{

template <int MaxContacts>
bool CollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
								  const Maths::C_Normal PCREF normA, const Maths::C_Normal PCREF normB, Collision::C_ContactPatch<MaxContacts>& patch, 
								  const U32 hashValue, const U32 physicsMaterial, const U32 effectMaterial)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const C_Normal n1(normA, C_Normal::UNIT_LENGTH_VECTOR);//vt10^vt11);
	const HF32 d1(-(n1|t10)); //the constant-normal form of the t1 plane


	//calculate the normal of the traingle t2 plane
	//const HVec3 vt20(t21-t20);
	//const HVec3 vt21(t22-t20);
	const C_Normal n2(normB, C_Normal::UNIT_LENGTH_VECTOR);//vt20^vt21); 
	const HF32 d2(-(n2|t20));//the constant-normal form of the t2 plane

	//the signed distances of each individual vertex of t1 to the t2 plane
	const HF32 dt10((n2|t10) + d2);
	const HF32 dt11((n2|t11) + d2);
	const HF32 dt12((n2|t12) + d2);

	//assert(dt10.NE(zero) && dt11.NE(zero) && dt12.NE(zero));

	const HF32 mt101(dt10*dt11);
	const HF32 mt102(dt10*dt12);
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt101.GT(zero) && mt102.GT(zero)) 
		return false;

	const HF32 dt20((n1|t20) + d1);
	const HF32 dt21((n1|t21) + d1);
	const HF32 dt22((n1|t22) + d1);

	//const Maths::HF32 pen(-Maths::Min(dt20, Maths::Min(dt21, dt22)));

	const HF32 mt201(dt20*dt21);
	const HF32 mt202(dt20*dt22);
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt201.GT(zero) && mt202.GT(zero))
		return false;

	//assert(dt20.NE(zero) && dt21.NE(zero) && dt22.NE(zero));
	/*
		the code below insure t10 is always on the other side of the t2 plane
	*/
	//const Scalar_Bool a0(mt101.LT(zero) & mt102.LT(zero)); //no change
	//const Scalar_Bool a1(mt101.GT(zero) & mt102.LT(zero)); //Swap 0 and 2, else Swap 0 and 1

	//const HVec3 nt10(Select(a0, t10, Select(a1, t12, t11)));
	//const HVec3 nt11(Select(a0, t11, Select(a1, t11, t10)));
	//const HVec3 nt12(Select(a0, t12, Select(a1, t10, t12)));
	//const HF32  ndt10(Select(a0, dt10, Select(a1, dt12, dt11)));
	//const HF32  ndt11(Select(a0, dt11, Select(a1, dt11, dt10)));
	//const HF32  ndt12(Select(a0, dt12, Select(a1, dt10, dt12)));

	

	//const Scalar_Bool ta0(mt101.LT(zero) & mt102.LT(zero)); //no change
	//const Scalar_Bool ta1(mt101.GT(zero) & mt102.LT(zero)); //Swap 0 and 2, else Swap 0 and 1

	//const HVec3 tnt10(Select(ta0, t10, Select(ta1, t12, t11)));
	//const HVec3 tnt11(Select(ta0, t11, Select(ta1, t11, t10)));
	//const HVec3 tnt12(Select(ta0, t12, Select(ta1, t10, t12)));
	//const HF32  tndt10(Select(ta0, dt10, Select(ta1, dt12, dt11)));
	//const HF32  tndt11(Select(ta0, dt11, Select(ta1, dt11, dt10)));
	//const HF32  tndt12(Select(ta0, dt12, Select(ta1, dt10, dt12)));

	const Scalar_Bool a0(mt101.LE(zero) & mt102.GE(zero)); //no change
	const Scalar_Bool a1(mt101.GE(zero) & mt102.LE(zero)); //Swap 0 and 2, else Swap 0 and 1

	const HVec3 nt10(Select(a0, t11, Select(a1, t12, t10)));
	const HVec3 nt11(Select(a0, t10, t11));
	const HVec3 nt12(Select(a1, t10, t12));
	const HF32  ndt10(Select(a0, dt11, Select(a1, dt12, dt10)));
	const HF32  ndt11(Select(a0, dt10, dt11));
	const HF32  ndt12(Select(a1, dt10, dt12));

//	assert(tnt10.All_EQ(nt10) && tnt11.All_EQ(nt11) && tnt12.All_EQ(nt12) && tndt10.EQ(ndt10) && tndt11.EQ(ndt11) && tndt12.EQ(ndt12));


	/*
		the code below insure t20 is always on the other side of the t1 plane
	*/
	//const Scalar_Bool b0(mt201.LT(zero) & mt202.LT(zero)); //no change
	//const Scalar_Bool b1(mt201.GT(zero) & mt202.LT(zero)); //Swap 0 and 2, else swap 0 and 1

	//const HVec3 nt20(Select(b0, t20, Select(b1, t22, t21)));
	//const HVec3 nt21(Select(b0, t21, Select(b1, t21, t20)));
	//const HVec3 nt22(Select(b0, t22, Select(b1, t20, t22)));
	//const HF32  ndt20(Select(b0, dt20, Select(b1, dt22, dt21)));
	//const HF32  ndt21(Select(b0, dt21, Select(b1, dt21, dt20)));
	//const HF32  ndt22(Select(b0, dt22, Select(b1, dt20, dt22)));

	//const Scalar_Bool tb0(mt201.LT(zero) & mt202.LT(zero)); //no change
	//const Scalar_Bool tb1(mt201.GT(zero) & mt202.LT(zero)); //Swap 0 and 2, else swap 0 and 1

	//const HVec3 tnt20(Select(tb0, t20, Select(tb1, t22, t21)));
	//const HVec3 tnt21(Select(tb0, t21, Select(tb1, t21, t20)));
	//const HVec3 tnt22(Select(tb0, t22, Select(tb1, t20, t22)));
	//const HF32  tndt20(Select(tb0, dt20, Select(tb1, dt22, dt21)));
	//const HF32  tndt21(Select(tb0, dt21, Select(tb1, dt21, dt20)));
	//const HF32  tndt22(Select(tb0, dt22, Select(tb1, dt20, dt22)));

	const Scalar_Bool b0(mt201.LE(zero) & mt202.GE(zero)); //no change
	const Scalar_Bool b1(mt201.GE(zero) & mt202.LE(zero)); //Swap 0 and 2, else swap 0 and 1

	const HVec3 nt20(Select(b0, t21, Select(b1, t22, t20)));
	const HVec3 nt21(Select(b0, t20,  t21));
	const HVec3 nt22(Select(b1, t20,  t22));
	const HF32  ndt20(Select(b0, dt21, Select(b1, dt22, dt20)));
	const HF32  ndt21(Select(b0, dt20, dt21));
	const HF32  ndt22(Select(b1, dt20, dt22));


	//assert(tnt20.All_EQ(nt20) && tnt21.All_EQ(nt21) && tnt22.All_EQ(nt22) && tndt20.EQ(ndt20) && tndt21.EQ(ndt21) && tndt22.EQ(ndt22));

	/*
		after the above test, the t1 plane and the t2 plane will intersect in a line, whose direction is the
		cross product of these two plane's normal
	*/
	const HVec3 dir(n1^n2);

	//calculate the intersect intervals of t1 and the line
	const HF32 pt10(dir|nt10);
	const HF32 pt11(dir|nt11);
	const HF32 pt12(dir|nt12);

	const HF32 _t1_min=pt10+(pt11-pt10)*(ndt10/(ndt10-ndt11));
	const HF32 _t1_max=pt10+(pt12-pt10)*(ndt10/(ndt10-ndt12));
	
	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Scalar_Bool con1(_t1_min.GT(_t1_max));
	const HF32 t1_min(Select(con1, _t1_max, _t1_min));
	const HF32 t1_max(Select(con1, _t1_min, _t1_max));
	const HVec3 nnt11(Select(con1, nt12, nt11));
	const HVec3 nnt12(Select(con1, nt11, nt12));
	const HF32 ddt11(Select(con1, ndt12, ndt11));
	const HF32 ddt12(Select(con1, ndt11, ndt12));


	//calculate the intersect intervals of t2 and the line
	const HF32 pt20(dir|nt20);
	const HF32 pt21(dir|nt21);
	const HF32 pt22(dir|nt22);

	const HF32 _t2_min=pt20+(pt21-pt20)*(ndt20/(ndt20-ndt21));
	const HF32 _t2_max=pt20+(pt22-pt20)*(ndt20/(ndt20-ndt22));


	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Scalar_Bool con2(_t2_min.GT(_t2_max));
	const HF32 t2_min(Select(con2, _t2_max, _t2_min));
	const HF32 t2_max(Select(con2, _t2_min, _t2_max));
	const HVec3 nnt21(Select(con2, nt22, nt21));
	const HVec3 nnt22(Select(con2, nt21, nt22));
	const HF32 ddt21(Select(con2, ndt22, ndt21));
	const HF32 ddt22(Select(con2, ndt21, ndt22));


	//Triangle intersect in one line
	if(t2_max.GE(t1_min) & t2_min.LE(t1_max))
	{
		const HF32 adt12(ddt12.Abs());
		const HF32 adt11(ddt11.Abs());
		const HF32 adt10(ndt10.Abs());

		const HF32 adt22(ddt22.Abs());
		const HF32 adt21(ddt21.Abs());
		const HF32 adt20(ndt20.Abs());

		const HF32 pen(Min(adt20, Min(adt21, adt22)));

		const HVec3 v210(nnt21 - nt20);
		const HF32 p210=adt20/(adt20+adt21);
		const HVec3 contact210(nt20 + p210 * v210);
	
		const HVec3 v110(nnt11 - nt10);
		const HF32 p110=adt10/(adt10+adt11);
		const HVec3 contact110(nt10 + p110 * v110);


		const HVec3 v120(nnt12 - nt10);
		const HF32 p120=adt10/(adt10+adt12);
		const HVec3 contact120(nt10 + p120 * v120);
	
		const HVec3 v220(nnt22 - nt20);
		const HF32 p220=adt20/(adt20+adt22);
		const HVec3 contact220(nt20 + p220 * v220);
		

		const Scalar_Bool con1(t1_min.LE(t2_min));
		const Scalar_Bool con2(t1_max.LE(t2_max));

		if(patch.m_NumContactPoints < MaxContacts)
		{
			C_ContactPoint& point1(patch.m_ContactPoints[patch.m_NumContactPoints++]);
			point1.m_ContactPoint = Select(con1, contact210, contact110);
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1.m_ContactPoint, 0.5f, Maths::RGBA(255, 0, 0));
			point1.m_HashValue = hashValue << 1;
			//point1.m_PenetrationDepth=Select(con1, adt20, adt10);
			//point1.m_PenetrationDepth=Maths::Max(ndt20, zero);
			//point1.m_PenetrationDepth=pen;
			Store(pen, &point1.m_PenetrationDepth);
			point1.m_PhysicsMaterialIndices = physicsMaterial;
			point1.m_EffectMaterialIndices = effectMaterial;

			
			if(patch.m_NumContactPoints < MaxContacts)
			{
				C_ContactPoint& point2(patch.m_ContactPoints[patch.m_NumContactPoints++]);
				point2.m_ContactPoint = Select(con2, contact120, contact220);
				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2.m_ContactPoint, 0.5f, Maths::RGBA(0, 0, 255));
				point2.m_HashValue = (hashValue << 1) | 1;
				//point2.m_PenetrationDepth=Select(con2, adt10, adt20);
				//point2.m_PenetrationDepth=Maths::Max(ndt20, zero);
				//point2.m_PenetrationDepth=pen;
				Store(pen, &point2.m_PenetrationDepth);
				point2.m_PhysicsMaterialIndices = physicsMaterial;
				point2.m_EffectMaterialIndices = effectMaterial;
			}
		}

		return true;

	}
	return false;
}




template <int MaxContacts>
bool TomasCollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
								  const Maths::C_Normal PCREF normA, const Maths::C_Normal PCREF normB, Collision::C_ContactPatch<MaxContacts>& patch, 
								  const U32 hashValue, const U32 physicsMaterial, const U32 effectMaterial)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	//calculate the normal of the triangle t1 plane
	const HVec3 vt10(t11-t10);
	const HVec3 vt11(t12-t10);
	const C_Normal n1(vt10^vt11);
	const HF32 d1(-(n1|t10)); //the constant-normal form of the t1 plane


	//calculate the normal of the traingle t2 plane
	const HVec3 vt20(t21-t20);
	const HVec3 vt21(t22-t20);
	const C_Normal n2(vt20^vt21); 
	const HF32 d2(-(n2|t20));//the constant-normal form of the t2 plane

	//the signed distances of each individual vertex of t1 to the t2 plane
	const HF32 dt10((n2|t10) + d2);
	const HF32 dt11((n2|t11) + d2);
	const HF32 dt12((n2|t12) + d2);



	const HF32 mt101=dt10*dt11;
	const HF32 mt102=dt10*dt12;
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt101.GT(zero) && mt102.GT(zero)) 
		return false;

	const HF32 dt20((n1|t20) + d1);
	const HF32 dt21((n1|t21) + d1);
	const HF32 dt22((n1|t22) + d1);

	const HF32 mt201=dt20*dt21;
	const HF32 mt202=dt20*dt22;
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt201.GT(zero) && mt202.GT(zero))
		return false;

	
	/*
		the code below insure t10 is always on the other side of the t2 plane
	*/
	const Maths::Scalar_Bool a0(mt101.LT(zero) & mt102.LT(zero)); //no change
	const Maths::Scalar_Bool a1(mt101.GT(zero) & mt102.LT(zero)); //Swap 0 and 2, else Swap 0 and 1

	const Maths::HVec3 nt10(Select(a0, t10, Select(a1, t12, t11)));
	const Maths::HVec3 nt11(Select(a0, t11, Select(a1, t11, t10)));
	const Maths::HVec3 nt12(Select(a0, t12, Select(a1, t10, t12)));
	const Maths::HF32  ndt10(Select(a0, dt10, Select(a1, dt12, dt11)));
	const Maths::HF32  ndt11(Select(a0, dt11, Select(a1, dt11, dt10)));
	const Maths::HF32  ndt12(Select(a0, dt12, Select(a1, dt10, dt12)));


	/*
		the code below insure t20 is always on the other side of the t1 plane
	*/
	const Maths::Scalar_Bool b0(mt201.LT(zero) & mt202.LT(zero)); //no change
	const Maths::Scalar_Bool b1(mt201.GT(zero) & mt202.LT(zero)); //Swap 0 and 2, else swap 0 and 1

	const Maths::HVec3 nt20(Select(b0, t20, Select(b1, t22, t21)));
	const Maths::HVec3 nt21(Select(b0, t21, Select(b1, t21, t20)));
	const Maths::HVec3 nt22(Select(b0, t22, Select(b1, t20, t22)));
	const Maths::HF32  ndt20(Select(b0, dt20, Select(b1, dt22, dt21)));
	const Maths::HF32  ndt21(Select(b0, dt21, Select(b1, dt21, dt20)));
	const Maths::HF32  ndt22(Select(b0, dt22, Select(b1, dt20, dt22)));

	/*
		after the above test, the t1 plane and the t2 plane will intersect in a line, whose direction is the
		cross product of these two plane's normal
	*/
	const HVec3 dir(n1^n2);

	//calculate the intersect intervals of t1 and the line
	const HF32 pt10(dir|nt10);
	const HF32 pt11(dir|nt11);
	const HF32 pt12(dir|nt12);

	const HF32 _t1_min=pt10+(pt11-pt10)*(ndt10/(ndt10-ndt11));
	const HF32 _t1_max=pt10+(pt12-pt10)*(ndt10/(ndt10-ndt12));

	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Maths::Scalar_Bool con1(_t1_min.GT(_t1_max));
	const Maths::HF32 t1_min(Select(con1, _t1_max, _t1_min));
	const Maths::HF32 t1_max(Select(con1, _t1_min, _t1_max));
	const Maths::HVec3 nnt11(Select(con1, nt12, nt11));
	const Maths::HVec3 nnt12(Select(con1, nt11, nt12));
	const Maths::HF32 ddt11(Select(con1, ndt12, ndt11));
	const Maths::HF32 ddt12(Select(con1, ndt11, ndt12));


	//calculate the intersect intervals of t2 and the line
	const HF32 pt20(dir|nt20);
	const HF32 pt21(dir|nt21);
	const HF32 pt22(dir|nt22);

	const HF32 _t2_min=pt20+(pt21-pt20)*(ndt20/(ndt20-ndt21));
	const HF32 _t2_max=pt20+(pt22-pt20)*(ndt20/(ndt20-ndt22));

	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Maths::Scalar_Bool con2(_t2_min.GT(_t2_max));
	const Maths::HF32 t2_min(Select(con2, _t2_max, _t2_min));
	const Maths::HF32 t2_max(Select(con2, _t2_min, _t2_max));
	const Maths::HVec3 nnt21(Select(con2, nt22, nt21));
	const Maths::HVec3 nnt22(Select(con2, nt21, nt22));
	const Maths::HF32 ddt21(Select(con2, ndt22, ndt21));
	const Maths::HF32 ddt22(Select(con2, ndt21, ndt22));


	//Triangle intersect in one line
	if(t2_max.GE(t1_min) && t2_min.LE(t1_max))
	{
		const Maths::HF32 adt12(ddt12.Abs());
		const Maths::HF32 adt11(ddt11.Abs());
		const Maths::HF32 adt10(ndt10.Abs());
		//const HF32 pen2(Min(adt10, Min(adt11, adt12)));

		const Maths::HF32 adt22(ddt22.Abs());
		const Maths::HF32 adt21(ddt21.Abs());
		const Maths::HF32 adt20(ndt20.Abs());
		//const HF32 pen(Min(adt20, Min(adt21, adt22)));
		const HF32 pen(adt20);

		const Maths::HVec3 v210(nnt21 - nt20);
		const Maths::HF32 p210=adt20/(adt20+adt21);
		const Maths::HVec3 contact210(nt20 + p210 * v210);

		const Maths::HVec3 v110(nnt11 - nt10);
		const Maths::HF32 p110=adt10/(adt10+adt11);
		const Maths::HVec3 contact110(nt10 + p110 * v110);

		const Maths::HVec3 v120(nnt12 - nt10);
		const Maths::HF32 p120=adt10/(adt10+adt12);
		const Maths::HVec3 contact120(nt10 + p120 * v120);

		const Maths::HVec3 v220(nnt22 - nt20);
		const Maths::HF32 p220=adt20/(adt20+adt22);
		const Maths::HVec3 contact220(nt20 + p220 * v220);

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(nt10, "nt10");
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(nt20, "nt20");

		const Maths::Scalar_Bool con1(t1_min.LE(t2_min));
		const Maths::Scalar_Bool con2(t1_max.LE(t2_max));

		if(patch.m_NumContactPoints < MaxContacts)
		{
			C_ContactPoint& point1(patch.m_ContactPoints[patch.m_NumContactPoints++]);
			point1.m_ContactPoint = Select(con1, contact210, contact110);
			point1.m_HashValue = hashValue << 1;
			//point1.m_PenetrationDepth=pen;
			Store(pen, &point1.m_PenetrationDepth);
			point1.m_PhysicsMaterialIndices = physicsMaterial;
			point1.m_EffectMaterialIndices = effectMaterial;


			if(patch.m_NumContactPoints < MaxContacts)
			{
				C_ContactPoint& point2(patch.m_ContactPoints[patch.m_NumContactPoints++]);
				point2.m_ContactPoint = Select(con2, contact120, contact220);
				point2.m_HashValue = (hashValue << 1) | 1;
				//point2.m_PenetrationDepth=pen;
				Store(pen, &point2.m_PenetrationDepth);
				point2.m_PhysicsMaterialIndices = physicsMaterial;
				point2.m_EffectMaterialIndices = effectMaterial;

			}
		}
		return true;

	}
	return false;
}

template <int MaxContacts>
bool OrenCollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
								  const Maths::C_Normal PCREF normA, const Maths::C_Normal PCREF normB, Collision::C_ContactPatch<MaxContacts>& patch,
								  const U32 hashValue, const U32 physicsMaterial, const U32 effectMaterial)
{
	using namespace Maths;

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
	const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HVec3 v110(t11 - t10);
	const Maths::HVec3 v120(t12 - t10);
	const Maths::HVec3 v121(t12 - t11);

	const Maths::HVec3 v210(t21 - t20);
	const Maths::HVec3 v220(t22 - t20);
	const Maths::HVec3 v221(t22 - t21);

	const Maths::HVec3 comp(v110 ^ v120);
	const Maths::HF32 nDeterminant0((comp | v210) + epsilon);
	const Maths::HF32 nDeterminant1((comp | v220) + epsilon);
	const Maths::HF32 nDeterminant2((comp | v221) + epsilon);

	const Maths::HF32 dDeterminant1(comp | (t20 - t10));//t20 + a0*v210 and t20 + a1*v220
	const Maths::HF32 dDeterminant2(comp | (t21 - t10));//t21 + a2*v221

	//DebugBreak();

	const Maths::HF32 a0(-dDeterminant1 / nDeterminant0); //t20 + a0*v210
	const Maths::HF32 a1(-dDeterminant1 / nDeterminant1); //t20 + a1*v220
	const Maths::HF32 a2(-dDeterminant2 / nDeterminant2); //t21 + a2*v221

	//const Maths::HF32 a0(-Maths::UnsafeDivide(dDeterminant1, nDeterminant0)); //t20 + a0*v210
	//const Maths::HF32 a1(-Maths::UnsafeDivide(dDeterminant1, nDeterminant1)); //t20 + a1*v220
	//const Maths::HF32 a2(-Maths::UnsafeDivide(dDeterminant2, nDeterminant2)); //t21 + a2*v221

	const Maths::Scalar_Bool con0(a0.GE(zero) & a0.LE(one)); 
	const Maths::Scalar_Bool con1(a1.GE(zero) & a1.LE(one)); 
	const Maths::Scalar_Bool con2(a2.GE(zero) & a2.LE(one)); 

	//No triangle intersection
	if((!con0) & (!con1) & (!con2))
		return false;

	const Maths::Scalar_Bool ccon1(con0&con1); //t20 is the right point
	const Maths::Scalar_Bool ccon2(con0&con2); //t21 is the right point

	const Maths::HVec3 nt20=Select(ccon1, t20, Select(ccon2, t21, t22));
	const Maths::HVec3 np0 =Select(ccon1, v210, Select(ccon2, -v210, -v220));
	const Maths::HVec3 np1 =Select(ccon1, v220, Select(ccon2,  v221, -v221));
	const Maths::HF32 b0  =Select(ccon1, a0, Select(ccon2, one-a0, one-a1));
	const Maths::HF32 b1  =Select(ccon1, a1, Select(ccon2, a2, one-a2));

	//Calculate the one intersection points of triangle 2 and plane 1
	const Maths::HVec3 point (nt20 + b0*np0);
	const Maths::HVec3 point2(nt20 + b1*np1);

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point, 0.5f,  RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point, point2, RGBA(0, 255, 0, 255));*/
	//Calculate the vector connect point1 and the second points
	const Maths::HVec3 vt(point2 - point);
	const Maths::HF32 vt_X(vt.X());
	const Maths::HF32 vt_Y(vt.Y());
	const Maths::HF32 vt_Z(vt.Z());

	const Maths::HVec3 r(point - t10);
	const Maths::HF32 v110_X(v110.X());
	const Maths::HF32 v110_Y(v110.Y());
	const Maths::HF32 nD0(v110_X*vt_Y - v110_Y*vt_X);
	const Maths::HF32 dD00(r.X()*vt_Y - r.Y()*vt_X);
	const Maths::HF32 dD01(v110_X*r.Y() - v110_Y*r.X());
	const Maths::HF32 lambda0(dD00/(nD0+epsilon));
	const Maths::HF32 mu0(-dD01/(nD0+epsilon));

	/*const Maths::HF32 lambda0(Maths::UnsafeDivide(dD00, (nD0+epsilon)));
	const Maths::HF32 mu0(-Maths::UnsafeDivide(dD01, (nD0+epsilon)));*/

	const Maths::HF32 v120_X(v120.X());
	const Maths::HF32 v120_Y(v120.Y());
	const Maths::HF32 nD1(v120_X*vt_Y - v120_Y*vt_X);
	const Maths::HF32 dD10(dD00);
	const Maths::HF32 dD11(v120_X*r.Y() - v120_Y*r.X());
	const Maths::HF32 lambda1(dD10/(nD1+epsilon));
	const Maths::HF32 mu1(-dD11/(nD1+epsilon));

	/*const Maths::HF32 lambda1(Maths::UnsafeDivide(dD10, (nD1+epsilon)));
	const Maths::HF32 mu1(-Maths::UnsafeDivide(dD11, (nD1+epsilon)));*/


	const Maths::HF32 v121_X(v121.X());
	const Maths::HF32 v121_Y(v121.Y());
	const Maths::HVec3 r1(point - t11);
	const Maths::HF32 nD2(v121_X*vt_Y - v121_Y*vt_X);
	const Maths::HF32 dD20(r1.X()*vt_Y - r1.Y()*vt_X);
	const Maths::HF32 dD21(v121_X*r1.Y() - v121_Y*r1.X());
	const Maths::HF32 lambda2(dD20/(nD2+epsilon));
	const Maths::HF32 mu2(-dD21/(nD2+epsilon));
	/*const Maths::HF32 lambda2(Maths::UnsafeDivide(dD20, (nD2+epsilon)));
	const Maths::HF32 mu2(-Maths::UnsafeDivide(dD21, (nD2+epsilon)));*/


	const Maths::Scalar_Bool c0(lambda0.GE(zero) & lambda0.LE(one));
	const Maths::Scalar_Bool c1(lambda1.GE(zero) & lambda1.LE(one));
	const Maths::Scalar_Bool c2(lambda2.GE(zero) & lambda2.LE(one));

	//No intersection
	if((!c0) & (!c1) & (!c2))
		return false;

	const Maths::Scalar_Bool cc0(c0&c1);
	const Maths::Scalar_Bool cc1(c0&c2);

	const Maths::HF32 mm0=Select(cc0, mu0, Select(cc1, mu0, mu1));
	const Maths::HF32 mm1=Select(cc0, mu1, mu2);

	const Maths::HF32 mu_min=Min(mm0, mm1);
	const Maths::HF32 mu_max=Max(mm0, mm1);

	const Maths::HF32 mu(mu_min*mu_max);

	const Maths::Scalar_Bool mCon0(mu_min.GE(zero) & mu_min.LE(one));
	const Maths::Scalar_Bool mCon1(mu_max.GE(zero) & mu_max.LE(one));
	const Maths::Scalar_Bool muCon(mu.GE(zero)); //in the same side


	if( muCon & !mCon0)
		return false;

	const Maths::HF32 d(normA|t10);
	const Maths::HF32 p0((normA|t20) - d);
	const Maths::HF32 p1((normA|t21) - d);
	const Maths::HF32 p2((normA|t22) - d);
	//const Maths::HF32 pen=Min(p0.Abs(), Min(p1.Abs(), p2.Abs()));
	const Maths::HF32 pen=Maths::Abs(Min(p0, Min(p1, p2)));

	if(patch.m_NumContactPoints < MaxContacts) // N.B. - we won't get here if we didn't have space for at least 1 POC!
	{
		C_ContactPoint& point1(patch.m_ContactPoints[patch.m_NumContactPoints++]);
		point1.m_ContactPoint = Select(muCon, point + mu_min*vt, Select(mCon1, point + mu_max*vt, point2));
		point1.m_HashValue = hashValue << 1;
		//point1.m_PenetrationDepth=pen;
		Store(pen, &point1.m_PenetrationDepth);
		point1.m_PhysicsMaterialIndices = physicsMaterial;
		point1.m_EffectMaterialIndices = effectMaterial;
	}

	if(patch.m_NumContactPoints < MaxContacts)
	{
		C_ContactPoint& point1(patch.m_ContactPoints[patch.m_NumContactPoints++]);
		point1.m_ContactPoint = Select(muCon, Select(mCon1, point+mu_max*vt, point2),point);
		point1.m_HashValue = hashValue << 1;
		//point1.m_PenetrationDepth=pen;
		Store(pen, &point1.m_PenetrationDepth);
		point1.m_PhysicsMaterialIndices = physicsMaterial;
		point1.m_EffectMaterialIndices = effectMaterial;
	}

	return true;
	
}


};

#endif
