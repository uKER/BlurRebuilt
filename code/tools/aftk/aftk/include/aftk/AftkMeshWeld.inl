#define AFTK_MESH_WELDER_GET_SORTED(sortedIndex) (searchArray[sortedIndexList[sortedIndex].index])


template <class T>
E_Axis C_MeshWeld::InternalGetWidestAxisPosition(
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
S32 C_MeshWeld::InternalLinearFindFirstWeldIndex(
	const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
	const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
	U32 sortedCompareIndex,
	F32 threshold,
	U32 axis,
	U32 sortedStartIndex)
{
	S32 firstIndex = -1;
	//Search backward for the first welding index
	for (S32 i = sortedStartIndex; i >= 0; i--)
	{
		F32 test = AFTK_MESH_WELDER_GET_SORTED(sortedCompareIndex)[axis] - AFTK_MESH_WELDER_GET_SORTED(i)[axis];
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
S32 C_MeshWeld::InternalBinaryFindFirstWeldIndex(
	const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
	const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
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
			F32 test = AFTK_MESH_WELDER_GET_SORTED(sortedCompareIndex)[axis] - AFTK_MESH_WELDER_GET_SORTED(i)[axis];
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

		F32 test = AFTK_MESH_WELDER_GET_SORTED(sortedCompareIndex)[axis] - AFTK_MESH_WELDER_GET_SORTED(middleSorted)[axis];
		if (fabsf(test) < threshold)
		{
			//The pivot is within our weld threshold. Go left from the pivot to find
			//the first point within our threshold.
			S32 firstPointWithin =  InternalLinearFindFirstWeldIndex(
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
			if (AFTK_MESH_WELDER_GET_SORTED(sortedCompareIndex)[axis] < (AFTK_MESH_WELDER_GET_SORTED(middleSorted))[axis])
			{
				firstIndex = InternalBinaryFindFirstWeldIndex(
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
				firstIndex = InternalBinaryFindFirstWeldIndex(
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

template <class T>
U32 C_MeshWeld::InternalWeldComponents(
	C_MeshWeldMemoryManager& weldMemoryManager,
	const Ut::UtVector< Ut::DynamicVector<T> >& _searchArray,
	const Matrix44* pTransform,
	F32 threshold,
	I_ComponentFunctor* additionalCheckPtr)
{
	if (_searchArray.Size() == 0)
		return 0;
	
	Ut::UtVector< Ut::DynamicVector<bool> >* pComponentsToSkip;
	weldMemoryManager.GetComponentsToSkipArray(&pComponentsToSkip);
	Ut::UtVector< Ut::DynamicVector<T> >* pSearchArrayTransformed;
	weldMemoryManager.GetTransformedArray(&pSearchArrayTransformed);
	Ut::UtVector< Ut::DynamicVector<T> >* pIntermediateData;
	weldMemoryManager.GetIntermediateArray(&pIntermediateData);
	
	const Ut::UtVector< Ut::DynamicVector<T> >* pInputArray;
	if (pTransform)
	{
		pSearchArrayTransformed->Resize(_searchArray.Size());
		//Create transformed version of search array.
		for (U32 i = 0; i < (U32)_searchArray.Size(); i++)
			(*pSearchArrayTransformed)[i] = 
			(T&)ApplyMatrix(*pTransform, (Vector3F32&)_searchArray[i]);
		pInputArray = pSearchArrayTransformed;
	}
	else
	{
		//Weld on obj space.
		pInputArray = &_searchArray;
	}

	U32 numWelds = 0;
	Ut::UtVector< Ut::DynamicVector<S32> >* pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);
	
	//Size arrays
	pRemapping->Resize((*pInputArray).Size());
	pComponentsToSkip->Reserve((*pInputArray).Size());

	E_Axis axis = InternalGetWidestAxisPosition<T>((*pInputArray));

	//Initialise arrays.
	Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >* pSortedIndexList;
	weldMemoryManager.GetSortedIndexArray(&pSortedIndexList);
	pSortedIndexList->Resize((*pInputArray).Size());

	for (U32 i = 0; i < (U32)(*pInputArray).Size(); i++)
	{
		(*pSortedIndexList)[i].axis = (*pInputArray)[i][axis];
		(*pSortedIndexList)[i].index = i;
		pComponentsToSkip->PushBack(false);
	}

	C_MeshWeldComponentAxis* pStart = &(*pSortedIndexList)[0];
	C_MeshWeldComponentAxis* pEnd = &(*pSortedIndexList)[(*pInputArray).Size()-1];
	pEnd++;
	std::sort(pStart, pEnd);

	for (U32 i = 0 ; i < (U32)(*pInputArray).Size(); i++)
	{
		if ((*pComponentsToSkip)[i])
			continue;

		U32 posIndexI = (*pSortedIndexList)[i].index;
		//Push untransformed position.
		pIntermediateData->PushBack(_searchArray[posIndexI]);
		U32 newDataIndex = pIntermediateData->Size()-1;
		(*pRemapping)[posIndexI] = newDataIndex;

		S32	firstWeldIndex = InternalBinaryFindFirstWeldIndex(
			(*pInputArray),
			*pSortedIndexList,
			i,
			threshold,
			axis,
			0, 
			(*pInputArray).Size()-1);
	
		if (firstWeldIndex == -1)
			//We can't weld this vertex.
			continue;

		for (U32 j = (U32)firstWeldIndex; j < (U32)(*pInputArray).Size(); j++)
		{
			U32 posIndexJ = (*pSortedIndexList)[j].index;

			//Make sure we skip the 'current' vertex position index.
			if (i == j)
				continue;

			F32 test = (*pInputArray)[posIndexI][axis] - (*pInputArray)[posIndexJ][axis];
			if (fabsf(test) > threshold)
				if ((*pInputArray)[posIndexJ][axis] > (*pInputArray)[posIndexI][axis])
					//Break out of inner loop
					break;

			//Test for vector equivilance
			bool equal = EqualityTestFVector(
				(*pInputArray)[posIndexI],
				(*pInputArray)[posIndexJ],
				threshold*threshold);

			if (additionalCheckPtr)
				//further check...
				equal = equal && (*additionalCheckPtr)(posIndexI, posIndexJ);

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
