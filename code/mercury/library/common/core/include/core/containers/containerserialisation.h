#ifndef __CORE_CONTAINER_SERIALISATION_H__
#define __CORE_CONTAINER_SERIALISATION_H__

// serialisation code for Ut::Vector etc.
namespace Ut
{

// read a Ut::UtVector
template<typename Archive, typename T>
void Deserialise(Archive &ar, Ut::UtVector<T> &t)
{
	int n;
	ar >> n;
	t.Resize(n);
	for (int i=0; i<n; i++)
		ar >> t[i];
}

// write a Ut::UtVector
template<typename Archive, typename T>
void Serialise(Archive &ar, Ut::UtVector<T> &t)
{
	int n = t.Size();
	ar << n;
	for (int i=0; i<n; i++)
		ar << t[i];
}

// read a Ut::Vector
template<typename Archive, typename T>
void Deserialise(Archive &ar, Ut::Vector<T> &t)
{
	int n;
	ar >> n;
	t.Resize(n);
	for (int i=0; i<n; i++)
		ar >> t[i];
}

// write a Ut::Vector
template<typename Archive, typename T>
void Serialise(Archive &ar, Ut::Vector<T> &t)
{
	int n = t.Size();
	ar << n;
	for (int i=0; i<n; i++)
		ar << t[i];
}

// read an ArrayVector
template<typename Archive, typename T, int SIZE>
void Deserialise(Archive &ar, Ut::UtVector<Ut::ArrayVector<T, SIZE> > &t)
{
	for (int i=0; i<SIZE; i++)
		ar >> t[i];
}

// write an ArrayVector
template<typename Archive, typename T, int SIZE>
void Serialise(Archive &ar, Ut::UtVector<Ut::ArrayVector<T, SIZE> > &t)
{
	for (int i=0; i<SIZE; i++)
		ar << t[i];
}

}


#endif
