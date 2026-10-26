#ifndef DGUTIL_HELPERS_H
#define DGUTIL_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Maths {
template<typename T>
void	SetMax(T& dst, const Ut::Vector<T>& src) {
	int	i;
	for (i=0; i<src.Size(); i++) { Maths::SetMax(dst, src[i]); }
}
template<typename T>
void	SetMin(T& dst, const Ut::Vector<T>& src) {
	int	i;
	for (i=0; i<src.Size(); i++) { Maths::SetMin(dst, src[i]); }
}

template<typename T>
void	SetMinMax(T& lo,T& hi, const Ut::Vector<T>& src) {
	int	i;
	for (i=0; i<src.Size(); i++) { Maths::SetMinMax(lo,hi, src[i]); }
}


template<typename T>
void	SetMax(Ut::Vector<T>& dst, const T& src) {
	int	i;
	for (i=0; i<dst.Size(); i++) { Maths::SetMax(dst[i], src); }
}
template<typename T>
void	SetMin(Ut::Vector<T>&	dst, const T& src) {
	int	i;
	for (i=0; i<dst.Size(); i++) { Maths::SetMin(dst[i], src); }
}
} // namespace Maths;


namespace DGUtil 
{

template<typename T>
int Find(const Ut::Vector<T>& src,const T& item) 
{
	for (int i=0; i<src.Size(); i++) 
	{
		if (src[i]==item)
			return	i;
	}
	return	-1;
};

template<typename T>
int	AddUnique(Ut::Vector<T>& src, const T& item)
{
	int	ret=Find(src,item);
	if (ret>=0)
		return	ret;
	src.PushBack(item);
	return	src.Size()-1;
};

template<typename T>
void	AddUnique(Ut::Vector<T>& dst, const Ut::Vector<T>& src)
{
	int	i;
	for (i=0; i<src.Size(); i++)
		AddUnique(dst, src[i]);
};


// Are any items in src1 in src2?
template<typename T>
bool	AnyOfFirstInSecond(const Ut::Vector<T>& src1, const Ut::Vector<T>& src2) 
{
	// no assumption about sort.
	// todo - opt...
	for (int i=0; i<src1.Size(); i++) 
	{
		if (Find(src2, src1[i])>=0)
			return	true;
	}
	return	false;
}

template<typename T>
bool	AnyOverlap(const Ut::Vector<T>& src1, const Ut::Vector<T>& src2) 
{
	// todo - opt...
	if (AnyOfFirstInSecond(src1,src2))
		return	true;
	return	AnyOfFirstInSecond(src2,src1);
}





}; // namespace DGUtil

#endif
