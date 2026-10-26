#ifndef NEW_AXIS_BOX_H
#define NEW_AXIS_BOX_H

namespace Collision
{
	class C_AxisBoxMinMax;

	//using namespace Maths;
	class C_CRAxisBox
	{
	public:
		Maths::HVec3 centre, extents;

		C_CRAxisBox() : centre(Maths::UNDEFINED), extents(Maths::UNDEFINED)
		{
		}

		NOALIAS __forceinline void SetCentre(const Maths::HVec3 PCREF c)
		{
			centre = c;
		}

		NOALIAS __forceinline void SetExtents(const Maths::HVec3 PCREF e)
		{
			extents = e;
		}

		NOALIAS __forceinline void SetCentre(Maths::HF32& x, Maths::HF32& y, Maths::HF32& z)
		{
			centre.X(x);
			centre.Y(y);
			centre.Z(z);
		}

		NOALIAS __forceinline void SetExtents(Maths::HF32& x, Maths::HF32& y, Maths::HF32& z)
		{
			extents.X(x);
			extents.Y(y);
			extents.Z(z);
		}

		NOALIAS __forceinline bool Inside(const C_CRAxisBox& outerBox) const __restrict 
		{
			using namespace Maths;
			const HVec3 ab(centre - outerBox.centre);
			const HVec3 abc(ab - extents);
			const HVec3 abd(ab + extents);
			return (abc.All_GE(-outerBox.extents) & abd.All_LE(outerBox.extents)) != 0;
		}

		NOALIAS __forceinline bool Inside(const Maths::HVec3 PCREF minExtent, const Maths::HVec3 PCREF maxExtent) const __restrict 
		{
			using namespace Maths;
			const Maths::HVec3 _min(centre - extents);
			const Maths::HVec3 _max(centre + extents);

			return minExtent.All_LE(_min) && maxExtent.All_GE(_max);
		}

		//NB - this will return true if the box is smaller than 2x as big as this!
		NOALIAS FORCE_INLINE bool IsSmaller(const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF max) const
		{
			//const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			//const Maths::HVec3 halfExtents((max - min) * Maths::Constants<Maths::HF32>::Quarter());
			//Work out the diagonal length...
			const Maths::HF32 four(Maths::Constants<Maths::HF32>::Four());
			const Maths::HVec3 minMax(max - min);

			const Maths::HVec3 fourExtents(extents * four);
			const Maths::HF32 radius((fourExtents | fourExtents));
			const Maths::HVec3 sqExtents(minMax * minMax);
			const Maths::HF32 ext(Maths::Min(sqExtents.X(), Maths::Min(sqExtents.Y(), sqExtents.Z())));
			return radius < ext;
		}

		NOALIAS __forceinline bool Intersects(const C_CRAxisBox& other) const __restrict
		{
			const Maths::HVec3 absDif(Maths::Abs(centre - other.centre));
			const Maths::HVec3 extSum((Maths::HVec3)(extents + other.extents));
			return absDif.All_LE(extSum) != 0;
		}

		NOALIAS __forceinline bool IntersectsInAxisBox(const C_CRAxisBox& other,
		const C_AxisBoxMinMax& enclosingBox) const __restrict;

		NOALIAS bool IntersectsSphere(const Maths::HVec3 PCREF c, const Maths::HF32 PCREF radius) const
		{
			const Maths::HVec3 transCentre(c - centre);
			const Maths::HF32 radSq(radius * radius);
			const Maths::HVec3 closest(Maths::Max(-extents, Maths::Min(transCentre, extents)));
			const Maths::HVec3 dif(closest - transCentre);
			const Maths::HF32 dis(dif | dif);
			return dis.LE(radSq);
		}

		NOALIAS bool IntersectsSphereInBox(const Maths::HVec3 PCREF c, const Maths::HF32 PCREF radius,
			const C_AxisBoxMinMax& enclosingBox) const;

		NOALIAS __forceinline Maths::HVec3 ClosestPoint(const Maths::HVec3 PCREF point) const
		{
			const Maths::HVec3 localPoint(point - centre);
			return centre + Maths::Min(Maths::Max(localPoint, -extents), extents);
		}

		NOALIAS __forceinline Maths::HF32 Volume() const
		{
			const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
			return extents.X() * extents.Y() * extents.Z() * two * two * two;
		}

		NOALIAS __forceinline Maths::HF32 IntersectionVolume(const Maths::HVec3 PCREF minV, const Maths::HVec3 PCREF maxV) const
		{
			const Maths::HVec3 vZero(Maths::Constants<Maths::HVec3>::Zero());
			const Maths::HVec3 _max(centre + extents);
			const Maths::HVec3 _min(centre - extents);
			const Maths::HVec3 maxOverlap(Maths::Min(_max, maxV));
			const Maths::HVec3 minOverlap(Maths::Max(_min, minV));
			const Maths::HVec3 overlap(Maths::Max(maxOverlap - minOverlap, vZero));
			const Maths::HF32 volume(overlap.X() * overlap.Y() * overlap.Z());
			return volume;
		}

		NOALIAS __forceinline void Transform(const Maths::HMat43& bodyMatrix, C_CRAxisBox& result) const
		{
		    using namespace Maths;
		    
		    #if 0
			HVec3 rx, ry, rz;
			rx = bodyMatrix.AX();
			ry = bodyMatrix.AY();
			rz = bodyMatrix.AZ();
			HF32 vx = extents.X(), vy = extents.Y(), vz = extents.Z();
			//x = (rx.X() * x).Abs() + (ry.X() * x).Abs() + (rz.X() * x).Abs();
			//y = (rx.Y() * y).Abs() + (ry.Y() * y).Abs() + (rz.Y() * y).Abs();
			//z = (rx.Z() * z).Abs() + (ry.Z() * z).Abs() + (rz.Z() * z).Abs();
            const HF32 x( Sum( Abs( extents.m_V * HVec3( rx.X(), ry.X(), rz.X() ) ) ) );
            const HF32 y( Sum( Abs( extents.m_V * HVec3( rx.Y(), ry.Y(), rz.Y() ) ) ) );
            const HF32 z( Sum( Abs( extents.m_V * HVec3( rx.Z(), ry.Z(), rz.Z() ) ) ) );
			result.extents.m_V.X(x);
			result.extents.m_V.Y(y);
			result.extents.m_V.Z(z);
			#else
			result.extents = AbsTransform( extents, bodyMatrix.Rotation() );
			#endif
            
			//Transform the centre point of the box. If the box should be rotated about its
			//centre, the centre position of the box should be (0,0,0) and its position in world space
			//should be represented in the bodyMatrix
			result.centre = centre * bodyMatrix; 
		}

		NOALIAS __forceinline void Transform(Maths::HMat43& parentBodyMatrix, Maths::HMat43& localBodyMatrix, C_CRAxisBox& result) const
		{
			result.centre = centre * parentBodyMatrix;
			Maths::HMat33 rot = parentBodyMatrix.Rotation() * localBodyMatrix.Rotation(); 
			//Get the axes...
			/*Maths::HVec3 rx, ry, rz;
			rot.GetAxes(rx, ry, rz);
			Maths::HF32 x = extents.X(), y = extents.Y(), z = extents.Z();
			x = (rx.X() * x).Abs() + (rx.Y() * x).Abs() + (rx.Z() * x).Abs();
			y = (ry.X() * y).Abs() + (ry.Y() * y).Abs() + (ry.Z() * y).Abs();
			z = (rz.X() * z).Abs() + (rz.Y() * z).Abs() + (rz.Z() * z).Abs();
			result.extents.X(x);
			result.extents.Y(y);
			result.extents.Z(z);*/
			result.extents = AbsTransform( extents, rot );
		}

		void MergeBox(const C_CRAxisBox& otherBox)
		{
			//Don't shift centre - instead, just enlarge extents. It is up to the artists to keep the centre of a composite shape roughly around the origin...
			const Maths::HVec3 maxO(otherBox.centre + otherBox.extents);
			const Maths::HVec3 minO(otherBox.centre - otherBox.extents);

			extents = Maths::Max(extents, Maths::Max(Maths::Abs(maxO), Maths::Abs(minO)));
		}
	};

	class C_AxisBoxMinMax
	{
	public:
		Maths::HVec3 m_min, m_max;

		C_AxisBoxMinMax() : m_min(Maths::UNDEFINED), m_max(Maths::UNDEFINED)
		{
		}

		void SetInvalidExtents()
		{
			using namespace Maths;
			m_min = Constants<HVec3>::Max();
			m_max = -Constants<HVec3>::Max();
		}

		NOALIAS __forceinline bool Inside(const C_AxisBoxMinMax& outerBox) const __restrict 
		{
			return (m_min.All_GE(outerBox.m_min) & m_max.All_LE(outerBox.m_max)) != 0;
		}

		NOALIAS __forceinline bool Intersects(const C_AxisBoxMinMax& other) const __restrict
		{
			return (m_max.Any_LT(other.m_min) || other.m_max.Any_LT(m_min)) == 0;
		}

		NOALIAS __forceinline bool Contains(const C_CRAxisBox& innerBox) const __restrict;

		NOALIAS __forceinline bool Intersects(const C_CRAxisBox& other) const __restrict;

		NOALIAS bool IntersectSphere(const Maths::HVec3 PCREF c, const Maths::HF32 PCREF radius) const;

		Maths::Scalar_Bool Contains(const Maths::HVec3 PCREF point) const __restrict;

		NOALIAS FORCE_INLINE Maths::HF32 Volume() const __restrict
		{
			const Maths::HVec3 dimensions(m_max - m_min);
			return dimensions.X() * dimensions.Y() * dimensions.Z();
		}

		template <typename Serial>
		void Serialise(Serial& serial) const
		{
			serial << m_min << m_max;
		}

		template <typename Serial>
		void Deserialise(Serial& serial)
		{
			serial >> m_min >> m_max;
		}
	};

	class C_QuantizedAxisBoxMinMax
	{
		MathsStorage::Fixed3_S16<14> m_min, m_max;

	public:
		C_QuantizedAxisBoxMinMax()
		{
		}

		C_QuantizedAxisBoxMinMax(const C_AxisBoxMinMax& box, const Maths::HVec3 PCREF low, const Maths::HVec3 PCREF high)
		{
			const Maths::HVec3 range((high - low).Reciprocal());
			const Maths::HVec3 vmin((box.m_min - low) * range);
			const Maths::HVec3 vmax((box.m_max - low) * range);
			m_min = (MathsStorage::Fixed3_S16<14>)vmin;
			m_max = (MathsStorage::Fixed3_S16<14>)vmax;
			m_max.x = Maths::Max<S16>(m_max.x, m_max.x + 1);
			m_max.y = Maths::Max<S16>(m_max.y, m_max.y + 1);
			m_max.z = Maths::Max<S16>(m_max.z, m_max.z + 1);
			m_min.x = Maths::Min<S16>(m_min.x, m_min.x - 1);
			m_min.y = Maths::Min<S16>(m_min.y, m_min.y - 1);
			m_min.z = Maths::Min<S16>(m_min.z, m_min.z - 1);


#ifndef CFG_SPEED
			C_AxisBoxMinMax temp;
			LoadBox(temp, low, high);
#endif

			/*m_min.x = Maths::Min<S16>(m_min.x, m_min.x - 1);
			m_min.y = Maths::Min<S16>(m_min.y, m_min.y - 1);
			m_min.z = Maths::Min<S16>(m_min.z, m_min.z - 1);

			m_max.x = Maths::Max<S16>(m_max.x, m_max.x + 1);
			m_max.y = Maths::Max<S16>(m_max.y, m_max.y + 1);
			m_max.z = Maths::Max<S16>(m_max.z, m_max.z + 1);*/

		}

		__forceinline void LoadBox(C_AxisBoxMinMax& box, const Maths::HVec3 PCREF low, const Maths::HVec3 PCREF high)
		{
			const Maths::HVec3 range(high - low);
			const Maths::HVec3 localMin(&m_min);
			const Maths::HVec3 localMax(&m_max);
			const Maths::HVec3 vmin((localMin * range) + low);
			const Maths::HVec3 vmax((localMax * range) + low);
			box.m_min = vmin;
			box.m_max = vmax;
		}

		__forceinline void LoadBox(C_AxisBoxMinMax& box, const Maths::HVec3 PCREF low, const Maths::HVec3 PCREF high, const Maths::HF32 PCREF inflation)
		{
			const Maths::HVec3 growth(inflation);
			const Maths::HVec3 range(high - low);
			const Maths::HVec3 localMin(&m_min);
			const Maths::HVec3 localMax(&m_max);
			const Maths::HVec3 vmin((localMin * range) + low);
			const Maths::HVec3 vmax((localMax * range) + low);
			box.m_min = vmin - growth;
			box.m_max = vmax + growth;
		}

		__forceinline void LoadBox(C_AxisBoxMinMax& box, const Maths::HVec3 PCREF low, const Maths::HVec3 PCREF high, const Maths::HVec3 PCREF inflation)
		{
			const Maths::HVec3 range(high - low);
			const Maths::HVec3 localMin(&m_min);
			const Maths::HVec3 localMax(&m_max);
			const Maths::HVec3 vmin((localMin * range) + low);
			const Maths::HVec3 vmax((localMax * range) + low);
			box.m_min = vmin - inflation;
			box.m_max = vmax + inflation;
		}

		template <typename Serial>
		void Serialise(Serial& serial) const
		{
			serial << m_min.x << m_min.y << m_min.z << m_max.x << m_max.y << m_max.z;
		}

		template <typename Serial>
		void DeSerialise(Serial& serial)
		{
			serial >> m_min.x >> m_min.y >> m_min.z >> m_max.x >> m_max.y >> m_max.z;
		}
	};

};


#endif
