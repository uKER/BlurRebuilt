#ifndef DMAN_UTIL_H
#define DMAN_UTIL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

template <typename T>
void BubbleSort(Btl::Vector<T>& array, bool asc = true)
{
	int i, j, flag = 1; 
	T temp;
	int arraySize = array.Size(); 
	for(i = 1; (i <= arraySize) && flag; i++)
	{
		flag = 0;
		for (j = 0; j < (arraySize -1); j++)
		{
			if(asc ? array[j+1] < array[j] : array[j+1] > array[j])
			{ 
				temp = array[j];
				array[j] = array[j+1];
				array[j+1] = temp;
				flag = 1;
			}
		}
	}
	return;
}

Xt::DomBlock* FindLocEntry(Xt::DomBlock* pDialogueEntry);

bool IsPathOk(const wxString& path);

Xt::DomRecord* FindRecordByName(Xt::DomPack* pPack, const wxString& recordName);

} //namespace DMan

#endif //DMAN_UTIL_H