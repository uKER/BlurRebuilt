#ifndef __WELD_TREE_H__
#define __WELD_TREE_H__

/*
 * Tree-based Welding algorithm
 *
 * Written by Jan van Valburg (c)2006 Bizarre Creations
 *
 *    int Weld(const Obj *pData, U32 nData, U32 iDimension, float fThreshold, U32 *pIndices)
 * 
 * pData		= Pointer to list of data items to weld
 * nData		= Number of items in the list
 * iDimension	= Dimensionality of the data (e.g. 3 for xyz vector)
 * fThreshold	= Weld threshold (e.g. 0.001 will weld items that are less than 0.001 apart)
 * pIndices		= Pointer to a list of indices, which will be filled out by the Weld function.
 *
 * 
Usage example code:

		// perform weld - returns remapped index array
		numWelds = Weld(&array[0], nData, T.SIZE, threshold, &indices[0]);

		// remap welded data data
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

		result: newData contains new data and remapping contains the index remapping table
 * 
 */

// partitioning code

// sorting function
template <typename Obj, int iDimension>
bool Partition_sort(const Obj &a, const Obj &b)
{
	return a[iDimension] < b[iDimension];
}

// recursively partition a list of points into leaves, such that the distance
// between the points in each leaf is at least 'fDistance' in one dimension
template <typename Obj, typename Context, typename TCallback>
void Partition(Obj *pObj, int nObjects, int nDimensions, float fThreshold, TCallback callback, Context *pContext, int iCurDimension = 0)
{
	// only 1 item left; nothing to weld
	if (nObjects == 1)
		return;

	for (int nFailed=0; nFailed<nDimensions; nFailed++)
	{
		// sort the data by the current dimension
		switch (iCurDimension)
		{
		case 0:	std::sort(pObj, &pObj[nObjects], Partition_sort<Obj, 0>); break;
		case 1:	std::sort(pObj, &pObj[nObjects], Partition_sort<Obj, 1>); break;
		case 2:	std::sort(pObj, &pObj[nObjects], Partition_sort<Obj, 2>); break;
		case 3:	std::sort(pObj, &pObj[nObjects], Partition_sort<Obj, 3>); break;
		default:
			assert(0);
		}

		int iNextDimension = (iCurDimension + 1) % nDimensions;

		// generate a new partition everytime two elements are spaced further apart than 'fDistance'
		int i = 0;
		int nPartitions = 1;
		Ut::Vector<Obj> partition;
		while (1)
		{
			// add this item to our partition
			partition.PushBack(pObj[i]);

			// reached the end of the object array
			if (i>=nObjects-1)
				break;

			// next item is > 'fThreshold' distance away, so recurse into the current partition and create a new one
			float d = pObj[i+1][iCurDimension] - pObj[i][iCurDimension];
			if (d>fThreshold)
			{
				Partition(&partition[0], partition.Size(), nDimensions, fThreshold, callback, pContext, iNextDimension);
				partition.Clear();
				nPartitions++;
			}

			i++;
		}

		// if all data is inside a single partition, try a different dimension
		if (nPartitions == 1)
		{
			iCurDimension = iNextDimension;
			continue;
		}

		// recurse into the final partition and return
		Partition(&partition[0], partition.Size(), nDimensions, fThreshold, callback, pContext, iNextDimension);
		return;
	}

	// no separation in any of the dimensions. run the callback and return
	callback(pObj, nObjects, pContext);
};

// welding code

struct WeldContext
{
	float fThreshold;
	U32 *pIndices;
	U32 iDimension;
	U32 nWelds;
};

template <typename Obj>
struct WeldData
{
    Obj data;
	U32 index;

	float operator[](int pos) const { return data[pos]; }
};

template <typename Obj>
bool operator<(const WeldData<Obj> &a, const WeldData<Obj> &b)
{
	return a[0] < b[0];
}

float NullBuggeredFloat(float f)
{
	int i = *(int*)&f;
	if ((i&0xff000000) == 0xff000000)
		return 0;
	else
		return f;
}


// weld callback
template <typename Obj, typename Context>
void WeldCallback(Obj *pData, int nData, Context *pContext)
{
	float fThreshold = pContext->fThreshold;
	float fSqrThreshold = fThreshold*fThreshold;

	// TODO: could improve performance by sorting on the largest axis
	std::sort(pData, &pData[nData]);

	for (int i=0; i<nData; i++)
	{
		float x = pData[i][0];

		for (int j=i+1; j<nData; j++)
		{
			// test for early-out
			if (pData[j][0] - x > fThreshold)
				continue;

			// test for vector equivilance
			float f = 0;
			for (U32 d=0; d<pContext->iDimension; d++)
			{
				float v = pData[i][d] - pData[j][d];
				v = NullBuggeredFloat(v);
				f += v*v;
			}

			if (f < fSqrThreshold)
			{
				int iI = pData[i].index;
				int iJ = pData[j].index;

				// weld by mapping the item with the highest index to the one with the lowest
//				printf("welding item %d to %d\n", iI, iJ);
//				printf("distance = %5.2f\n", sqrt(f));

				if (iI > iJ)
					pContext->pIndices[iI] = iJ;
				else
					pContext->pIndices[iJ] = iI;
				pContext->nWelds++;
				// skip to the next item
				break;
			}
		}
	}    
}

// weld all points within a given threshold. returns a remapping index list
template <typename Obj>
int Weld(const Obj *pData, U32 nData, U32 iDimension, float fThreshold, U32 *pIndices)
{
	// initialize indices array
	for (U32 i=0; i<nData; i++)
		pIndices[i] = i;

	// initialize data array
	Ut::Vector< WeldData<Obj> > data;
	data.Resize(nData);
	for (U32 i=0; i<nData; i++)
	{
		data[i].index = i;
		data[i].data = pData[i];
	}

	WeldContext context;
	context.fThreshold = fThreshold;
	context.pIndices = pIndices;
	context.iDimension = iDimension;
	context.nWelds = 0;

	Partition(&data[0], nData, iDimension, fThreshold, WeldCallback<WeldData<Obj>, WeldContext>, &context);

	return context.nWelds;
}


#endif
