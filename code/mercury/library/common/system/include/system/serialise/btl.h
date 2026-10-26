#ifndef SYS_BTL_CONTAINER_SERIALISATION_H
#define SYS_BTL_CONTAINER_SERIALISATION_H

namespace Sys
{

// read a Btl Vector
template<typename Archive, typename T>
void Deserialise(Archive &ar, Btl::Vector<T> &t)
{
	int n;
	ar >> n;
	t.Resize(n);
	for (int i=0; i<n; i++)
		ar >> t[i];
}

// write a Btl Vector
template<typename Archive, typename T>
void Serialise(Archive &ar, const Btl::Vector<T> &t)
{
	int n = t.Size();
	ar << n;
	for (int i=0; i<n; i++)
		ar << t[i];
}

// read an ArrayVector
template<typename Archive, typename T, int SIZE>
void Deserialise(Archive &ar, Btl::Vector<T, Btl::VectorParameters_Array<SIZE> > &t)
{
	for (int i=0; i<SIZE; i++)
		ar >> t[i];
}

// write an ArrayVector
template<typename Archive, typename T, int SIZE>
void Serialise(Archive &ar, const Btl::Vector<T, Btl::VectorParameters_Array<SIZE> > &t)
{
	for (int i=0; i<SIZE; i++)
		ar << t[i];
}

template <class Archive, class T, class A>
void Deserialise(Archive& ar, Btl::Vector<T, Btl::VectorParameters_FixedSize<A> > &t)
{
	S32 n;
	ar >> n;
	t.InitialiseSize(n);
	for (int i = 0; i < n; i++)
		ar >> t[i];
}

}


#endif
