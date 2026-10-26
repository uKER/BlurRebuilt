#ifndef UIMATHS_H
#define UIMATHS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIDefines.h"
#include "ui/Game/UILua.h"
#include "ui/Game/UIXt.h"

#include <btl/Containers/containers.h>

namespace UI
{

/// @addtogroup Maths

// other stuff

//class UIEquation
//{
//public:
//	UIEquation(){single=true;data=NULL;mSingleData=0.f;};
//	~UIEquation()
//	{
//		if(data) delete[] data;
//	};
//
//	void Add(int pos,float var);
//	float WorkoutBracket(float *equation,int *index);
//	bool Solve(float num=0.0f);
//	float Var;
//
//private:
//	bool single;
//	float *data;
//	float mSingleData;
//};

class CatmullRom
{
public:
	void				Init( );

	const Maths::HVec3	Result( ) { return m_Result; }
	void				Solve(const F32 t);
	void				AddControlPoint(const U32 controlPoint, const Maths::HVec3& in);
private:
	void				Solve(const Maths::HVec3& v0,const Maths::HVec3& v1,const Maths::HVec3& v2,const Maths::HVec3& v3,const F32 s);

private:
	static const U32	m_TotalControlPoints = 4;
	bool				m_Closed;
	Maths::HVec3		m_Result;
	Maths::HVec3		m_ControlPoints[m_TotalControlPoints];
};

class UISpline
{
public:
	void AddPoint1(float x,float y,float z){x1=x;y1=y;z1=z;};
	void AddPoint2(float x,float y,float z){x2=x;y2=y;z2=z;};

	void AddControlPoint1(float x,float y,float z){cx1=x;cy1=y;cz1=z;};
	void AddControlPoint2(float x,float y,float z){cx2=x;cy2=y;cz2=z;};

	void Solve(float iter);

	void GetXYZ(float *px,float *py,float *pz){*px=x;*py=y;*pz=z;};

private:
	float x,y,z;

	float x1,y1,z1;
	float x2,y2,z2;

	float cx1,cy1,cz1;
	float cx2,cy2,cz2;
};

} // namespace UI

#endif // UIMATHS_H

