#ifndef AFTK_EULER_H
#define AFTK_EULER_H

#include <core/maths/hvec/new/IncludeGeneral.h>
#include <core/maths/hvec/new/IncludeGeneral.inl>

namespace Aftk
{
	inline void MatrixToEulerZYX(Vector3F32& euler, const Matrix44& m)
	{
		//Normalise axes to remove scaling and improve numerical stability.
		Vector3F32 i (m[0][0], m[0][1], m[0][2]);
		Vector3F32 j (m[1][0], m[1][1], m[1][2]);
		Vector3F32 k (m[2][0], m[2][1], m[2][2]);

		UtMaths::SafeNormalise(&i);
		UtMaths::SafeNormalise(&j);
		UtMaths::SafeNormalise(&k);

		F32 sinTheta20 = Maths::Clamped(j[0], -1.f, 1.f);
		euler[1] = asinf(sinTheta20);
		if ( euler[1] < Maths::Pi/2 )
		{
			if ( euler[1] > -Maths::Pi/2 )
			{
				euler[0] = atan2(-k[1],k[2]);
				euler[2] = atan2(-j[0],i[0]);
			}
			else
			{
				// not a unique solution
				euler[0] = -atan2(i[1],j[1]);
				euler[2] = 0;
			}
		}
		else
		{
			// not a unique solution
			euler[0] = atan2(i[1],j[1]);
			euler[2] = 0;
		}
	}

	inline void MatrixToEulerYZX(Vector3F32& euler, const Matrix44& m)
	{
		//Normalise axes to remove scaling and improve numerical stability.
		Vector3F32 i (m[0][0], m[0][1], m[0][2]);
		Vector3F32 j (m[1][0], m[1][1], m[1][2]);
		Vector3F32 k (m[2][0], m[2][1], m[2][2]);

		UtMaths::SafeNormalise(&i);
		UtMaths::SafeNormalise(&j);
		UtMaths::SafeNormalise(&k);

		F32 sinTheta10 = Maths::Clamped(j[0], -1.f, 1.f);
		euler[2] = asin(-sinTheta10);
		if ( euler[2] < Maths::Pi/2 )
		{
			if ( euler[2] > -Maths::Pi/2 )
			{
				euler[0] = atan2(j[2],j[1]);
				euler[1] = atan2(k[0],i[0]);
			}
			else
			{
				// not a unique solution
				euler[0] = -atan2(-i[2],k[2]);
				euler[1] = 0;
			}
		}
		else
		{
			// not a unique solution
			euler[0] = atan2(-i[2],k[2]);
			euler[1] = 0;
		}
	}

	inline void MatrixToEulerZXY(Vector3F32& euler, const Matrix44& m)
	{
		//Normalise axes to remove scaling and improve numerical stability.
		Vector3F32 i (m[0][0], m[0][1], m[0][2]);
		Vector3F32 j (m[1][0], m[1][1], m[1][2]);
		Vector3F32 k (m[2][0], m[2][1], m[2][2]);

		UtMaths::SafeNormalise(&i);
		UtMaths::SafeNormalise(&j);
		UtMaths::SafeNormalise(&k);

		F32 sinTheta21 = Maths::Clamped(k[1], -1.f, 1.f);
		euler[0] = asin(-sinTheta21);
		if ( euler[0] < Maths::Pi/2 )
		{
			if ( euler[0] > -Maths::Pi/2 )
			{
				euler[1] = atan2(k[0],k[2]);
				euler[2] = atan2(i[1],j[1]);
			}
			else
			{
				// not a unique solution
				euler[1] = -atan2(-j[0],i[0]);
				euler[2] = 0;
			}
		}
		else
		{
			// not a unique solution
			euler[1] = atan2(-j[0],i[0]);
			euler[2] = 0;
		}
	}

	inline void MatrixToEulerXZY(Vector3F32& euler, const Matrix44& m)
	{
		//Normalise axes to remove scaling and improve numerical stability.
		Vector3F32 i (m[0][0], m[0][1], m[0][2]);
		Vector3F32 j (m[1][0], m[1][1], m[1][2]);
		Vector3F32 k (m[2][0], m[2][1], m[2][2]);

		UtMaths::SafeNormalise(&i);
		UtMaths::SafeNormalise(&j);
		UtMaths::SafeNormalise(&k);

		F32 sinTheta01 = Maths::Clamped(i[1], -1.f, 1.f);
		euler[2] = asin(sinTheta01);
		if ( euler[2] < Maths::Pi/2 )
		{
			if ( euler[2] > -Maths::Pi/2 )
			{
				euler[1] = atan2(-i[2],i[0]);
				euler[0] = atan2(-k[1],j[1]);
			}
			else
			{
				// not a unique solution
				euler[1] = -atan2(j[2],k[2]);
				euler[0] = 0;
			}
		}
		else
		{
			// not a unique solution
			euler[1] = atan2(j[2],k[2]);
			euler[0] = 0;
		}
	}

	inline void MatrixToEulerYXZ(Vector3F32& euler, const Matrix44& m)
	{
		//Normalise axes to remove scaling and improve numerical stability.
		Vector3F32 i (m[0][0], m[0][1], m[0][2]);
		Vector3F32 j (m[1][0], m[1][1], m[1][2]);
		Vector3F32 k (m[2][0], m[2][1], m[2][2]);

		UtMaths::SafeNormalise(&i);
		UtMaths::SafeNormalise(&j);
		UtMaths::SafeNormalise(&k);

		F32 sinTheta12 = Maths::Clamped(j[2], -1.f, 1.f);
		euler[0] = asin(sinTheta12);
		if ( euler[0] < Maths::Pi/2 )
		{
			if ( euler[0] > -Maths::Pi/2 )
			{
				euler[2] = atan2(-j[0],j[1]);
				euler[1] = atan2(-i[2],k[2]);
			}
			else
			{
				// not a unique solution
				euler[2] = -atan2(k[0],i[0]);
				euler[1] = 0;
			}
		}
		else
		{
			// not a unique solution
			euler[2] = atan2(k[0],i[0]);
			euler[1] = 0;
		}
	}

	inline void MatrixToEulerXYZ(Vector3F32& euler, const Matrix44& m)
	{
		//Normalise axes to remove scaling and improve numerical stability.
		Vector3F32 i (m[0][0], m[0][1], m[0][2]);
		Vector3F32 j (m[1][0], m[1][1], m[1][2]);
		Vector3F32 k (m[2][0], m[2][1], m[2][2]);

		UtMaths::SafeNormalise(&i);
		UtMaths::SafeNormalise(&j);
		UtMaths::SafeNormalise(&k);

		F32 sinTheta02 = Maths::Clamped(i[2], -1.f, 1.f);
		euler[1] = asin(-sinTheta02);
		if ( euler[1] < Maths::Pi/2 )
		{
			if ( euler[1] > -Maths::Pi/2 )
			{
				euler[2] = atan2(i[1],i[0]);
				euler[0] = atan2(j[2],k[2]);
			}
			else
			{
				// not a unique solution
				euler[2] = -atan2(-j[0],k[0]);
				euler[0] = 0;
			}
		}
		else
		{
			// not a unique solution
			euler[2] = atan2(-j[0],k[0]);
			euler[0] = 0;
		}
	}

	inline void MatrixToEuler(Maths::HVec3& euler, const Maths::HMat33& m, E_RotationOrder rotationOrder)
	{	
		switch(rotationOrder)
		{
		case E_RotationOrder_XYZ:
			{
			Maths::EulerXYZ tmpEuler = Maths::MakeEulerXYZ(m);
			euler = Maths::HVec3(Maths::X(tmpEuler).m_V, Maths::Y(tmpEuler).m_V, Maths::Z(tmpEuler).m_V);
			return;
			}

		case E_RotationOrder_YZX:
			{
			Maths::EulerYXZ tmpEuler = Maths::MakeEulerYXZ(m);
			euler = Maths::HVec3(Maths::X(tmpEuler).m_V, Maths::Y(tmpEuler).m_V, Maths::Z(tmpEuler).m_V);
			return;
			}

		case E_RotationOrder_ZXY:
			{
			Maths::EulerZXY tmpEuler = Maths::MakeEulerZXY(m);
			euler = Maths::HVec3(Maths::X(tmpEuler).m_V, Maths::Y(tmpEuler).m_V, Maths::Z(tmpEuler).m_V);
			return;
			}

		case E_RotationOrder_XZY:
			{
			Maths::EulerXZY tmpEuler = Maths::MakeEulerXZY(m);
			euler = Maths::HVec3(Maths::X(tmpEuler).m_V, Maths::Y(tmpEuler).m_V, Maths::Z(tmpEuler).m_V);
			return;
			}

		case E_RotationOrder_YXZ:
			{
			Maths::EulerYXZ tmpEuler = Maths::MakeEulerYXZ(m);
			euler = Maths::HVec3(Maths::X(tmpEuler).m_V, Maths::Y(tmpEuler).m_V, Maths::Z(tmpEuler).m_V);
			return;
			}

		case E_RotationOrder_ZYX:
			{
			Maths::EulerZYX tmpEuler = Maths::MakeEulerZYX(m);
			euler = Maths::HVec3(Maths::X(tmpEuler).m_V, Maths::Y(tmpEuler).m_V, Maths::Z(tmpEuler).m_V);
			return;
			}

		default:
			euler = Maths::HVec3(0,0,0);
			return;
		};
	}

	inline void MatrixToEuler(Vector3F32& euler, const Matrix44& m, E_RotationOrder rotationOrder)
	{
		Maths::HMat33 tmp;
		TemplateToNightmareMatrix33(tmp, m);

		Maths::HVec3 tmpEuler;
		MatrixToEuler(tmpEuler, tmp, rotationOrder);

		//euler[0] = Maths::X(tmpEuler);//.m_V;
		//euler[1] = Maths::Y(tmpEuler);//.m_V;
		//euler[2] = Maths::Z(tmpEuler);//.m_V;
		euler[0] = tmpEuler.X();//.m_V;
		euler[1] = tmpEuler.Y();//.m_V;
		euler[2] = tmpEuler.Z();//.m_V;
	}

	inline void EulerToMatrix(Maths::HMat33& out, const Maths::HVec3& euler, E_RotationOrder rotationOrder)
	{
		Maths::Mat33 rx = Maths::MakeMat33_RotX(Maths::MakeVF32(euler.X()));
		Maths::Mat33 ry = Maths::MakeMat33_RotY(Maths::MakeVF32(euler.Y()));
		Maths::Mat33 rz = Maths::MakeMat33_RotZ(Maths::MakeVF32(euler.Z()));

		switch(rotationOrder)
		{
		case E_RotationOrder_XYZ:
			out = rx * ry * rz;
			break;

		case E_RotationOrder_YZX:
			out = ry * rz * rx;
			break;

		case E_RotationOrder_ZXY:
			out = rz * rx * ry;
			break;

		case E_RotationOrder_XZY:
			out = rx * rz * ry;
			break;

		case E_RotationOrder_YXZ:
			out = ry * rx * rz;
			break;

		case E_RotationOrder_ZYX:
			out = rz * ry * rx;
			break;

		default:
			out.SetIdentity();
		};
	}

	inline void EulerToMatrix(Matrix44& out, const Vector3F32& euler, E_RotationOrder rotationOrder)
	{
		Maths::HMat33 hmatOut;
		Maths::HVec3 eulerIn(euler[0], euler[1], euler[2]);
		EulerToMatrix(hmatOut, eulerIn, rotationOrder);
		NightmareToTemplateMatrix33(out, hmatOut);
	}


	inline void EulerToMatrixXYZ(Maths::HMat33& out, const Maths::HVec3& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_XYZ);
	}

	inline void EulerToMatrixYZX(Maths::HMat33& out, const Maths::HVec3& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_YZX);
	}

	inline void EulerToMatrixZXY(Maths::HMat33& out, const Maths::HVec3& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_ZXY);
	}

	inline void EulerToMatrixXZY(Maths::HMat33& out, const Maths::HVec3& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_XZY);
	}

	inline void EulerToMatrixYXZ(Maths::HMat33& out, const Maths::HVec3& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_YXZ);
	}

	inline void EulerToMatrixZYX(Maths::HMat33& out, const Maths::HVec3& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_ZYX);
	}


	inline void EulerToMatrixXYZ(Matrix44& out, const Vector3F32& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_XYZ);
	}

	inline void EulerToMatrixYZX(Matrix44& out, const Vector3F32& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_YZX);
	}

	inline void EulerToMatrixZXY(Matrix44& out, const Vector3F32& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_ZXY);
	}

	inline void EulerToMatrixXZY(Matrix44& out, const Vector3F32& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_XZY);
	}

	inline void EulerToMatrixYXZ(Matrix44& out, const Vector3F32& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_YXZ);
	}

	inline void EulerToMatrixZYX(Matrix44& out, const Vector3F32& euler)
	{
		return EulerToMatrix(out, euler, E_RotationOrder_ZYX);
	}
}

#endif
