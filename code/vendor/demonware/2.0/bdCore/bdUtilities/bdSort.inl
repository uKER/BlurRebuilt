// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


template <typename T, typename COMPARISON_FUNCTOR>
void BD_CALL bdCombSort(T *const data,
						const bdUInt size,
						COMPARISON_FUNCTOR compare)
{
	if(size > 1)
	{
		bdUInt gap = size;
		for (;;)
		{
			bdNewGap(gap);
			bdBool swapped = false;
			for (bdUInt i = 0; i < size - gap; i++)
			{
				bdUInt j = i + gap;

				T& one = data[i];
				T& two = data[j];
				if(compare(two, one))
				{
					T temp(one);
					one = two;
					two = temp;
					swapped = true;
				}
			}
			if (gap == 1 && !swapped)
			{
				break;
			}
		}
	}
}

template <typename T>
void BD_CALL bdCombSort(T *const data,
						const bdUInt size)
{
	bdCombSort(data, size, bdLessThan<T>());
}

inline
void BD_CALL bdNewGap(bdUInt& gap)
{
	gap = (gap * 10) / 13;
	if (gap == 9 || gap == 10)
	{
		gap = 11;
	}
	if (gap < 1)
	{
		gap = 1;
	}
}
