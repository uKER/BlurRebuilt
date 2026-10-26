#ifndef AFTK_SEARCH_H
#define AFTK_SEARCH_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <algorithm>
#ifdef JAN_NEW_WELDING_CODE
#include "tools_common\weld.h"
#endif

#define AFTK_GET_SORTED(sortedIndex) (searchArray[sortedIndexList[sortedIndex].index])

namespace Aftk
{
	template <class T>
	E_Axis GetWidestAxisPosition(
		const Ut::UtVector< Ut::DynamicVector<T> >& dataArray)
	{
		F32 min[3];
		min[0] = dataArray[0].X();
		min[1] = dataArray[0].Y();
		min[2] = dataArray[0].Z();

		F32 max[3];
		max[0] = dataArray[0].X();
		max[1] = dataArray[0].Y();
		max[2] = dataArray[0].Z();

		for (U32 i = 1; i < (U32)dataArray.Size(); i++)
		{
			for (U32 j = 0; j < 3; j++)
			{
				if (dataArray[i][j] < min[j])
				{
					min[j] = dataArray[i][j];
				}
				else if (dataArray[i][j] > max[j])
				{
					max[j] = dataArray[i][j];
				}
			}
		}
		F32 deltaAxis[4];
		deltaAxis[0] = max[0] - min[0];
		deltaAxis[1] = max[1] - min[1];
		deltaAxis[2] = max[2] - min[2];

		//Choose sorting axis, the widest axis.
		E_Axis axis = E_Axis_X;
		if (fabsf(deltaAxis[1]) > fabsf(deltaAxis[axis]))
		{
			axis = E_Axis_Y;
		}
		else if (fabsf(deltaAxis[2]) > fabsf(deltaAxis[axis]))
		{
			axis = E_Axis_Z;
		}

		return axis;
	}


	template <class T>
	S32 LinearFindFirstWeldIndex(
		const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
		const Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> > sortedIndexList,
		U32 sortedCompareIndex,
		F32 threshold,
		U32 axis,
		U32 sortedStartIndex)
	{
		S32 firstIndex = -1;
		//Search backward for the first welding index
		for (S32 i = sortedStartIndex; i >= 0; i--)
		{
			F32 test = AFTK_GET_SORTED(sortedCompareIndex)[axis] - AFTK_GET_SORTED(i)[axis];
			if (fabsf(test) < threshold)
			{
				firstIndex = i;
			}
			else
			{
				return firstIndex;
			}
		}

		return firstIndex;
	}


	template <class T>
	S32 BinaryFindFirstWeldIndex(
		const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
		const Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> >& sortedIndexList,
		U32 sortedCompareIndex,
		F32 threshold,
		U32 axis,
		U32 sortedLowIndex,
		U32 sortedHighIndex)
	{
		S32 firstIndex = -1;

		//Cut-off below which we perform a brute force O(n^2) search.
		static const U32 cutOff = 6;
		if (sortedLowIndex + cutOff > sortedHighIndex)
		{
			//We've only a few entries to search. Find the first point within our threshold.
			//Search backward for the first welding index
			for (U32 i = sortedLowIndex; i <= sortedHighIndex; i++)
			{
				F32 test = AFTK_GET_SORTED(sortedCompareIndex)[axis] - AFTK_GET_SORTED(i)[axis];
				if (fabsf(test) < threshold)
				{
					return i;
				}
			}
		}
		else
		{
			//The middle index is our 'pivot' point.
			U32 middleSorted = (sortedLowIndex + sortedHighIndex) / 2;

			F32 test = AFTK_GET_SORTED(sortedCompareIndex)[axis] - AFTK_GET_SORTED(middleSorted)[axis];
			if (fabsf(test) < threshold)
			{
				//The pivot is within our weld threshold. Go left from the pivot to find
				//the first point within our threshold.
				S32 firstPointWithin =  LinearFindFirstWeldIndex(
					searchArray,
					sortedIndexList,
					sortedCompareIndex,
					threshold,
					axis,
					middleSorted);

				return firstPointWithin;
			}
			else
			{
				//Go left if the value we're looking for lies on the left of the pivot point.
				if (AFTK_GET_SORTED(sortedCompareIndex)[axis] < (AFTK_GET_SORTED(middleSorted))[axis])
				{
					firstIndex = BinaryFindFirstWeldIndex(
						searchArray,
						sortedIndexList,
						sortedCompareIndex, 
						threshold, 
						axis,
						sortedLowIndex, 
						middleSorted-1);
				}
				//Else go right if the value we're looking for lies on the right of the pivot point.
				else
				{
					firstIndex = BinaryFindFirstWeldIndex(
						searchArray, 
						sortedIndexList,
						sortedCompareIndex,
						threshold,
						axis,
						middleSorted+1,
						sortedHighIndex);
				}
			}
		}

		return firstIndex;
	}

#ifdef JAN_NEW_WELDING_CODE
	template <class T>
	U32 WeldComponents(
		const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
		const Matrix44* pTransform,
		F32 threshold,
		Ut::UtVector< Ut::DynamicVector<T> >&	newData,
		Ut::UtVector< Ut::DynamicVector<U32> >&	remapping)
	{
		U32 nData = searchArray.Size();
		if (nData == 0)
		{
			return 0;
		}
		U32 numWelds = 0;

		Ut::UtVector< Ut::DynamicVector<bool> > componentsToSkip;
		Ut::UtVector< Ut::DynamicVector<T> > searchArrayTransformed;

		//Size arrays
		remapping.Resize(nData);
		componentsToSkip.Reserve(nData);
		if (pTransform)
		{
			searchArrayTransformed.Resize(nData);
			//Create transformed version of search array.
			for (U32 i = 0; i < (U32)nData; i++)
				searchArrayTransformed[i] = (T&)ApplyMatrix(*pTransform, (Vector3F32&)searchArray[i]);
		}

		searchArrayTransformed.Reserve(nData);

		Ut::UtVector< Ut::DynamicVector<U32> > indices;
		indices.Resize(nData);

		if (pTransform)
			numWelds = Weld<T>(&searchArrayTransformed[0], nData, T.SIZE, threshold, &indices[0]);
		else
			numWelds = Weld<T>(&searchArray[0], nData, T.SIZE, threshold, &indices[0]);

		// remap data
		U32 iNew = 0;
		for (U32 i=0; i<nData; i++)
		{
			if (indices[i] != i)
			{
				// item was welded. duplicate a previously added item
				remapping[i] = remapping[indices[i]];
			}
			else
			{
				// not welded - copy this item into the new data array
				remapping[i] = newData.Size();
				newData.PushBack(searchArray[indices[i]]);
			}
		}

		return numWelds;
	}
#else
	template <class T>
	U32 WeldComponents(
		const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
		const Matrix44* pTransform,
		F32 threshold,
		C_WeldMemoryManager* pWeldMemoryManager)
	{
		AFTK_ASSERT(pWeldMemoryManager);
		if (searchArray.Size() == 0)
			return 0;
		U32 numWelds = 0;

		Ut::UtVector< Ut::DynamicVector<bool> >* pComponentsToSkip;
		pWeldMemoryManager->GetComponentsToSkipArray(&pComponentsToSkip);
		Ut::UtVector< Ut::DynamicVector<T> >* pSearchArrayTransformed;
		pWeldMemoryManager->GetTransformedArray(&pSearchArrayTransformed);
		Ut::UtVector< Ut::DynamicVector<T> >* pIntermediateData;
		pWeldMemoryManager->GetIntermediateArray(&pIntermediateData);

		Ut::UtVector< Ut::DynamicVector<U32> >* pRemapping;
		pWeldMemoryManager->GetRemappingArray(&pRemapping);
		
		//Size arrays
		pRemapping->Resize(searchArray.Size());
		pComponentsToSkip->Reserve(searchArray.Size());
		if (pTransform)
		{
			pSearchArrayTransformed->Resize(searchArray.Size());
			//Create transformed version of search array.
			for (U32 i = 0; i < (U32)searchArray.Size(); i++)
				(*pSearchArrayTransformed)[i] = 
				(T&)ApplyMatrix(*pTransform, (Vector3F32&)searchArray[i]);
		}

		pSearchArrayTransformed->Reserve(searchArray.Size());

		E_Axis axis;
		axis = GetWidestAxisPosition<T>(searchArray);

		//Initialise arrays.
		Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> >* pSortedIndexList;
		pWeldMemoryManager->GetSortedIndexArray(&pSortedIndexList);
		pSortedIndexList->Resize(searchArray.Size());

		for (U32 i = 0; i < (U32)searchArray.Size(); i++)
		{
			(*pSortedIndexList)[i].axis = searchArray[i][axis];
			(*pSortedIndexList)[i].index = i;
			pComponentsToSkip->PushBack(false);
		}

		C_ComponentAxis* pStart = &(*pSortedIndexList)[0];
		C_ComponentAxis* pEnd = &(*pSortedIndexList)[searchArray.Size()-1];
		pEnd++;
		std::sort(pStart, pEnd);

		for (U32 i = 0 ; i < (U32)searchArray.Size(); i++)
		{
			if ((*pComponentsToSkip)[i])
			{
				continue;
			}

			U32 posIndexI = (*pSortedIndexList)[i].index;
			pIntermediateData->PushBack(searchArray[posIndexI]);
			U32 newDataIndex = pIntermediateData->Size()-1;
			(*pRemapping)[posIndexI] = newDataIndex;

			S32 firstWeldIndex;
			if (!pTransform)
			{
				firstWeldIndex = BinaryFindFirstWeldIndex(
				searchArray,
				*pSortedIndexList,
				i,
				threshold,
				axis,
				0, 
				searchArray.Size()-1);
			}
			//Use transformed search array.
			else
			{
				firstWeldIndex = BinaryFindFirstWeldIndex(
					*pSearchArrayTransformed,
					*pSortedIndexList,
					i,
					threshold,
					axis,
					0, 
					searchArray.Size()-1);
			}

			if (firstWeldIndex == -1)
			{
				//We can't weld this vertex.
				continue;
			}

			for (U32 j = (U32)firstWeldIndex; j < (U32)searchArray.Size(); j++)
			{
				U32 posIndexJ = (*pSortedIndexList)[j].index;

				//Make sure we skip the 'current' vertex position index.
				if (i == j)
				{
					continue;
				}

				if (!pTransform)
				{
					F32 test = searchArray[posIndexI][axis] - searchArray[posIndexJ][axis];
					if (fabsf(test) > threshold)
					{
						if (searchArray[posIndexJ][axis] > searchArray[posIndexI][axis])
						{
							//Break out of inner loop
							break;
						}
					}
				}
				else
				{
					//Use transformed search array
					F32 test = (*pSearchArrayTransformed)[posIndexI][axis] - (*pSearchArrayTransformed)[posIndexJ][axis];
					if (fabsf(test) > threshold)
					{
						if ((*pSearchArrayTransformed)[posIndexJ][axis] > (*pSearchArrayTransformed)[posIndexI][axis])
						{
							//Break out of inner loop
							break;
						}
					}
				}

				//Test for vector equivilance
				bool equal;
				if (!pTransform)
				{
					equal = EqualityTestFVector(
						searchArray[posIndexI],
						searchArray[posIndexJ],
						threshold*threshold);
				}
				else
				{
					equal = EqualityTestFVector(
						(*pSearchArrayTransformed)[posIndexI],
						(*pSearchArrayTransformed)[posIndexJ],
						threshold*threshold);
				}

				if (equal)
				{
					numWelds++;

					(*pComponentsToSkip)[j] = true;
					(*pRemapping)[posIndexJ] = newDataIndex;
				}
			}
		}

		return numWelds;
	}
#endif



	S32 LinearFindFirstPolygonVertexWeldIndex(
		const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
		const Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> >& sortedIndexList,
		U32 sortedCompareIndex,
		F32 threshold,
		U32 sortedStartIndex)
	{
		F32 thresholdSquared = threshold*threshold;
		S32 firstIndex = -1;
		//Search backward for the first welding index
		for (S32 i = sortedStartIndex; i >= 0; i--)
		{
			F32 test = sortedIndexList[sortedCompareIndex].axis - sortedIndexList[i].axis;
			if (fabsf(test) <= threshold)
			{
				firstIndex = i;
			}
			else
			{
				return firstIndex;
			}
		}

		return firstIndex;
	}


	S32 BinaryFindFirstPolygonVertexWeldIndex(
		const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
		const Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> >& sortedIndexList,
		U32 sortedCompareIndex,
		F32 threshold,
		U32 sortedLowIndex,
		U32 sortedHighIndex)
	{
		F32 thresholdSquared = threshold*threshold;
		S32 firstIndex = -1;

		//Cut-off below which we perform a brute force O(n^2) search.
		static U32 cutOff = 1;
		if (sortedLowIndex + cutOff > sortedHighIndex)
		{
			//We've only a few entries to search. Find the first point within our threshold.
			//Search backward for the first welding index
			for (U32 i = sortedLowIndex; i <= sortedHighIndex; i++)
			{
				if (AFTK_GET_SORTED(sortedCompareIndex).EqualityTest(
					AFTK_GET_SORTED(i), thresholdSquared))
				{
					return i;
				}
			}
		}
		else
		{
			//The middle index is our 'pivot' point.
			U32 middleSorted = (sortedLowIndex + sortedHighIndex) / 2;
			C_PolygonVertex target = AFTK_GET_SORTED(sortedCompareIndex);
			C_PolygonVertex pivot = AFTK_GET_SORTED(middleSorted);

			F32 test = sortedIndexList[sortedCompareIndex].axis - sortedIndexList[middleSorted].axis;
			if (fabsf(test) <= threshold)
			{
				//The pivot is within our weld threshold. Go left from the pivot to find
				//the first point within our threshold.
				S32 firstPointWithin =  LinearFindFirstPolygonVertexWeldIndex(
					searchArray,
					sortedIndexList,
					sortedCompareIndex,
					threshold,
					middleSorted);

				return firstPointWithin;
			}
			else
			{
				//Go left if the value we're looking for lies on the left of the pivot point.
				if (sortedIndexList[sortedCompareIndex].axis < sortedIndexList[middleSorted].axis)
				{
					firstIndex = BinaryFindFirstPolygonVertexWeldIndex(
						searchArray,
						sortedIndexList,
						sortedCompareIndex, 
						threshold, 
						sortedLowIndex, 
						middleSorted-1);
				}
				//Else go right if the value we're looking for lies on the right of the pivot point.
				else
				{
					firstIndex = BinaryFindFirstPolygonVertexWeldIndex(
						searchArray, 
						sortedIndexList,
						sortedCompareIndex,
						threshold,
						middleSorted+1,
						sortedHighIndex);
				}
			}
		}

		return firstIndex;
	}

	U32 WeldPolygonVertices(
		const Ut::UtVector< Ut::DynamicVector<Position3> >& positionArray,
		const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
		F32 threshold,
		C_WeldMemoryManager* pWeldMemoryManager)
	{
		
		if (searchArray.Size() == 0)
		{
			return 0;
		}

		F32 thresholdSquared = threshold*threshold;

		U32 numWelds = 0;

		Ut::UtVector<Ut::DynamicVector<C_ComponentAxis> >* pSortedIndexList;
		pWeldMemoryManager->GetSortedIndexArray(&pSortedIndexList);
		pSortedIndexList->Resize(searchArray.Size());

		Ut::UtVector<Ut::DynamicVector<bool> >* pComponentsToSkip;
		pWeldMemoryManager->GetComponentsToSkipArray(&pComponentsToSkip);

		Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >* pIntermediateData;
		pWeldMemoryManager->GetIntermediateArray(&pIntermediateData);

		Ut::UtVector< Ut::DynamicVector<U32> >* pRemapping;
		pWeldMemoryManager->GetRemappingArray(&pRemapping);

		//Size arrays
		pRemapping->Resize(searchArray.Size());
		pComponentsToSkip->Reserve(searchArray.Size());
		pIntermediateData->Reserve(searchArray.Size());


		E_Axis axis;
		axis = GetWidestAxisPosition<Position3>(positionArray);

		//Initialise arrays.
		for (U32 i = 0; i < (U32)searchArray.Size(); i++)
		{
			(*pSortedIndexList)[i].axis = positionArray[searchArray[i].GetPositionIndex()][axis];
			(*pSortedIndexList)[i].index = i;
			pComponentsToSkip->PushBack(false);
		}

		C_ComponentAxis* pStart = &(*pSortedIndexList)[0];
		C_ComponentAxis* pEnd = &(*pSortedIndexList)[searchArray.Size()-1];
		pEnd++;
		std::sort(pStart, pEnd);

		for (U32 i = 0; i < (U32)searchArray.Size(); i++)
		{
			C_PolygonVertex pv = searchArray[(*pSortedIndexList)[i].index];
		}

		for (U32 i = 0 ; i < (U32)searchArray.Size(); i++)
		{
			if ((*pComponentsToSkip)[i])
			{
				continue;
			}

			U32 posIndexI = (*pSortedIndexList)[i].index;
			pIntermediateData->PushBack(searchArray[posIndexI]);
			U32 newDataIndex = pIntermediateData->Size()-1;
			(*pRemapping)[posIndexI] = newDataIndex;

			S32 firstWeldIndex;

			firstWeldIndex = BinaryFindFirstPolygonVertexWeldIndex(
				searchArray,
				*pSortedIndexList,
				i,
				threshold,
				0, 
				searchArray.Size()-1);

			if (firstWeldIndex == -1)
			{
				continue;
			}


			for (U32 j = (U32)firstWeldIndex; j < (U32)searchArray.Size(); j++)
			{
				U32 posIndexJ = (*pSortedIndexList)[j].index;

				//Make sure we skip the 'current' vertex position index.
				if (i == j)
				{
					continue;
				}

				F32 test = (*pSortedIndexList)[j].axis - (*pSortedIndexList)[i].axis;
				if (fabsf(test) > threshold)
				{
					if ((*pSortedIndexList)[j].axis > (*pSortedIndexList)[i].axis)
					{
						//Break out of inner loop
						break;
					}
				}

				if (searchArray[posIndexI].EqualityTest(
					searchArray[posIndexJ], thresholdSquared))
				{
					numWelds++;

					(*pComponentsToSkip)[j] = true;
					(*pRemapping)[posIndexJ] = newDataIndex;
				}
			}
		}

		return numWelds;
	}
}
#endif
