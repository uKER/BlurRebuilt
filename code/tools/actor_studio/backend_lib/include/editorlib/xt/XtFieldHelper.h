#ifndef EDITOR_LIB_XT_FIELD_HELPER_H
#define EDITOR_LIB_XT_FIELD_HELPER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	namespace XtFieldHelper
	{
		void GetVector(Maths::HVec3& vector, Xt::DomBlock* pBlock);
		void SetVector(Xt::DomBlock* pBlock, const Maths::HVec3& v);

		void GetVector(Vector3& vector, Xt::DomBlock* pBlock);
		void SetVector(Xt::DomBlock* pBlock, const Vector3& v);

		void GetMatrix(Matrix44& m, Xt::DomBlock* pBlock);
		void SetMatrix(Xt::DomBlock* pBlock, const Matrix44& m);	

		void GetColour(Maths::C_RGBA_U8& colour, Xt::DomBlock* pBlock);
		void SetColour(Xt::DomBlock* pBlock, const Maths::C_RGBA_U8& colour);

		void GetScalar(F32& scalar, Xt::DomField* pField);
		void SetScalar(Xt::DomField* pField, const F32& scalar);

		void GetScalar(S32& scalar, Xt::DomField* pField);
		void SetScalar(Xt::DomField* pField, const S32& scalar);

		void GetScalar(U32& scalar, Xt::DomField* pField);
		void SetScalar(Xt::DomField* pField, U32 scalar);

		void GetScalar(U8& scalar, Xt::DomField* pField);
		void SetScalar(Xt::DomField* pField, U8 scalar);

		void GetBool(bool& boolean, Xt::DomField* pField, bool defaultValue = false);
		void SetBool(const bool boolean, Xt::DomField* pField);

		bool IsNullVector(Xt::DomBlock* pBlock);
		bool IsNullMatrix(Xt::DomBlock* pBlock);
		bool IsNullScalar(Xt::DomField* pField);
	};
}

#endif
