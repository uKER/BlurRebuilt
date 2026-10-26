#ifndef SYS_UTMATHS_SERIALISATION_H
#define SYS_UTMATHS_SERIALISATION_H

namespace Sys
{

template<typename Stream, typename Vector>
Stream& Serialise(Stream& out, MVector<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out << vector[i];

	return out;
}

template<typename Stream, typename Vector>
Stream& Deserialise(Stream& in, MVector<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		in >> vector[i];

	return in;
}

template<typename Stream, typename Vector>
Stream& Serialise(Stream& out, MNormal<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out << vector[i];

	return out;
}

template<typename Stream, typename Vector>
Stream& Deserialise(Stream& in, MNormal<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		in >> vector[i];

	return in;
}

template<typename Stream, typename Vector>
Stream& Serialise(Stream& out, MPoint<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out << vector[i];

	return out;
}

template<typename Stream, typename Vector>
Stream& Deserialise(Stream& in, MPoint<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		in >> vector[i];

	return in;
}

}

#endif