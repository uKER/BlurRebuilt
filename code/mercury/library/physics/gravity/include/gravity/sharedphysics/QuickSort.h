##ifndef _NEW_QUICKSORT_H_
#define _NEW_QUICKSORT_H_

namespace SharedPhysics
{

#define QS_THRESHOLD		10


template <class Elem>
inline void QuickSort(Elem* elems, int numElements, int (*CompFunc)(Elem* a, Elem* b), int* stack)
{
	//An implementation of quicksort to help sort the data...
	int size = 2;
	stack[0] = numElements - 1;
	stack[1] = 0;
	int l, r, l2, r2, /*mid,*/ diff, a;
	Elem pivot;
	Elem temp;
	bool bChanged;
	while(size)
	{
		l = stack[--size];
		r = stack[--size];
		diff = r - l;
		if(diff < QS_THRESHOLD)
		{
			//Do bubble sort
			do
			{
				bChanged = false;
				for(a = l; a < r; ++a)
				{
					if(CompFunc(&elems[a], &elems[a+1]) > 0)
					{
						temp = elems[a];
						elems[a] = elems[a+1];
						elems[a+1] = temp;
						bChanged = true;
					}
				}
			}
			while(bChanged);
			continue;
		}

		l2 = l;
		r2 = r;
		pivot = elems[l];
		++l;
		do
		{
			while(CompFunc(&elems[l], &pivot) <= 0 && l < r)
				++l;
			while(CompFunc(&elems[r], &pivot) > 0)
				--r;
			if(l < r)
			{
				//Do a swap...
				temp = elems[l];
				elems[l] = elems[r];
				elems[r] = temp;
			}
		}
		while(l < r);
		//both pointing at the same place so it must be the spot to put the pivot
		elems[l2] = elems[r];
		elems[r] = pivot; //pivot put in final position...

		//Push into the stack...
		diff = r2 - r - 1;
		
		stack[size] = r2;
		stack[size+1] = r + 1;
		size += 2 * (diff > 0);

		diff = r - l2 - 1;
		stack[size] = r - 1;
		stack[size+1] = l2;
		size += 2 * (diff > 0);

#ifdef VERIFY_ORDER
		for(int a = l2; a < l; ++a)
		{
			BIZ_ASSERT(CompFunc(&elems[a], &elems[r]) <= 0);
		}
		for(int a = l + 1; a <= r2; ++a)
		{
			BIZ_ASSERT(CompFunc(&elems[a], &elems[r]) > 0);
		}
#endif
	}

}

};


#endif