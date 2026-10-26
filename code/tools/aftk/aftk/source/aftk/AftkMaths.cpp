#include <stdafx.h>
#include <Aftk/AftkEuler.h>

//#pragma optimize("", off)
namespace Aftk
{
	Vector4F32 ConvertU32ToVector4F32(U32 srcColour)
	{
		Vector4F32 colourVec;

		F32 scale = 1.0f/255.0f;

		U8 r = (U8)(srcColour >> 0) & 0xff;
		U8 g = (U8)(srcColour >> 8) & 0xff;
		U8 b = (U8)(srcColour >> 16) & 0xff;
		U8 a = (U8)(srcColour >> 24) & 0xff;

		F32 fr = (F32)r;
		F32 fg = (F32)g;
		F32 fb = (F32)b;
		F32 fa = (F32)a;

		colourVec[0] = r;
		colourVec[1] = g;
		colourVec[2] = b;
		colourVec[3] = a;

		colourVec = colourVec * scale;

		return colourVec;
	}

	U32 ConvertVector4F32ToU32(const Vector4F32 &colourVec)
	{
		F32 colourF[4];
		for (U32 i = 0; i < 4; i++)
		{
			colourF[i] = colourVec[i];
			if (colourF[i] > 1.0f)
			{
				colourF[i] = 1.0f;
			}
			else if (colourF[i] < 0.0f)
			{
				colourF[i] = 0.0f;
			}
			colourF[i] *= 255.0f;
		}
		
		U8 r= (U8) colourF[0];
		U8 g= (U8) colourF[1];
		U8 b= (U8) colourF[2];
		U8 a= (U8) colourF[3];

		U32	rgba = (r)|(g<<8)|(b<<16)|(a<<24);

		return rgba;
	}

	Vector4F32 ConvertU64ToVector4F32(U64 srcColour)
	{
		Vector4F32 colourVec;

		F32 scale = 1.0f/65535.0f;

		U8 r = (U8)(srcColour >> 0) & 0xffff;
		U8 g = (U8)(srcColour >> 16) & 0xffff;
		U8 b = (U8)(srcColour >> 32) & 0xffff;
		U8 a = (U8)(srcColour >> 48) & 0xffff;

		F32 fr = (F32)r;
		F32 fg = (F32)g;
		F32 fb = (F32)b;
		F32 fa = (F32)a;

		colourVec[0] = r;
		colourVec[1] = g;
		colourVec[2] = b;
		colourVec[3] = a;

		colourVec = colourVec * scale;

		return colourVec;
	}

	U64 ConvertVector4F32ToU64(const Vector4F32 &colourVec)
	{
		F32 colourF[4];
		for (U32 i = 0; i < 4; i++)
		{
			colourF[i] = colourVec[i];
			if (colourF[i] > 1.0f)
			{
				colourF[i] = 1.0f;
			}
			else if (colourF[i] < 0.0f)
			{
				colourF[i] = 0.0f;
			}
			colourF[i] *= 65535.0f;
		}

		U16 r= (U16) colourF[0];
		U16 g= (U16) colourF[1];
		U16 b= (U16) colourF[2];
		U16 a= (U16) colourF[3];

		U64	rgba = ((U64)r)|(((U64)g)<<16)|(((U64)b)<<32)|(((U64)a)<<48);

		return rgba;
	}

	Normal3	ApplyMatrix(const Matrix44& matrix, const Normal3& src)
	{
		Vector3F32 dst;

		dst[0] = ( (matrix.At<0>().X() * src.X()) + (matrix.At<1>().X() * src.Y()) + (matrix.At<2>().X() * src.Z()) );
		dst[1] = ( (matrix.At<0>().Y() * src.X()) + (matrix.At<1>().Y() * src.Y()) + (matrix.At<2>().Y() * src.Z()) );
		dst[2] = ( (matrix.At<0>().Z() * src.X()) + (matrix.At<1>().Z() * src.Y()) + (matrix.At<2>().Z() * src.Z()) );

		return (Normal3&)dst;
	}

	Vector3F32 ApplyMatrix(const Matrix44& matrix, const Vector3F32& src)
	{
		Vector3F32 dst;

		dst[0] = ( (matrix.At<0>().X() * src.X()) + (matrix.At<1>().X() * src.Y()) + (matrix.At<2>().X() * src.Z()) + matrix.At<3>().X() );
		dst[1] = ( (matrix.At<0>().Y() * src.X()) + (matrix.At<1>().Y() * src.Y()) + (matrix.At<2>().Y() * src.Z()) + matrix.At<3>().Y() );
		dst[2] = ( (matrix.At<0>().Z() * src.X()) + (matrix.At<1>().Z() * src.Y()) + (matrix.At<2>().Z() * src.Z()) + matrix.At<3>().Z() );

		return dst;
	}

	Vector4F32 ApplyMatrix(const Matrix44& matrix, const Vector4F32& src)
	{
		return src * matrix;
	}

	Position3 ApplyMatrix(const Matrix44& matrix, const Position3& src)
	{
		Position3 dst;

		dst[0] = ( (matrix.At<0>().X() * src.X()) + (matrix.At<1>().X() * src.Y()) + (matrix.At<2>().X() * src.Z()) + matrix.At<3>().X() );
		dst[1] = ( (matrix.At<0>().Y() * src.X()) + (matrix.At<1>().Y() * src.Y()) + (matrix.At<2>().Y() * src.Z()) + matrix.At<3>().Y() );
		dst[2] = ( (matrix.At<0>().Z() * src.X()) + (matrix.At<1>().Z() * src.Y()) + (matrix.At<2>().Z() * src.Z()) + matrix.At<3>().Z() );

		return dst;
	}

	Normal3 ApplyTransposeOfMatrixInverseToNormal(const Matrix44& matrix, const Normal3& src)
	{
		Normal3 dst;
		Vector3F32 dstvec;

		Matrix44 temp, normalTransform;
		temp = matrix;
		UtMaths::Transpose(&temp);
		UtMaths::Invert4x4(&normalTransform, temp);

		dstvec[0] = ( (normalTransform.At<0>().X() * src.X()) + (normalTransform.At<1>().X() * src.Y()) + (normalTransform.At<2>().X() * src.Z()) );
		dstvec[1] = ( (normalTransform.At<0>().Y() * src.X()) + (normalTransform.At<1>().Y() * src.Y()) + (normalTransform.At<2>().Y() * src.Z()) );
		dstvec[2] = ( (normalTransform.At<0>().Z() * src.X()) + (normalTransform.At<1>().Z() * src.Y()) + (normalTransform.At<2>().Z() * src.Z()) );

		UtMaths::SafeNormalise(&dstvec);

		dst[0] = dstvec[0];
		dst[1] = dstvec[1];
		dst[2] = dstvec[2];

		return dst;
	}

	Normal3 ApplyMatrixToNormal(const Matrix44& matrix, const Normal3& src)
	{
		Vector3F32 dst;

		dst[0] = ( (matrix.At<0>().X() * src.X()) + (matrix.At<1>().X() * src.Y()) + (matrix.At<2>().X() * src.Z()));
		dst[1] = ( (matrix.At<0>().Y() * src.X()) + (matrix.At<1>().Y() * src.Y()) + (matrix.At<2>().Y() * src.Z()));
		dst[2] = ( (matrix.At<0>().Z() * src.X()) + (matrix.At<1>().Z() * src.Y()) + (matrix.At<2>().Z() * src.Z()));

		UtMaths::SafeNormalise(&dst);

		Normal3 temp;
		temp[0] = dst[0];
		temp[1] = dst[1];
		temp[2] = dst[2];

		return temp;
	}

	void BuildMatrixSShRT(
		Maths::HMat43& out,
		const Maths::HVec3& scale,
		const Maths::HVec3& shear,
		const Maths::HVec3& rotate, 
		const Maths::HVec3& translate,
		Aftk::E_RotationOrder order)
	{
		Maths::HMat33 id;
		id.SetIdentity();

		BuildMatrix(
			out,
			Maths::HVec3(0.f,0.f,0.f),	//no scale pivot
			scale,
			shear,
			Maths::HVec3(0.f,0.f,0.f),	//no scale pivot translation
			Maths::HVec3(0.f,0.f,0.f),	//no rotate pivot
			id,							//no rotate orientation matrix
			rotate,
			Maths::HVec3(0.f,0.f,0.f),	//no rotate pivot translation
			translate,
			order);
	}


	void BuildMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scalePivot,
		const Maths::HVec3& scale,
		const Maths::HVec3& shear,
		const Maths::HVec3& scalePivotTranslation,
		const Maths::HVec3& rotatePivot,
		const Maths::HMat33& ro,	//rotate orientation matrix
		const Maths::HMat33& r,		// rotation matrix
		const Maths::HVec3& rotatePivotTranslation,
		const Maths::HVec3& translate)
	{
		//Scale pivot inverse (Sp-1)
		Maths::HMat43 spInv(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			-scalePivot);

		//Scale (S)
		Maths::HMat43 s(Maths::C_MatrixScale(scale.X(), scale.Y(), scale.Z()));

		// Shear (Sh) matrix	[ 1,   YX,  ZX,  0, ]
		//						[ XY,  1,   ZY,  0, ]
		//						[ XZ,  YZ,  1,   0, ]
		//			            [ 0,   0,   0,   1  ]

		//Shear in XY, XZ, YZ planes
		Maths::HMat43 sh(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(shear.X(), 1.f, 0.f),
			Maths::HVec3(shear.Y(), shear.Z(), 1.f),
			Maths::HVec3(0.f, 0.f, 0.f));

		//Scale pivot (Sp)
		Maths::HMat43 sp(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			scalePivot);

		//Scale pivot translation. (St)
		Maths::HMat43 st(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			scalePivotTranslation);

		//Rotate pivot inverse (Rp-1)
		Maths::HMat43 rpInv(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			-rotatePivot);

		//Rotate pivot (Rp)
		Maths::HMat43 rp(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			rotatePivot);

		//Rotate pivot translation (Rt)
		Maths::HMat43 rt(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			rotatePivotTranslation);

		//Scale pivot (t)
		Maths::HMat43 t(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			translate);
		
		//The transformation matrix is constructed as follows (post-multiplication):
		// -1                      -1
		//[Sp]x[S]x[Sh]x[Sp]x[St]x[Rp]x[Ro]x[R]x[Rp]x[Rt]x[T]
		out = spInv * s * sh * sp * st * rpInv * Maths::HMat43(ro) * Maths::HMat43(r) * rp * rt * t;
	}
/* broken in simd mode
	void BuildMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scalePivot,
		const Maths::HVec3& scale,
		const Maths::HVec3& shear,
		const Maths::HVec3& scalePivotTranslation,
		const Maths::HVec3& rotatePivot,
		const Maths::HMat33& ro,	//rotate orientation matrix
		const Maths::HMat33& r,		// rotation matrix
		const Maths::HVec3& rotatePivotTranslation,
		const Maths::HVec3& translate)
	{
		Maths::HVec3 myHVec(0.f, 1.f, 0.f);
		std::cout << myHVec.X() << " " << myHVec.Y() << " " << myHVec.Z() << std::endl;

		Maths::HMat33 x(
			myHVec,
			myHVec,
			myHVec);

		std::cout << x.AZ().X() << " " << x.AZ().Y() << " " << x.AZ().Z() << " " << x.AZ().W() << std::endl;

		//Scale pivot inverse (Sp-1)
		Maths::HMat43 spInv(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			-scalePivot);

		Maths::HMat44 spInv2(
			Maths::HVec4(1.f, 0.f, 0.f, 0.f),
			Maths::HVec4(0.f, 1.f, 0.f, 0.f),
			Maths::HVec4(0.f, 0.f, 1.f, 0.f),
			Maths::HVec4(-(F32)scalePivot.X(), -(F32)scalePivot.Y(), -(F32)scalePivot.Z(), 1.f));

		std::cout << spInv.AZ().X() << " " << spInv.AZ().Y() << " " << spInv.AZ().Z() << " " << spInv.AZ().W() << std::endl;
		std::cout << spInv2.AZ().X() << " " << spInv2.AZ().Y() << " " << spInv2.AZ().Z() << " " << spInv2.AZ().W() << std::endl;

		//Scale (S)
//		Maths::HMat43 s(Maths::C_MatrixScale(scale.X(), scale.Y(), scale.Z()));
		Maths::HMat43 s(
			Maths::HVec3(scale.X(), 0.f, 0.f),
			Maths::HVec3(0.f, scale.Y(), 0.f),
			Maths::HVec3(0.f, 0.f, scale.Z()),
			-scalePivot);

		// Shear (Sh) matrix	[ 1,   YX,  ZX,  0, ]
		//						[ XY,  1,   ZY,  0, ]
		//						[ XZ,  YZ,  1,   0, ]
		//			            [ 0,   0,   0,   1  ]

		std::cout << (F32)scale.X() << std::endl;
		std::cout << (F32)s.AX().X() << std::endl;

		//Shear in XY, XZ, YZ planes
		Maths::HMat43 sh(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(shear.X(), 1.f, 0.f),
			Maths::HVec3(shear.Y(), shear.Z(), 1.f),
			Maths::HVec3(0.f, 0.f, 0.f));

		//Scale pivot (Sp)
		Maths::HMat43 sp(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			scalePivot);

		//Scale pivot translation. (St)
		Maths::HMat43 st(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			scalePivotTranslation);

		//Rotate pivot inverse (Rp-1)
		Maths::HMat43 rpInv(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			-rotatePivot);

		//Rotate pivot (Rp)
		Maths::HMat43 rp(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			rotatePivot);

		//Rotate pivot translation (Rt)
		Maths::HMat43 rt(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			rotatePivotTranslation);

		//Scale pivot (t)
		Maths::HMat43 t(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			translate);
		
		//The transformation matrix is constructed as follows (post-multiplication):
		// -1                      -1
		//[Sp]x[S]x[Sh]x[Sp]x[St]x[Rp]x[Ro]x[R]x[Rp]x[Rt]x[T]
		out = spInv * s * sh * sp * st * rpInv * Maths::HMat43(ro) * Maths::HMat43(r) * rp * rt * t;
	}
*/
	void BuildMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scalePivot,
		const Maths::HVec3& scale,
		const Maths::HVec3& shear,
		const Maths::HVec3& scalePivotTranslation,
		const Maths::HVec3& rotatePivot,
		const Maths::HMat33& rotateOrientation,
		const Maths::HVec3& rotate,
		const Maths::HVec3& rotatePivotTranslation,
		const Maths::HVec3& translate,
		E_RotationOrder rotationOrder)
	{
		//Initialise from rotation
		Maths::HMat33 rotatem;
		Aftk::EulerToMatrix(
			rotatem, 
			rotate,
			rotationOrder);

		BuildMatrix(
			out,
			scalePivot,
			scale,
			shear,
			scalePivotTranslation,
			rotatePivot,
			rotateOrientation,
			rotatem,
			rotatePivotTranslation,
			translate);
	}

	void BuildJointMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scale,
		const Maths::HMat33& ro, //scale orientation
		const Maths::HMat33& r, //rotation matrix
		const Maths::HMat33& jo, //joint orientation
		const Maths::HVec3& inverseScale,
		const Maths::HVec3& translate)
	{
		//Interestingly, in Maya-land joint transforms do not support shearing.
		//XSI doesn't seem to have a 'bone' or 'joint' node type in the same way as Maya.

		//Scale
		Maths::HMat43 s(Maths::C_MatrixScale(scale.X(), scale.Y(), scale.Z()));

		//Inverse scale
		Maths::HMat43 is(Maths::C_MatrixScale(inverseScale.X(), inverseScale.Y(), inverseScale.Z()));

		//Translation.
		Maths::HMat43 t(
			Maths::HVec3(1.f, 0.f, 0.f),
			Maths::HVec3(0.f, 1.f, 0.f),
			Maths::HVec3(0.f, 0.f, 1.f),
			translate);

		//The transformation matrix for a joint node is below (post-multiplication):
		//	matrix = [S] * [RO] * [R] * [JO] * [IS] * [T]
		out = s * Maths::HMat43(ro) * Maths::HMat43(r) * Maths::HMat43(jo) * is * t;
	}

	void BuildJointMatrix(
		Maths::HMat43& out,
		const Maths::HVec3& scale,
		const Maths::HMat33& scaleOrientation,
		const Maths::HVec3& rotate,
		const Maths::HMat33& orientation,
		const Maths::HVec3& inverseScale,
		const Maths::HVec3& translate,
		E_RotationOrder rotationOrder)
	{
		//Initialise from rotation
		Maths::HMat33 rotatem;
		Aftk::EulerToMatrix(
			rotatem, 
			rotate,
			rotationOrder);

		BuildJointMatrix(
			out,
			scale,
			scaleOrientation,
			rotatem,
			orientation,
			inverseScale,
			translate);
	}

	//The bulk of this function is Copyright (c) 2002, Industrial Light & Magic, 
	//a division of Lucas Digital Ltd. LLC. See ImathMatrixAlgo.h from 
	//OpenEXR (http://www.openexr.com/). A caveat of this routine however is that
	//it doesn't cope with -ve scaling for all cases. Therefore we'll need
	//to perform additional operations to correct this - see full matrix decomposition routine.
	bool ExtractAndRemoveScalingAndShear(
		Maths::HMat33& mat, 
		Maths::HVec3& scl, 
		Maths::HVec3& shr)
	{
		//This implementation follows the technique described in the paper by
		//Spencer W. Thomas in the Graphics Gems II article: "Decomposing a 
		//Matrix into Simple Transformations", p. 320.

		F32 maxVal = 0;

		//It's a shame there isn't a way of being able to efficiently loop thru the elements
		//of an HMat matrix. Hence the bloated code below. Oh well, that's another couple of loops
		//the compiler doesn't have to bother unrolling.
		
		//Row 0
		if (Maths::Abs (mat.AX().X()) > maxVal)
			maxVal = Maths::Abs(mat.AX().X());
		if (Maths::Abs (mat.AX().Y()) > maxVal)
			maxVal = Maths::Abs(mat.AX().Y());
		if (Maths::Abs (mat.AX().Z()) > maxVal)
			maxVal = Maths::Abs(mat.AX().Z());

		//Row 1
		if (Maths::Abs (mat.AY().X()) > maxVal)
			maxVal = Maths::Abs(mat.AY().X());
		if (Maths::Abs (mat.AY().Y()) > maxVal)
			maxVal = Maths::Abs(mat.AY().Y());
		if (Maths::Abs (mat.AY().Z()) > maxVal)
			maxVal = Maths::Abs(mat.AY().Z());

		//Row 2
		if (Maths::Abs (mat.AZ().X()) > maxVal)
			maxVal = Maths::Abs(mat.AZ().X());
		if (Maths::Abs (mat.AZ().Y()) > maxVal)
			maxVal = Maths::Abs(mat.AZ().Y());
		if (Maths::Abs (mat.AZ().Z()) > maxVal)
			maxVal = Maths::Abs(mat.AZ().Z());

		//Matrix is zero'd, we cannot decompose this.
		if (maxVal == 0)
			return false;

		//We normalize the 3x3 matrix here.
		//It was noticed that this can improve numerical stability significantly,
		//especially when many of the upper 3x3 matrix's coefficients are very
		//close to zero; we correct for this step at the end by multiplying the 
		//scaling factors by maxVal at the end (shear and rotation are not 
		//affected by the normalization).
		mat.AX(mat.AX() / maxVal);
		mat.AY(mat.AY() / maxVal);
		mat.AZ(mat.AZ() / maxVal);

		// Compute X scale factor.
		{
			F32 length = mat.AX().Length();
			//Avoid div 0
			if (length == 0.f)
				return false;
			scl.X(length);
		}

		// Normalize first row.
		mat.AX(mat.AX() / scl.X());

		// An XY shear factor will shear the X coord. as the Y coord. changes.
		// There are 6 combinations (XY, XZ, YZ, YX, ZX, ZY), although we only
		// extract the first 3 because we can effect the last 3 by shearing in
		// XY, XZ, YZ combined rotations and scales.
		//
		// shear matrix <   1,  YX,  ZX,  0,
		//                 XY,   1,  ZY,  0,
		//                 XZ,  YZ,   1,  0,
		//                  0,   0,   0,  1 >

		// Compute XY shear factor and make 2nd row orthogonal to 1st.
		shr.X(mat.AX() | mat.AY());
		mat.AY(mat.AY() - mat.AX()*shr.X());

		// Now, compute Y scale.
		{
			F32 length = mat.AY().Length();
			//Avoid div 0
			if (length == 0.f)
				return false;
			scl.Y(length);
		}

		// Normalize 2nd row and correct the XY shear factor for Y scaling.
		mat.AY(mat.AY() / scl.Y()); 
		shr.X(shr.X() / scl.Y());

		// Compute XZ and YZ shears, orthogonalize 3rd row.
		shr.Y(mat.AX() | mat.AZ());
		mat.AZ(mat.AZ() - mat.AX() * shr.Y());
		shr.Z(mat.AY() | mat.AZ());
		mat.AZ(mat.AZ() - mat.AY() * shr.Z());

		// Next, get Z scale.
		{
			F32 length = mat.AZ().Length();
			//Avoid div 0
			if (length == 0.f)
				return false;
			scl.Z(length);
		}

		// Normalize 3rd row and correct the XZ and YZ shear factors for Z scaling.
		mat.AZ(mat.AZ() / scl.Z());
		shr.Y(shr.Y() / scl.Z());
		shr.Z(shr.Z() / scl.Z());

		//At this point, the upper 3x3 matrix in mat is orthonormal.
		//Check for a coordinate system flip. If the determinant
		//is less than zero, then negate the matrix and the scaling factors.
		if ((mat.AX() | (mat.AY() ^ mat.AZ())) < 0.f)
		{
			scl = scl * -1.f;
			mat = mat * -1.f;
		}

		// The upper 3x3 matrix in mat is now a rotation matrix.

		//Correct the scaling factors for the normalization step that we 
		//performed above; shear and rotation are not affected by the 
		//normalization.
		scl *= maxVal;
		return true;
	}

	bool GetScale(
		Maths::HVec3& outScale,
		const Maths::HMat43& m4x3)
	{
		Maths::HVec3 shear;
		return ExtractAndRemoveScalingAndShear(
			(Maths::HMat33)m4x3,
			outScale,
			shear);
	}

	bool GetSShRT(
		Maths::HVec3& outScale,
		Maths::HVec3& outShear,
		Maths::HVec3& outRotate, 
		Maths::HVec3& outTranslate,
		const Maths::HMat43& m4x3,
		Aftk::E_RotationOrder order)
	{
		//Create a 3x3 matrix...
		Maths::HMat33 m3x3(m4x3);

		//Attempt to remove scaling and shear.
		if (!ExtractAndRemoveScalingAndShear(
			m3x3,
			outScale,
			outShear))
			return false;
		//Although the magnitude of the shearing and scaling values extracted are correct, their 
		//signs may not be. We'll correct this later on.

		//m3x3 is now a pure rotation matrix. 
		//Now decompose into Euler angles for the given rotation order.
		MatrixToEuler(outRotate, m3x3, order);

		//Now this seems to be the step that everyone misses. The original IL&M code and all the other
		//decomposition code I've seen to date, doesn't work correctly when people start using -ve
		//scaling and shearing values.
		//I reckon I might have come up with a solution though. What we'll do is create a matrix 
		//containing the shearing and rotation proporties we've extracted. If we subtract this matrix
		//from the matrix we originally set out to decompose then what we're left with are the correctly
		//signed scaling values on the matrix diagonal. The fact that the shearing value signs might be 
		//incorrect when constucting the shearRotate matrix doesn't seem to matter. I could do with 
		//actually proving the mathematical correctness of the shearing part of this. However, emperical 
		//results indicate that the method works.
		Maths::HMat43 shearRotate;
		BuildMatrixSShRT(
			shearRotate,			//Destination matrix
			Maths::HVec3(1,1,1),	//No scaling
			outShear,				//Include shear
			outRotate,				//Include rotation
			Maths::HVec3(0,0,0),	//No translation
			order);					//Euler order

		//Okay, perform a full inverse of the shearRotate matrix as it will not be orthogonal if
		//shearing is present. Sumtract this matrix from the matrix we were originally tasked with
		//decomposing.
		Maths::HMat33 sansShearRotate((Maths::HMat33)m4x3 * ((Maths::HMat33)shearRotate).Inverse());

		//Store the signs of the values on the diagonal of the sansShearRotate matrix.
		Maths::HVec3 SignScl(
			Maths::Sign((F32)sansShearRotate.AX().X()),
			Maths::Sign((F32)sansShearRotate.AY().Y()),
			Maths::Sign((F32)sansShearRotate.AZ().Z()));
	
		//Correct signs for scaling
		if (Maths::Sign((F32)outScale.X()) != SignScl.X())
            outScale.X(-outScale.X());
		if (Maths::Sign((F32)outScale.Y()) != SignScl.Y())
            outScale.Y(-outScale.Y());
		if (Maths::Sign((F32)outScale.Z()) != SignScl.Z())
            outScale.Z(-outScale.Z());

		//Correct signs for shearing. Remmember that shearing takes place in the following
		//three planes XY, XZ, YZ.
		if (Maths::Sign((F32)outScale.X()) != Maths::Sign((F32)outScale.Y()))
			outShear.X(-outShear.X());
		if (Maths::Sign((F32)outScale.X()) != Maths::Sign((F32)outScale.Z()))
			outShear.Y(-outShear.Y());
		if (Maths::Sign((F32)outScale.Y()) != Maths::Sign((F32)outScale.Z()))
			outShear.Z(-outShear.Z());

		//Finally we perform the the trivial step of extracting the translation.
		outTranslate = m4x3.AW();

/*
		std::cout << "Rotate " 
			<< Maths::RadToDeg((F32)outRotate.X()) 
			<< " " 
			<< Maths::RadToDeg((F32)outRotate.Y()) 
			<< " " 
			<< Maths::RadToDeg((F32)outRotate.Z()) 
			<< std::endl;

		std::cout << "Scale " 
			<< outScale.X()
			<< " " 
			<< outScale.Y() 
			<< " " 
			<< outScale.Z()
			<< std::endl;

		std::cout << "Shear " 
			<< outShear.X()
			<< " " 
			<< outShear.Y() 
			<< " " 
			<< outShear.Z()
			<< std::endl;

		std::cout << "Translate " 
			<< outTranslate.X()
			<< " " 
			<< outTranslate.Y() 
			<< " " 
			<< outTranslate.Z()
			<< std::endl;
*/

		//We did it!
		return true;
	}
}
