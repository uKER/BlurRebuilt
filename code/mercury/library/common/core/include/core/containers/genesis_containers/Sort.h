#ifndef GENESIS_SORT_H
#define GENESIS_SORT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Util
{

template<class A, class T> void QuickSortArray(A& array, const S32 first, const S32 last )
{
	int i,j;
	T   x,t;
	
	x = array[(first+last)/2];
	i = first; j= last;
	for(;;)
	{
		while(!(array[i] > x))
		{
			i++;
		}
		while(!(x > array[j]))
		{
			j--;
		}
		if(i>=j)
		{
			break;
		}
		t = array[i];
		array[i] = array[j];
		array[j] = t;
		i++;
		j--;
	}
	if(first < (i -1 ))
	{
		QuickSortArray<A,T>(array, first, i-1);
	}
	if((j+1) < last)
	{
		QuickSortArray<A,T>(array, j+1, last);
	}
}

template<class A, class T> void QuickSortArray(A& array, const S32 noItems )
{
    QuickSortArray<A,T>(array, 0, noItems );
}

template<class A, class T> void QuickSortArray(A& array)
{
    QuickSortArray<A,T>(array, 0, array.Elements());
}

// array sorting
// array needs [] operator
// array item needs > operator

template<class A, class T> void ShellSortArray(A& array, const S32 noItems)
{
    S32 i, j, increment;

    increment = 3;
    while (increment > 0)
    {
        for (i=0; i < noItems; i++)
        {
            j = i;
            T temp = array[i];
            while ((j >= increment) && (array[j-increment] > temp))
            {
                array[j] = array[j - increment];
                j = j - increment;
            }
            array[j] = temp;
        }

        if (increment/2 != 0)
            increment = increment/2;
        else if (increment == 1)
            increment = 0;
        else
            increment = 1;
    }
}

template<class A, class T> void ShellSortArray(A& array)
{
    ShellSortArray<A,T>(array, array.Elements());
}

}; // namespace Util


#endif
