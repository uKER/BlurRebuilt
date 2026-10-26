#include "stdafx.h"
#include <EditorLib/Xt/XtFieldHelper.h>

#include <EditorLib/Core.h>

namespace EditorLib
{
namespace XtFieldHelper
{

void GetVector(Maths::HVec3& v, Xt::DomBlock* pBlock)
{
	v.X((F32)atof(Xt::AsText(Xt::At(pBlock, "vx"))));
	v.Y((F32)atof(Xt::AsText(Xt::At(pBlock, "vy"))));
	v.Z((F32)atof(Xt::AsText(Xt::At(pBlock, "vz"))));
}

void SetVector(Xt::DomBlock* pBlock, const Maths::HVec3& v)
{
	Xt::Set(Xt::AcquireField(pBlock, "vx"), (const char*)Format("%f", (F32)v.X()));
	Xt::Set(Xt::AcquireField(pBlock, "vy"), (const char*)Format("%f", (F32)v.Y()));
	Xt::Set(Xt::AcquireField(pBlock, "vz"), (const char*)Format("%f", (F32)v.Z()));
}

void GetVector(Vector3& v, Xt::DomBlock* pBlock)
{
	v.x = (F32)atof(Xt::AsText(Xt::At(pBlock, "vx")));
	v.y = (F32)atof(Xt::AsText(Xt::At(pBlock, "vy")));
	v.z = (F32)atof(Xt::AsText(Xt::At(pBlock, "vz")));
}

void SetVector(Xt::DomBlock* pBlock, const Vector3& v)
{
	Xt::Set(Xt::AcquireField(pBlock, "vx"), (const char*)Format("%f", v.x));
	Xt::Set(Xt::AcquireField(pBlock, "vy"), (const char*)Format("%f", v.y));
	Xt::Set(Xt::AcquireField(pBlock, "vz"), (const char*)Format("%f", v.z));
}

void GetMatrix(Matrix44& m, Xt::DomBlock* pBlock)
{
	GetVector((Vector3&)m[0], Xt::AsBlock(Xt::At(pBlock, "ax")));
	GetVector((Vector3&)m[1], Xt::AsBlock(Xt::At(pBlock, "ay")));
	GetVector((Vector3&)m[2], Xt::AsBlock(Xt::At(pBlock, "az")));
	GetVector((Vector3&)m[3], Xt::AsBlock(Xt::At(pBlock, "aw")));

	m[0][3] = 0.f;
	m[1][3] = 0.f;
	m[2][3] = 0.f;
	m[3][3] = 1.f;
}

void SetMatrix(Xt::DomBlock* pBlock, const Matrix44& m)
{
	SetVector(Xt::AcquireBlock(pBlock, "ax"), (Vector3&)m[0]);
	SetVector(Xt::AcquireBlock(pBlock, "ay"), (Vector3&)m[1]);
	SetVector(Xt::AcquireBlock(pBlock, "az"), (Vector3&)m[2]);
	SetVector(Xt::AcquireBlock(pBlock, "aw"), (Vector3&)m[3]);
}

void GetColour(Maths::C_RGBA_U8& colour, Xt::DomBlock* pBlock)
{
	colour.R((U8)(255.f*(F32)atof(Xt::AsText(Xt::At(pBlock, "r")))));
	colour.G((U8)(255.f*(F32)atof(Xt::AsText(Xt::At(pBlock, "g")))));
	colour.B((U8)(255.f*(F32)atof(Xt::AsText(Xt::At(pBlock, "b")))));
	colour.A((U8)(255.f*(F32)atof(Xt::AsText(Xt::At(pBlock, "a")))));
}

void SetColour(Xt::DomBlock* pBlock, const Maths::C_RGBA_U8& colour)
{
	const F32 s = 1.f/255.f;
	Xt::Set(Xt::AcquireField(pBlock, "r"), (const char*)Format("%f", s*colour.R()));
	Xt::Set(Xt::AcquireField(pBlock, "g"), (const char*)Format("%f", s*colour.G()));
	Xt::Set(Xt::AcquireField(pBlock, "b"), (const char*)Format("%f", s*colour.B()));
	Xt::Set(Xt::AcquireField(pBlock, "a"), (const char*)Format("%f", s*colour.A()));
}

void GetScalar(F32& scalar, Xt::DomField* pField)
{
	scalar = (F32)atof(Xt::AsText(pField));
}

void SetScalar(Xt::DomField* pField, const F32& scalar)
{
	Xt::Set(pField, Format("%f", scalar));
}

void  GetScalar(S32& scalar, Xt::DomField* pField)
{
	scalar = (S32)atoi(Xt::AsText(pField));
}

void  SetScalar(Xt::DomField* pField, const S32& scalar)
{
	Xt::Set(pField, Format("%i", scalar));
}

void GetScalar(U32& scalar, Xt::DomField* pField)
{
	scalar = (U32)strtoul(Xt::AsText(pField), 0, 0);
}

void SetScalar(Xt::DomField* pField, U32 scalar)
{
	Xt::Set(pField, Format("%lu", scalar));
}

void GetScalar(U8& scalar, Xt::DomField* pField)
{
	U32 tmp = (U32)strtoul(Xt::AsText(pField), 0, 0);
	scalar = (U8)tmp;
}

void SetScalar(Xt::DomField* pField, U8 scalar)
{
	Xt::Set(pField, Format("%lu", (U32)scalar));
}

void GetBool(bool& boolean, Xt::DomField* pField, bool defaultValue)
{
	if (!pField || pField->value == "")
	{
		boolean = defaultValue;
		return;
	}

	if (pField->value == "1")
		boolean = true;
	else
		boolean = false;
}

void SetBool(const bool boolean, Xt::DomField* pField)
{
	BIZ_ASSERT(pField);
	boolean ? pField->value = "1" : pField->value = "0";
}

bool IsNullVector(Xt::DomBlock* pBlock)
{
	if (IsNullScalar(Xt::AsField(Xt::At(pBlock, "vx"))) &&
		IsNullScalar(Xt::AsField(Xt::At(pBlock, "vy"))) &&
		IsNullScalar(Xt::AsField(Xt::At(pBlock, "vz"))))
		return true;

	return false;
}

bool IsNullMatrix(Xt::DomBlock* pBlock)
{
	if (IsNullVector(Xt::AsBlock(Xt::At(pBlock, "ax"))) &&
		IsNullVector(Xt::AsBlock(Xt::At(pBlock, "ay"))) &&
		IsNullVector(Xt::AsBlock(Xt::At(pBlock, "az"))) &&
		IsNullVector(Xt::AsBlock(Xt::At(pBlock, "aw"))))
		return true;

	return false;
}

bool IsNullScalar(Xt::DomField* pField)
{
	//If field or string is null.
	if (pField && *Xt::AsText(pField))
		return false;

	return true;
}

}
}

