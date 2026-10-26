#ifndef MAYA_IO_FILE_UTILITY_H
#define MAYA_IO_FILE_UTILITY_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <float.h>
namespace MayaIo
{
	class C_Importer;

	void SetProgressMax(U32 max);

	F32 GetProgressPercent();

	U32 GetProgress();

	void SetProgress(U32 progress);

	void AdvanceProgress();

	MString	comment(const MString& text);

	MString	quote(const MString& text);

	bool MayaToAftkMatrix44(
		Aftk::Matrix44& aftkMatrix,
		const MMatrix& mayaMatrix, 
		F32 absNumericRange);

	void AftkToMayaMatrix44(
		Aftk::Matrix44& aftkMatrix,
		const MMatrix& mayaMatrix);

	std::string GetFirstToken(const std::string &input);

	bool StripNodeNameFromPlugName(
		const std::string& inputName, 
		std::string& outputName);

	bool StripNodeNameAndIndexFromPlugName(
		const std::string& inputName, 
		std::string& outputName);

	bool BuildCorrectChildName(
		const std::string& parentName, 
		const std::string& inputName, 
		std::string& outputName);

	inline bool IsNumberValid(F32 number, F32 absNumericRange)
	{
		if (_finite(number) == 0)
			return false;
		if (fabsf(number) > absNumericRange)
			return false;
		return true;
	}

	inline bool IsNumberValid(F32 number)
	{
		if (_finite(number) == 0)
			return false;
		return true;
	}

	Aftk::C_Status ConnectPlugs(
		MFnDependencyNode sourceNode, 
		MString sourcePlugName, 
		MFnDependencyNode destinationNode,
		MString destinationPlugName);

	inline bool FileExists(const char* fileName)
	{
		// Let's see if we can open the output file.
		int test = _access(fileName, 0);
		if (test == 0)
			return true;
		else
			return false;
	}

	Aftk::C_Status MayaPlugToAftkDynamicPlug(
		C_Importer* pImporter,
		const MPlug& mayaPlug,
		Aftk::C_DynamicPlug& aftkPlug);

	Aftk::C_Status MayaPlugToAftkGenericAttribute(
		const MPlug& mayaPlug,
		Aftk::C_GenericAttribute& aftkGenericAttribute);

	bool GetOptionValueString(const char* pStr, std::string &value);

	bool GetOptionValueBool(const char* pStr, bool &value);

	bool GetOptionValueF32(const char* pStr, F32 &value);

	class HashKeyOpSetU32
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const Ut::UtSet<U32>& key)
		{
			unsigned int hash = 0;
			for (Ut::UtSet<U32>::ConstIterator it = key.Begin(); it != key.End(); ++it)
				hash += it->GetKey();

			return hash;
		}

		static bool Equal(const Ut::UtSet<U32>& key1, const Ut::UtSet<U32>& key2)
		{
			return key1 == key2;
		}
	};

	inline Aftk::E_RotationOrder GetAftkRotationOrderMEulerRotation(
		MEulerRotation::RotationOrder mRotationOrder)
	{
		switch(mRotationOrder)
		{
		case MEulerRotation::kXYZ:
			return Aftk::E_RotationOrder_XYZ;

		case MEulerRotation::kYZX:
			return Aftk::E_RotationOrder_YZX;

		case MEulerRotation::kZXY:
			return Aftk::E_RotationOrder_ZXY;

		case MEulerRotation::kXZY:
			return Aftk::E_RotationOrder_XZY;

		case MEulerRotation::kYXZ:
			return Aftk::E_RotationOrder_YXZ;

		case MEulerRotation::kZYX:
			return Aftk::E_RotationOrder_ZYX;
		};

		return Aftk::E_RotationOrder_XYZ;
	}

	inline Aftk::E_RotationOrder GetAftkRotationOrderMTransformationMatrix(
		MTransformationMatrix::RotationOrder mRotationOrder)
	{
		switch(mRotationOrder)
		{
		case MTransformationMatrix::kXYZ:
			return Aftk::E_RotationOrder_XYZ;

		case MTransformationMatrix::kYZX:
			return Aftk::E_RotationOrder_YZX;

		case MTransformationMatrix::kZXY:
			return Aftk::E_RotationOrder_ZXY;

		case MTransformationMatrix::kXZY:
			return Aftk::E_RotationOrder_XZY;

		case MTransformationMatrix::kYXZ:
			return Aftk::E_RotationOrder_YXZ;

		case MTransformationMatrix::kZYX:
			return Aftk::E_RotationOrder_ZYX;
		};

		return Aftk::E_RotationOrder_XYZ;
	}
}	

#endif
