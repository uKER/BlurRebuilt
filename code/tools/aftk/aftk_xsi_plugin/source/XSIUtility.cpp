#include "XSIA3DExportPCH.h"
#include "XSIUtility.h"

namespace XSITrans
{

bool XsiToAftkMatrix44(
	Aftk::Matrix44& aftkMatrix,
	const XSI::MATH::CMatrix4& xsiMatrix, 
	F32 absNumericRange)
{
	F64 io_d00, io_d01, io_d02, io_d03; 
	F64 io_d10, io_d11, io_d12, io_d13; 
	F64 io_d20, io_d21, io_d22, io_d23;
	F64 io_d30, io_d31, io_d32, io_d33;

	xsiMatrix.Get(	
		io_d00, io_d01, io_d02, io_d03,
		io_d10, io_d11, io_d12, io_d13, 
		io_d20, io_d21, io_d22, io_d23,
		io_d30, io_d31, io_d32, io_d33);

	if (!IsNumberValid((F32)io_d00, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d01, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d02, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d03, absNumericRange))
		return false;

	if (!IsNumberValid((F32)io_d10, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d11, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d12, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d13, absNumericRange))
		return false;

	if (!IsNumberValid((F32)io_d20, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d21, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d22, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d23, absNumericRange))
		return false;

	if (!IsNumberValid((F32)io_d30, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d31, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d32, absNumericRange))
		return false;
	if (!IsNumberValid((F32)io_d33, absNumericRange))
		return false;

	aftkMatrix[0][0] = (F32)io_d00;
	aftkMatrix[0][1] = (F32)io_d01;
	aftkMatrix[0][2] = (F32)io_d02;
	aftkMatrix[0][3] = (F32)io_d03;

	aftkMatrix[1][0] = (F32)io_d10;
	aftkMatrix[1][1] = (F32)io_d11;
	aftkMatrix[1][2] = (F32)io_d12;
	aftkMatrix[1][3] = (F32)io_d13;

	aftkMatrix[2][0] = (F32)io_d20;
	aftkMatrix[2][1] = (F32)io_d21;
	aftkMatrix[2][2] = (F32)io_d22;
	aftkMatrix[2][3] = (F32)io_d23;

	aftkMatrix[3][0] = (F32)io_d30;
	aftkMatrix[3][1] = (F32)io_d31;
	aftkMatrix[3][2] = (F32)io_d32;
	aftkMatrix[3][3] = (F32)io_d33;

	return true;
}

void AftkToXsiMatrix44(
	XSI::MATH::CMatrix4& xsiMatrix,
	Aftk::Matrix44& aftkMatrix)
{
	xsiMatrix.Set(	
		aftkMatrix[0][0], aftkMatrix[0][1], aftkMatrix[0][2], aftkMatrix[0][3],
		aftkMatrix[1][0], aftkMatrix[1][1], aftkMatrix[1][2], aftkMatrix[1][3], 
		aftkMatrix[2][0], aftkMatrix[2][1], aftkMatrix[2][2], aftkMatrix[2][3],
		aftkMatrix[3][0], aftkMatrix[3][1], aftkMatrix[3][2], aftkMatrix[3][3]);
}

}
