#ifndef __SERIALISEUTVECTORTOOST_H__
#define __SERIALISEUTVECTORTOOST_H__

namespace Ut
{

template <typename T, typename Archive>
Archive& operator << (Archive& ar, Ut::Vector<T>&vec)
{
	ar << vec.Size();
	for (int i=0; i<vec.Size(); i++)
		ar << vec[i];

	return ar;
}

template <typename T, typename Archive>
Archive& operator >> (Archive& ar, Ut::Vector<T>&vec)
{
	int c;
	ar >> c;
	vec.Resize(c);
	for (int i=0; i<c; i++)
		ar >> vec[i];

	return ar;
}

};

#endif
