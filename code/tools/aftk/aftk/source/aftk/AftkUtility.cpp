#include <stdafx.h>

#include <aftk/AftkUtility.h>
#ifdef PLATFORM_PC
#include <windows.h>
#endif
//#pragma message("Todo: Fix this. Inclusion of old crc, metadata and utilities.")
#include "../../../common/tools_common/fileutilities.h"
#include "../../../common/metadata/MetaData.h"
#include "../../../common/crc/crc32.h"

namespace Aftk
{
	void InsertionSortPolygonVertices(
		Ut::UtVector< Ut::DynamicVector<S32> >& v,
		Ut::UtVector< Ut::DynamicVector<U32> >& sortedIndexArray, 
		U32 low, 
		U32 high)
	{
		for (U32 p = low+1; p <= high; p++)
		{
			S32 tmp = v[p];
			U32 tmp2 = sortedIndexArray[p];
			U32 j;

			for (j = p; j > 0 && tmp < (v[j-1]); j--)
			{
				v[j] = v[j-1];
				sortedIndexArray[j] = sortedIndexArray[j-1];
			}

			v[j] = tmp;
			sortedIndexArray[j] = tmp2;
		}
	}

	U32 LinearInsert(
		Ut::UtVector< Ut::DynamicVector<
		Vector3F32
		> >& v,
		Vector3F32 var, 
		U32 low, 
		U32 high,
		F32 threshold)
	{
		for (U32 i = low; i < high; i++)
		{
			if (EqualityTestFVector(v[i], var, threshold))
			{
				//No need to add another copy of the same data
				return i;
			}

			if (GreaterThanTestVector(v[i], var))
			{
				Ut::UtVector< Ut::DynamicVector<Vector3F32> >::Iterator it = 
					v.Begin();
				it = it + i;
				//Perform insertion.
				v.InsertBefore(var, it);
				return i;
			}
		}

		v.PushBack(var);
		return (U32)v.Size()-1;
	}

	U32 LinearInsert(
		Ut::UtVector< Ut::DynamicVector<
		Normal3
		> >& v,
		Normal3 var, 
		U32 low, 
		U32 high,
		F32 threshold)
	{
		for (U32 i = low; i < high; i++)
		{
			if (EqualityTestFVector(v[i], var, threshold))
			{
				//No need to add another copy of the same data
				return i;
			}

			if (GreaterThanTestVector(v[i], var))
			{
				Ut::UtVector< Ut::DynamicVector<Normal3> >::Iterator it = 
					v.Begin();
				it = it + i;
				//Perform insertion.
				v.InsertBefore(var, it);
				return i;
			}
		}

		v.PushBack(var);
		return (U32)v.Size()-1;
	}

	U32 BinaryInsert(
		Ut::UtVector< Ut::DynamicVector<Vector3F32> >& v,
		Vector3F32 var, 
		U32 low, 
		U32 high,
		F32 threshold)
	{
		static U32 cutOff = 4;
		if (low + cutOff > high)
		{
			return Aftk::LinearInsert(v, var, low, high, threshold);
		}
		else
		{
			U32 middle = (low + high) / 2;
			if (EqualityTestFVector(v[middle], var, threshold))
			{
				//We've already got this value within the array.
				return middle;
			}
	        
			if (LessThanTestVector(v[middle], var))
			{
				//Go left
				return Aftk::BinaryInsert(v, var, low, middle, threshold);
			}
			else
			{
				//Go right
				return Aftk::BinaryInsert(v, var, middle+1, high, threshold);
			}
		}
	}

	U32 BinaryInsert(
		Ut::UtVector< Ut::DynamicVector<Normal3> >& v,
		Normal3 var, 
		U32 low, 
		U32 high,
		F32 threshold)
	{
		static U32 cutOff = 4;
		if (low + cutOff > high)
		{
			return Aftk::LinearInsert(v, var, low, high, threshold);
		}
		else
		{
			U32 middle = (low + high) / 2;
			if (EqualityTestFVector(v[middle], var, threshold))
			{
				//We've already got this value within the array.
				return middle;
			}
	        
			if (LessThanTestVector(v[middle], var))
			{
				//Go left
				return Aftk::BinaryInsert(v, var, low, middle, threshold);
			}
			else
			{
				//Go right
				return Aftk::BinaryInsert(v, var, middle+1, high, threshold);
			}
		}
	}

	std::string GetUniqueName(const std::string& oldName)
	{
		std::string newName;

		//Find the number of contiguous digits at the end of the oldName.
		S32 i;
		for (i = (S32)oldName.length()-1; i >= 0; i--)
		{
			if (!std::isdigit(oldName[i]))
			{
				//We've found the end of the contigious digit block, break out.
				break;
			}
		}

		U32 number;
		std::string newNumberStr;
	    
		if (i == (S32)oldName.length()-1)
		{
			//We didn't find any digits at the end of the name
			newName = oldName;
			number = 1;
			{
				std::stringstream ss;
				ss << number;
				newNumberStr = ss.str();
			}
		}
		else
		{
			std::string digitBlock = oldName.substr(i+1, (S32)oldName.length()-i);
			{
				std::stringstream ss;
				ss << digitBlock;
				ss >> number;
			}
			number++;
			{
				std::stringstream ss;
				ss << number;
				newNumberStr = ss.str();
			}

			//Remove the digit block from the end of the old name during the copy
			newName = oldName.substr(0,oldName.length()-newNumberStr.length());
		}

		//Append our new digit block to the name
		newName += newNumberStr;

		return newName;
	}

	bool CheckCRC(const std::string& fileName, U32 crc, C_Status& status)
	{
		U32 fileCRC;
		CRC32 crc32;
		bool result = crc32.CRCFromFile(fileName.c_str(), fileCRC);
		if (!result)
		{
			status = C_Status(C_Status::E_Status_Failure, "Failed to generate CRC.");
			return true;
		}

		if (fileCRC != crc)
		{
			return false;
		}

		return true;
	}

	U32 CalculateCRC(const std::string& fileName, C_Status& status)
	{
		U32 crc;
		CRC32 crc32;
		bool result = crc32.CRCFromFile(fileName.c_str(), crc);
		if (!result)
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to generate CRC.");
		}
		return crc;
	}
}
