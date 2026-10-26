#include "stdafx.h"
#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>

namespace EditorLib
{
namespace PlugHelpers
{

C_NullPlug* CreateCompoundVec3(
	I_DgNode* pParent,
	void(*pOnChangedCallback)(I_Plug*),
	E_PlugType type,
	const char* name, 
	F32 x, F32 y, F32 z)
{
	C_NullPlug* pPlug = new C_NullPlug(pParent, type, name);
	C_StaticPlug<F32>* v[3];
	v[0] = new C_StaticPlug<F32>(pPlug, pOnChangedCallback, type, "x");
	v[1] = new C_StaticPlug<F32>(pPlug, pOnChangedCallback, type, "y");
	v[2] = new C_StaticPlug<F32>(pPlug, pOnChangedCallback, type, "z");
	v[0]->SetValue(x); v[1]->SetValue(y); v[2]->SetValue(z);
	return pPlug;
}

const I_DgNode* GetTopLevelParent(const I_Plug& plug)
{
	const I_Plug* pPlug = &plug;
	//Find top level parent.
	while(!pPlug->IsTopLevel())
	{
		pPlug = (I_Plug*)pPlug->GetParentPlug();
	}
	//Set top level parent.
	return pPlug->GetParentNode();
}

I_DgNode* GetTopLevelParent(I_Plug& plug)
{
	//Lets do a const cast to save typing.
	return const_cast<I_DgNode*>(GetTopLevelParent((const I_Plug&)plug));
}

E_CompoundType GetCompoundType(const I_Plug& plug)
{
	U32 nChildren = plug.GetNumChildren();
	if ((nChildren < 2) || (nChildren > 4))
		return E_CompoundType_Unknown;

	Util::E_VarType t = plug.GetChild(0)->GetDataType();

	for (U32 i = 1; i < nChildren; i++)
	{
		if (plug.GetChild(i)->GetDataType() != t)
			return E_CompoundType_Unknown;
	}
	if (nChildren == 4)
	{
		switch(t)
		{
		case Util::E_VarType_U32:
			return E_CompoundType_Vector4U32;
		case Util::E_VarType_U64:
			return E_CompoundType_Vector4U64;
		case Util::E_VarType_S32:
			return E_CompoundType_Vector4S32;
		case Util::E_VarType_S64:
			return E_CompoundType_Vector4S64;
		case Util::E_VarType_F32:
			return E_CompoundType_Vector4F32;
		case Util::E_VarType_F64:
			return E_CompoundType_Vector4F64;
		default:
			break;
		}
	}
	else if (nChildren == 3)
	{
		switch(t)
		{
		case Util::E_VarType_U32:
			return E_CompoundType_Vector3U32;
		case Util::E_VarType_U64:
			return E_CompoundType_Vector3U64;
		case Util::E_VarType_S32:
			return E_CompoundType_Vector3S32;
		case Util::E_VarType_S64:
			return E_CompoundType_Vector3S64;
		case Util::E_VarType_F32:
			return E_CompoundType_Vector3F32;
		case Util::E_VarType_F64:
			return E_CompoundType_Vector3F64;
		default:
			break;
		}
	}
	else
	{
		switch(t)
		{
		case Util::E_VarType_U32:
			return E_CompoundType_Vector2U32;
		case Util::E_VarType_U64:
			return E_CompoundType_Vector2U64;
		case Util::E_VarType_S32:
			return E_CompoundType_Vector2S32;
		case Util::E_VarType_S64:
			return E_CompoundType_Vector2S64;
		case Util::E_VarType_F32:
			return E_CompoundType_Vector2F32;
		case Util::E_VarType_F64:
			return E_CompoundType_Vector2F64;
		default:
			break;
		}
	}

	return E_CompoundType_Unknown;
}

void SetMatrixComponents(
	C_NullPlug& scalePlug, 
	C_NullPlug& shearPlug, 
	C_NullPlug& rotatePlug, 
	C_NullPlug& translatePlug, 
	E_RotationOrder order, 
	const Matrix44& matrix,
	bool triggerTopLevelParentOnChanged)
{
	//We need to be able to invert our matrices.
	if (UtMaths::GetDeterminant4x4(matrix) == 0.f)
	{
//		assert(!"Non invertable matrix used in SetMatrixComponents!");
		return;
	}

	Maths::HMat33 tempNightmare;
	Aftk::TemplateToNightmareMatrix33(tempNightmare, matrix);

	MathsStorage::Float33 tempStore;
	Maths::Store<Maths::HMat33, MathsStorage::Float33>(tempNightmare, &tempStore);

	Maths::Mat33 temp;
	temp = Maths::Load<Maths::Mat33>(&tempStore);

	Maths::ScaleMat3 scale;
	Maths::ShearMat3 shear;
	Maths::Mat33 rotate;
	Maths::DecomposeScaleShearRotate(temp, &scale, &shear, &rotate);

	Maths::EulerXYZ rotateXYZ = Maths::MakeEulerXYZ(rotate);

	SetCompoundValue(scalePlug, Vector3(X(scale).m_V, Y(scale).m_V, Z(scale).m_V), triggerTopLevelParentOnChanged);
	SetCompoundValue(shearPlug, Vector3(GetXY(shear).m_V, GetXZ(shear).m_V, GetYZ(shear).m_V), triggerTopLevelParentOnChanged);
	SetCompoundValue(rotatePlug, Vector3(X(rotateXYZ).m_V, Y(rotateXYZ).m_V, Z(rotateXYZ).m_V), triggerTopLevelParentOnChanged);
	SetCompoundValue(translatePlug, Vector3(matrix[3].X(), matrix[3].Y(), matrix[3].Z()), triggerTopLevelParentOnChanged);
}

void SetMatrix(
	Matrix44& matrix,
	const C_NullPlug& scalePlug, 
	const C_NullPlug& shearPlug, 
	const C_NullPlug& rotatePlug, 
	const C_NullPlug& translatePlug, 
	E_RotationOrder order,
	bool update)
{
	Vector3 translate;
	Vector3 scale;
	Vector3 shear;
	Matrix44 rotate;
	Vector3 scalePivot(0,0,0);
	Vector3 scalePivotTranslation(0,0,0);
	Vector3 rotatePivot(0,0,0);
	Matrix44 rotateOrientation;
	UtMaths::SetIdentity(&rotateOrientation);
	Vector3 rotatePivotTranslation(0,0,0);

	GetCompoundValue(translatePlug, translate, update);
	GetCompoundValue(scalePlug, scale, update);
	GetCompoundValue(shearPlug, shear, update);

	{
		Vector3 rotateEuler;
		GetCompoundValue(rotatePlug, rotateEuler, update);
		Aftk::EulerToMatrix(rotate, rotateEuler, (Aftk::E_RotationOrder)order);
	}

	Aftk::BuildMatrix(
		matrix,
		scalePivot,
		scale,
		shear,
		scalePivotTranslation,
		rotatePivot,
		rotateOrientation,
		rotate, 
		rotatePivotTranslation,
		translate);
}

void SetRotationMatrix(
	Matrix44& matrix,
	const C_NullPlug& rotatePlug, 
	E_RotationOrder order,
	bool update)
{
	Vector3 rotateEuler;
	GetCompoundValue(rotatePlug, rotateEuler, update);
	Aftk::EulerToMatrix(matrix, rotateEuler, (Aftk::E_RotationOrder)order);
}

void SetRotationMatrixComponents(
	C_NullPlug& rotatePlug, 
	E_RotationOrder order,
	const Matrix44& matrix,
	bool triggerTopLevelParentOnChanged)
{
	Vector3 rotateEuler;
	Aftk::MatrixToEuler(rotateEuler, matrix, (Aftk::E_RotationOrder)order);
	SetCompoundValue(rotatePlug, rotateEuler, triggerTopLevelParentOnChanged);
}

bool IsChildOfOrEqualTo(I_Plug* pParent, I_Plug* pChild)
{
	if (pParent == pChild) 
		return true;

	if (!pParent || !pChild) 
		return false;

	I_Plug* pCurrentPlug = pChild->GetParentPlug();
	while (pCurrentPlug)
	{
		if (pCurrentPlug == pParent)
			return true;

		pCurrentPlug = pCurrentPlug->GetParentPlug();
	}

	return false;
}

bool AreRelated(I_Plug* l, I_Plug* r)
{
	if (l == r) 
		return true;

	if (!l || !r) 
		return false;

	return IsChildOfOrEqualTo(l, r) || IsChildOfOrEqualTo(r, l);
}

}
}
