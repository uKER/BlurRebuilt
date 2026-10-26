
#include "hvec\vector.h"
#include "storage.h"

namespace Maths
{
	template <typename VectorType>
	FORCE_INLINE void Convert(UtMaths::MVector<VectorType>& dst, const Maths::C_Vector& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		Set(&dst, src.X(), src.Y(), src.Z());
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(UtMaths::MPoint<VectorType>& dst, const Maths::C_Vertex& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		Set(&dst, src.X(), src.Y(), src.Z());
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(UtMaths::MNormal<VectorType>& dst, const Maths::C_Normal& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		Set(&MakeVector(dst), src.X(), src.Y(), src.Z());
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(UtMaths::MVector<VectorType>& dst, const Maths::HVec3& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		Set(&MakeVector(dst), src.X(), src.Y(), src.Z());
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(UtMaths::MVector<VectorType>& dst, const Maths::HVec4& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 4);
		Set(&MakeVector(dst), src.X(), src.Y(), src.Z(), src.W());
	}

	/////////////////////////

	template <typename VectorType>
	FORCE_INLINE void Convert(Maths::C_Vector& dst, const UtMaths::MVector<VectorType>& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		dst = Maths::C_Vector(float(src.X()), float(src.Y()), float(src.Z()));
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(Maths::C_Vertex& dst, const UtMaths::MPoint<VectorType>& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		dst = Maths::C_Vertex(float(src.X()), float(src.Y()), float(src.Z()));
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(Maths::C_Normal& dst, const UtMaths::MNormal<VectorType>& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		dst = Maths::C_Normal(float(src.X()), float(src.Y()), float(src.Z()));
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(Maths::HVec3& dst, const UtMaths::MVector<VectorType>& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 3);
		dst = Maths::HVec3(float(src.X()), float(src.Y()), float(src.Z()));
	}

	template <typename VectorType>
	FORCE_INLINE void Convert(Maths::HVec4& dst, const UtMaths::MVector<VectorType>& src)
	{
		BIZ_STATIC_ASSERT(VectorType::SIZE == 4);
		dst = Maths::HVec4(float(src.X()), float(src.Y()), float(src.Z()), float(src.W()));
	}

	// to do: specialise for XMVectorWrapper
	// to do: convert to and from storage types
}

/*
void TestConversion()
{
	GmVector gmv;
	GmPoint gmp;
	GmNormal gmn;
	UtMaths::MVector<VectorXYZ<float> > mv3;
	UtMaths::MVector<VectorXYZW<float> > mv4;

	Maths::C_Vector cv;
	Maths::C_Vertex cp;
	Maths::C_Normal cn;

	Maths::HVec3 hv3;
	Maths::HVec4 hv4;

	GmAxis gma;
	Maths::HMat43 hmat43;

	//gmv = cv; // no direct conversion

	Convert(gmv, cv);
	Convert(gmp, cp);
	Convert(gmn, cn);
	Convert(mv3, hv3);
	Convert(mv4, hv4);
	//Convert(mv3, hv4); number of components must match

	Convert(cv, gmv);
	Convert(cp, gmp);
	Convert(cn, gmn);
	Convert(hv3, mv3);
	Convert(hv4, mv4);
	
	gmv = HVecToGm(cv);
	gmp = HVecToGm(cp);
	gmn = HVecToGm(cn);
	gmv = HVecToGm(cn); // normal to vector: okay
	//gmn = HVecToGm(cv); // vector to normal: not okay

	cv = GmToHVec(gmv);
	cp = GmToHVec(gmp);
	cn = GmToHVec(gmn);
	hv3 = GmToHVec(gmv);
	hv4 = GmToHVec(gmv);

	mv3 = HVecToGm(hv3);

	gma = HMatToGm(hmat43);
	hmat43 = GmToHMat(gma);
}
*/
