#include <stdafx.h>
#include "gravity/narrowphase/Primitive.h"


//using namespace Maths;

namespace Collision
{

/*HVec3 OBB::Support(const ConvexHull& convex, const Maths::HVec3 PCREF dir) const 
{
	const HVec3 zero(Constants<HVec3>::Zero());
	const HVec3 ld (dir * u.InverseRotation());
	const Vector3_Bool b(ld < zero);
	const HVec3 v = Select(b, -e, e);
	

	const HVec3 ld2(dir|u.AX(), dir|u.AY(), dir|u.AZ());
	const Vector3_Bool b2(ld2 < zero);
	const HVec3 v1=Select(b2, -e, e);
	assert(v1.All_EQ(v));
	return  c+( v * u);
}
*/


/*Maths::HVec3 OBB::Support(const ConvexHull& convex, const Maths::HVec3 PCREF dir)
{
	const OBB& obb = (const OBB&)convex;
	const Maths::HVec3 zero(Maths::Constants<HVec3>::Zero());
	const Maths::HVec3 e(obb.e);
	const Maths::HVec3 emin(-e);
	const Maths::HMat33 u(obb.u);
	const Maths::HVec3 c(obb.c);
	
	const Maths::HVec3 ld2(dir|u.AX(), dir|u.AY(), dir|u.AZ());
	const Maths::Vector3_Bool b2(ld2 < (zero));
	const Maths::HVec3 v(Select(b2, emin, e));

	return  c+( v * u);
}
*/



bool IntersectOBBSegment(const C_OBB& obb, const Maths::HVec3 PCREF p0, const Maths::HVec3 PCREF p1)
{
	using namespace Maths;

	const HVec3 obbc(obb.Centre());
	const HVec3 e(obb.Extent());
	const HMat33 u = obb.Orientation();
	const HMat33& uInver(u.InverseRotation());
	const HVec3 half(Maths::Constants<HVec3>::Half());;
	const HVec3 a((p1-obbc)*uInver);
	const HVec3 b((p0-obbc)*uInver);
	const HVec3 mid((a+b)*half); //translate box and segment to origin
	const HVec3 d(b-mid);
	const HVec3 absm(Abs(mid));
	const HVec3 absd(Abs(d));
	const HVec3 temp(e+absd);

	if(!(absm.All_LE(temp)))
		return false;

	const HVec3 epislon(Maths::Constants<HVec3>::Epsilon2());
	const HVec3 ed(absd+epislon);
	
	const HF32 c1((mid.Y()*d.Z()-mid.Z()*d.Y()).Abs());
	const HF32 c2((mid.Z()*d.X()-mid.X()*d.Z()).Abs());
	const HF32 c3((mid.X()*d.Y()-mid.Y()*d.X()).Abs());

	const HF32 d1(e.Y()*ed.Z() +e.Z()*ed.Y());
	const HF32 d2(e.X()*ed.Z() +e.Z()*ed.X());
	const HF32 d3(e.X()*ed.Y() +e.Y()*ed.X());

	const HVec3 con(c1, c2, c3);
	const HVec3 don(d1, d2, d3);

	return (con.All_LE(don)==1);
}



bool IntersectOBBs(const C_OBB& a, const C_OBB& b)
{
	using namespace Maths;

	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon3());
	const HVec3 ae(a.Extent());
	const HVec3 be(b.Extent());
	const HMat33 au=a.Orientation();
	const HMat33& bu(b.Orientation());
	//Compute the rotation matrix expressing b in a's coordinate
	const HMat33 _mat(au.InverseRotation()*bu);
	/*
		Compute common subexpressions. Add in an epsilon term to conteract arithmetic errors when
		two edges are parallel and their cross product is (near) null
	*/
	const HMat33 mat(Abs(_mat.AX()) + epsilon, Abs(_mat.AY()) + epsilon, Abs(_mat.AZ()) + epsilon);
	const HMat33 inverse_mat(mat.InverseRotation());

	//Compute translation vector t
	const HVec3 _t(b.Centre()-a.Centre());
	const HVec3 t(_t*au); 
	
	//Test axes a.u[0], a.u[1], a.u[2]
	{
		const HVec3 rb(be*mat);
		const HVec3 t1(Maths::Abs(t));
		const HVec3 aerb(ae + rb);
		if(!t1.All_LE(aerb))
			return false;
	}


	//Test axes b.u[0], b.u[1], b.u[2]
	{
		const HVec3 ra(ae*inverse_mat);
		const HVec3 t1(t*_mat);
		const HVec3 t2(Maths::Abs(t1));
		if(!t2.All_LE(ra+be))
			return false;
	}

	const HF32 aex(ae.X());
	const HF32 aey(ae.Y());
	const HF32 aez(ae.Z());

	const HF32 bex(be.X());
	const HF32 bey(be.Y());
	const HF32 bez(be.Z());

	const HVec3 matX(mat.AX());
	const HVec3 matY(mat.AY());
	const HVec3 matZ(mat.AZ());
	

	const HF32 mat00(matX.X());
	const HF32 mat01(matX.Y());
	const HF32 mat02(matX.Z());
	

	const HF32 mat10(matY.X());
	const HF32 mat11(matY.Y());
	const HF32 mat12(matY.Z());
	
	const HF32 mat20(matZ.X());
	const HF32 mat21(matZ.Y());
	const HF32 mat22(matZ.Z());

	const HVec3 _matX(_mat.AX());
	const HVec3 _matY(_mat.AY());
	const HVec3 _matZ(_mat.AZ());
	

	const HF32 _mat00(_matX.X());
	const HF32 _mat01(_matX.Y());
	const HF32 _mat02(_matX.Z());

	const HF32 _mat10(_matY.X());
	const HF32 _mat11(_matY.Y());
	const HF32 _mat12(_matY.Z());
	
	const HF32 _mat20(_matZ.X());
	const HF32 _mat21(_matZ.Y());
	const HF32 _mat22(_matZ.Z());
	
	/*
		The tests below do not happen very frequecy
	*/
	//Test axes a.u[0] X b.u[0]
	{
	const HF32 ra=aey*mat20 + aez*mat10; //ra=a.e[1]*mat[2][0] + a.e[2]*mat[1][0]
	const HF32 rb=bey*mat02 + bez*mat01; //rb=b.e[1]*mat[0][2] + b.e[2]*mat[0][1]
	if(Abs(t.Z()*_mat10 - t.Y()*_mat20).GT(ra+rb))//t[2]*_mat[1][0]-t[1]*_mat[2][0]
		return false;
	}

	//Test axes a.u[0] X b.u[1]
	{
	const HF32 ra=aey*mat21 + aez*mat11; //ra=a.e[1]*mat[2][1] + a.e[2]*mat[1][1]
	const HF32 rb=bex*mat02 + bez*mat00; //rb=b.e[0]*mat[0][2] + b.e[2]*mat[0][0]
	if(Abs(t.Z()*_mat11 - t.Y()*_mat21).GT(ra+rb))//t[2]*_mat[1][1]-t[1]*_mat[2][1]
		return false;
	}

	//Test axes a.u[0] X b.u[2]
	{
	const HF32 ra=aey*mat22+ aez*mat12; //ra=a.e[1]*mat[2][2] + a.e[2]*mat[1][2]
	const HF32 rb=bex*mat01 + bey*mat00; //rb=b.e[0]*mat[0][1] + b.e[1]*mat[0][0]
	if(Abs(t.Z()*_mat12 - t.Y()*_mat22).GT(ra+rb))//t[2]*_mat[1][2]-t[1]*_mat[2][2]
		return false;
	}

	//Test axes a.u[1] X b.u[0]
	{
	const HF32 ra=aex*mat20 + aez*mat00; //ra=a.e[0]*mat[2][0] + a.e[2]*mat[0][0]
	const HF32 rb=bey*mat12 + bez*mat11; //rb=b.e[1]*mat[1][2] + b.e[2]*mat[1][1]
	if(Abs(t.X()*_mat20 - t.Z()*_mat00).GT(ra+rb))//t[0]*_mat[2][0]-t[2]*_mat[0][0]
		return false;
	}
	
	//Test axes a.u[1] X b.u[1]
	{
	const HF32 ra=aex*mat21 + aez*mat01; //ra=a.e[0]*mat[2][1] + a.e[2]*mat[0][1]
	const HF32 rb=bex*mat12 + bez*mat10; //rb=b.e[0]*mat[1][2] + b.e[2]*mat[1][0]
	if(Abs(t.X()*_mat21 - t.Z()*_mat01).GT(ra+rb))//t[0]*_mat[2][1]-t[2]*_mat[0][1]
		return false;
	}

	//Test axes a.u[1] X b.u[2]
	{
	const HF32 ra=aex*mat22 + aez*mat02; //ra=a.e[0]*mat[2][2] + a.e[2]*mat[0][2]
	const HF32 rb=bex*mat11 + bey*mat10; //rb=b.e[0]*mat[1][1] + b.e[1]*mat[1][0]
	if(Abs(t.X()*_mat22- t.Z()*_mat02).GT(ra+rb))//t[0]*_mat[2][2]-t[2]*_mat[0][2]
		return false;
	}

	//Test axes a.u[2] X b.u[0]
	{
	const HF32 ra=aex*mat10 + aey*mat00; //ra=a.e[0]*mat[1][0] + a.e[1]*mat[0][0]
	const HF32 rb=bey*mat22 + bez*mat21; //rb=b.e[1]*mat[2][2] + b.e[2]*mat[2][1]
	if(Abs(t.Y()*_mat00 - t.X()*_mat10).GT(ra+rb))//t[1]*_mat[0][0]-t[0]*_mat[1][0]
		return false;
	}

	//Test axes a.u[2] X b.u[1]
	{
	const HF32 ra=aex*mat11 + aey*mat01; //ra=a.e[0]*mat[1][1] + a.e[1]*mat[0][1]
	const HF32 rb=bex*mat22 + bez*mat20; //rb=b.e[0]*mat[2][2] + b.e[2]*mat[2][0]
	if(Abs(t.Y()*_mat01 - t.X()*_mat11).GT(ra+rb))//t[1]*_mat[0][1]-t[0]*_mat[1][1]
		return false;
	}

	//Test axes a.u[2] X b.u[2]
	{
	const HF32 ra=aex*mat12 + aey*mat02; //ra=a.e[0]*mat[1][2] + a.e[1]*mat[0][2]
	const HF32 rb=bex*mat21 + bey*mat20; //rb=b.e[0]*mat[2][1] + b.e[1]*mat[2][0]
	if(Abs(t.Y()*_mat02 - t.X()*_mat12).GT(ra+rb))//t[1]*_mat[0][2]-t[0]*_mat[1][2]
		return false;
	}

	return true;
}


bool IntersectOBBs2(const C_OBB& a, const C_OBB& b)
{
	using namespace Maths;

	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon3());
	const HMat33 u(a.Orientation());
	const HVec3 ae(a.Extent());
	const HVec3 be(b.Extent());
	//Compute the rotation matrix expressing b in a's coordinate
	const HMat33 _mat(u.InverseRotation()*b.Orientation());
	/*
		Compute common subexpressions. Add in an epsilon term to conteract arithmetic errors when
		two edges are parallel and their cross product is (near) null
	*/
	const HMat33 mat(Abs(_mat.AX()) + epsilon, Abs(_mat.AY()) + epsilon, Abs(_mat.AZ()) + epsilon);
	const HMat33 inverse_mat(mat.InverseRotation());

	//Compute translation vector t
	const HVec3 _t(b.Centre()-a.Centre());
	const HVec3 t(_t*u); 
	
	//Test axes a.u[0], a.u[1], a.u[2]
	{
		const HVec3 rb(be*mat);
		const HVec3 t1(Maths::Abs(t));
		const HVec3 aerb(ae + rb);
		if(!t1.All_LE(aerb))
			return false;
	}


	//Test axes b.u[0], b.u[1], b.u[2]
	{
		const HVec3 ra(ae*inverse_mat);
		const HVec3 t1(t*_mat);
		const HVec3 t2(Maths::Abs(t1));
		if(!t2.All_LE(ra+be))
			return false;
	}

	const HF32 aex(ae.X());
	const HF32 aey(ae.Y());
	const HF32 aez(ae.Z());

	const HF32 bex(be.X());
	const HF32 bey(be.Y());
	const HF32 bez(be.Z());

	const HVec3 matX(mat.AX());
	const HVec3 matY(mat.AY());
	const HVec3 matZ(mat.AZ());
	

	const HF32 mat00(matX.X());
	const HF32 mat01(matX.Y());
	const HF32 mat02(matX.Z());

	const HVec3 _matX(_mat.AX());
	const HVec3 _matY(_mat.AY());
	const HVec3 _matZ(_mat.AZ());
	
	/*
		The tests below do not happen very frequecy
	*/
	
	
	const HVec3 b1(bey, bex, bex);//(1, 0, 0)
	const HVec3 b2(bez, bez, bey);//(2, 2, 1)


	//Batch process a.u[0]X b.u[0], a.u[0] X b.u[1], a.u[0] X b.u[2]
	{
	const HVec3 ra((aey*matZ)+(aez*matY));

	const HVec3 temp1(mat02, mat02, mat01);
	const HVec3 temp2(mat01, mat00, mat00);

	const HVec3 rb((b1 * temp1) + (b2 * temp2));

	const HVec3 absc(Abs((t.Z()*_matY) - (t.Y()*_matZ)));
	
	if(!(absc.All_LT(ra+rb)))
		return false;
	}
	
	const HF32 mat10(matY.X());
	const HF32 mat11(matY.Y());
	const HF32 mat12(matY.Z());
	
	
	//Batch process a.u[1]X b.u[0], a.u[1] X b.u[1], a.u[1] X b.u[2]
	{
	
	const HVec3 ra((aex*matZ)+(aez*matX));

	const HVec3 temp1(mat12, mat12, mat11);
	const HVec3 temp2(mat11, mat10, mat10);

	const HVec3 rb((b1 * temp1) + (b2 * temp2));
	const HVec3 absc(Abs((t.X()*_matZ) - (t.Z()*_matX)));
	if(!(absc.All_LT(ra+rb)))
		return false;
	}
	
	const HF32 mat20(matZ.X());
	const HF32 mat21(matZ.Y());
	const HF32 mat22(matZ.Z());
	//Batch process a.u[2]X b.u[0], a.u[2] X b.u[1], a.u[2] X b.u[2]
	{
	
	const HVec3 ra((aex*matY)+(aey*matX));

	const HVec3 temp1(mat22, mat22, mat21);
	const HVec3 temp2(mat21, mat20, mat20);

	const HVec3 rb((b1 * temp1) + (b2 * temp2));
	const HVec3 absc(Abs((t.Y()*_matX) - (t.X()*_matY)));
	if(!(absc.All_LT(ra+rb)))
		return false;
	}
	

	return true;
}

bool IntersectOBBPlane(const C_OBB& obb, const Maths::HVec3 PCREF n, const Maths::HF32 PCREF d)
{
	using namespace Maths;

	const HVec3 e(obb.Extent());
	const HMat33 u(obb.Orientation());

	const HF32 r =	e.X()*Abs(n|u.AX())+
					e.Y()*Abs(n|u.AY())+
					e.Z()*Abs(n|u.AZ());
	const HF32 s=(n|obb.Centre()) - d;
	return Abs(s).LE(r);
}

bool IntersectOBBTriangle(const C_OBB& obb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	using namespace Maths;

	const HVec3 ca(obb.Centre());
	const HVec3 e(obb.Extent());
	//const HMat33 u(obb.u);
	const HMat33& uo = obb.Orientation(); 
	const HMat33& u = uo.InverseRotation();//obb.u.InverseRotation());

	const HVec3 na(a-ca);
	const HVec3 nb(b-ca);
	const HVec3 nc(c-ca);

	//Translate triangle as conceptually moving AABB to origin
	const HVec3 v0(na*u); 
	const HVec3 v1(nb*u);
	const HVec3 v2(nc*u);

	//Compute edge vectors for triangle
	const HVec3 f0(v1-v0);
	const HVec3 f1(v2-v1);
	const HVec3 f2(v0-v2);

	
	const HF32 v0Y(v0.Y());
	const HF32 v0Z(v0.Z());

	const HF32 v2Y(v2.Y());
	const HF32 v2Z(v2.Z());
	

	const HF32 f0Y(f0.Y());
	const HF32 f0Z(f0.Z());
	

	const HF32 absf0Y(f0Y.Abs());
	const HF32 absf0Z(f0Z.Abs());
	//const HVec3 absF0(Abs(f0));

	HF32 p0(Maths::UNDEFINED), p1(Maths::UNDEFINED), ra(Maths::UNDEFINED), max(Maths::UNDEFINED), min(Maths::UNDEFINED);
	//Test axis a00=u0 X f0, all triangle vertexs project onto axis a00
	
	
	p0=(v0Z*f0Y - v0Y*f0Z);
	p1=(v2Z*f0Y - v2Y*f0Z);
	ra=(e.Y()*absf0Z + e.Z()*absf0Y);
	max=Max(p0, p1);
	min=Min(p0, p1);
	if(max < (-ra) || min > (ra))
		return false;
	
	
	
	const HF32 f0X(f0.X());
	const HF32 v0X(v0.X());
	const HF32 v2X(v2.X());
	const HF32 absf0X(f0X.Abs());
	//Test axis a10=u1 X f0, all triangle vertex project onto axis a10

	p0=(v0X*f0.Z()-v0Z*f0.X());
	p1=(v2X*f0.Z()-v2Z*f0.X());
	ra=(e.X()*absf0Z+e.Z()*absf0X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;	
	
	//Test axis a20=u2 X f0, all triangle vertexs project onto axis a20
	p0=(v0Y*f0.X()-v0X*f0.Y());
	p1=(v2Y*f0.X()-v2X*f0.Y());
	
	ra=(e.X()*absf0Y+e.Y()*absf0X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;


	const HF32 v1Y(v1.Y());
	const HF32 v1Z(v1.Z());

	const HF32 f1X(f1.X());
	const HF32 f1Y(f1.Y());
	const HF32 f1Z(f1.Z());
	const HF32 absf1X(f1X.Abs());
	const HF32 absf1Y(f1Y.Abs());
	const HF32 absf1Z(f1Z.Abs());
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01

	p0=(v0Z*f1Y-v0Y*f1Z);
	p1=(v1Z*f1Y-v1Y*f1Z);
	ra=(e.Y()*absf1Z + e.Z()*absf1Y);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	

	const HF32 v1X(v1.X());
	//Test axis a11=u1 X f1, all triangle vertexs project onto axis a11
	
	p0=(v0X*f1Z-v0Z*f1X);
	p1=(v1X*f1Z-v1Z*f1X);
	ra=(e.X()*absf1Z+e.Z()*absf1X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	
	
	

	//Test axis a21=u2 X f1, all triangle vertexs project onto axis a21
	
	p0=(v0Y*f1X-v0X*f1Y);
	p1=(v1Y*f1X-v1X*f1Y);
	ra=(e.X()*absf1Y+e.Y()*absf1X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	
	
	const HF32 f2X(f2.X());
	const HF32 f2Y(f2.Y());
	const HF32 f2Z(f2.Z());
	const HF32 absf2X(f2X.Abs());
	const HF32 absf2Y(f2Y.Abs());
	const HF32 absf2Z(f2Z.Abs());

	//Test axis a02=u0 X f2, all triangle vertexs project onto axis a02
	p0=(v0Z*f2Y-v0Y*f2Z);
	p1=(v1Z*f2Y-v1Y*f2Z);
	ra=(e.Y()*absf2Z+ e.Z()*absf2Y);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	
	
	
	//Test axis a12=u1 X f2, all triangle vertexs project onto axis a12
	p0=(v0X*f2Z-v0Z*f2X);
	p1=(v1X*f2Z-v1Z*f2X);
	ra=(e.X()*absf2Z+e.Z()*absf2X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	

	//Test axis a21=u2 X f2, all triangle vertexs project onto axis a22
	p0=(v0Y*f2X-v0X*f2Y);
	p1=(v1Y*f2X-v1X*f2Y);
	ra=(e.X()*absf2Y+e.Y()*absf2X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) | min > (ra))
		return false;
	


	const HVec3 max012(Max(v0, Max(v1, v2)));
	const HVec3 min012(Min(v0, Min(v1, v2)));
	//Test axis u[0]=(1,0,0), u[1]={0,1,0}, u[2]={0, 0, 1}
	if(!(max012.All_GT(-e) & min012.All_LT(e)))
		return false;
	//U32 vector_b(max012.AllLE(-e) | min012.All_GE(e));
	//Vector3_Bool vecb((max012 <= -e) || (min012 >= e));

	//Test triangle normal and AABB intersect
	
	const HVec3 n(f0^f1);
	const HF32 d(n|v0);
	const HVec3 absn(Abs(n));
	//const HF32 r=ex*Abs(n.X())+ey*Abs(n.Y())+ez*Abs(n.Z());
	const HF32 r(e|absn);
	//const HF32 s=(n|aabb.c)-d;
	return Abs(d).LE(r);	
}


};
