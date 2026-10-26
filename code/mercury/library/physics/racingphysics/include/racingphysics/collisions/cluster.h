#ifndef RP_CLUSTER_H
#define RP_CLUSTER_H
#ifdef _MSC_VER
#pragma once
#endif

namespace RacingPhysics {
namespace KMeans {

// Cluster holds a list of items and a centroid.
template<typename T, typename U>
class Cluster
{
public:
	typedef T DataType;
	typedef U CentroidType;

	Cluster() {}
	explicit Cluster(const DataType& item);
	Cluster(const CentroidType& centroid) : m_centroid(centroid) {}

	CentroidType ComputeCentroid() const;
	const CentroidType& GetCentroid() const {return m_centroid;}

	void Clear() {items.Destroy();}

	Ut::UtVector<Ut::DynamicVector<DataType> > items;

private:
	CentroidType m_centroid;
};

//////////////////////////////////////////

template<typename T, typename U>
Cluster<T, U>::Cluster(const DataType& item)
{
	m_centroid.Include(item.GetCentroid());
	m_centroid.Finish();
}

template<typename T, typename U>
typename Cluster<T, U>::CentroidType Cluster<T, U>::ComputeCentroid() const
{
	CentroidType centroid;

	for (int i = 0; i < items.Size(); i++)
		centroid.Include(items[i].GetCentroid());

	centroid.Finish();

	return centroid;
}

//////////////////////////////////////////

template <typename T>
class Centroid
{
public:
	typedef T VectorType;

	Centroid() 
	{
		m_count = 0;
		Clear(&m_mean);
	}

	void Include(const T& b)
	{
		m_mean += b;
		m_count++;
	}

	void Finish()
	{
		m_mean /= (float) m_count;
	}

	const T& Mean() const {return m_mean;}

private:
	T m_mean;
	int m_count;
};

} }

#endif
