
namespace Maths
{
	////////////////////////////////////////////////////////////////////////

	template<> struct Constants<C_Vector>
	{
		__forceinline static const C_Vector Zero() {return C_Vector(Constants<HVec3>::Zero());}
		__forceinline static const C_Vector Null() {return C_Vector(Constants<HVec3>::Zero());}
		__forceinline static const C_Vector X() {return C_Vector(Constants<HVec3>::UnitAxisPositiveX());}
		__forceinline static const C_Vector Y() {return C_Vector(Constants<HVec3>::UnitAxisPositiveY());}
		__forceinline static const C_Vector Z() {return C_Vector(Constants<HVec3>::UnitAxisPositiveZ());}
		__forceinline static const C_Vector NegativeX() {return C_Vector(Constants<HVec3>::UnitAxisNegativeX());}
		__forceinline static const C_Vector NegativeY() {return C_Vector(Constants<HVec3>::UnitAxisNegativeY());}
		__forceinline static const C_Vector NegativeZ() {return C_Vector(Constants<HVec3>::UnitAxisNegativeZ());}
		__forceinline static const C_Vector XY() {return C_Vector(Constants<HVec3>::UnitAxisPositiveX() + Constants<HVec3>::UnitAxisPositiveY());}
		__forceinline static const C_Vector XZ() {return C_Vector(Constants<HVec3>::UnitAxisPositiveX() + Constants<HVec3>::UnitAxisPositiveZ());}
		__forceinline static const C_Vector YZ() {return C_Vector(Constants<HVec3>::UnitAxisPositiveY() + Constants<HVec3>::UnitAxisPositiveZ());}
		__forceinline static const C_Vector XYZ() {return C_Vector(Constants<HVec3>::UnitAxisPositiveX() + Constants<HVec3>::UnitAxisPositiveY() + Constants<HVec3>::UnitAxisPositiveZ());}
	};

	template <> struct Constants<C_Normal>
	{
		__forceinline static const C_Normal X() {return C_Normal(Constants<HVec3>::UnitAxisPositiveX());}
		__forceinline static const C_Normal Y() {return C_Normal(Constants<HVec3>::UnitAxisPositiveY());}
		__forceinline static const C_Normal Z() {return C_Normal(Constants<HVec3>::UnitAxisPositiveZ());}
		__forceinline static const C_Normal NegativeX() {return C_Normal(Constants<HVec3>::UnitAxisNegativeX());}
		__forceinline static const C_Normal NegativeY() {return C_Normal(Constants<HVec3>::UnitAxisNegativeY());}
		__forceinline static const C_Normal NegativeZ() {return C_Normal(Constants<HVec3>::UnitAxisNegativeZ());}
	};

	template <> struct Constants<C_Vertex>
	{
		__forceinline static const C_Vertex Origin() {return C_Vertex(Constants<HVec3>::Zero());}
		__forceinline static const C_Vertex Min() {return C_Vertex(Constants<HVec3>::Zero());}
		__forceinline static const C_Vertex Max() {return C_Vertex(Constants<HVec3>::Zero());}
		__forceinline static const C_Vertex NegativeMax() {return C_Vertex(Constants<HVec3>::Zero());}
	};

	/*template <> struct Constants<Scalar_Bool>
	{
		__forceinline static const Scalar_Bool True() {return Scalar_Bool(1);}
		__forceinline static const Scalar_Bool False() {return Scalar_Bool(0);}
	};*/

	template <> struct Constants<Vector3_Bool>
	{
		__forceinline static const Vector3_Bool AllTrue() {return Create_Vector3_Bool<1, 1, 1>();}
		__forceinline static const Vector3_Bool AllFalse() {return Create_Vector3_Bool<0, 0, 0>();}
	};

	/////////////////////////////////////////////////////////////////

	__forceinline void SinCos(HF32* s, HF32* c, const HF32 a)
	{
		*s = Sin(a);
		*c = Cos(a);
	}

	//__forceinline bool Finite(HF32 f);

	////////////////////////////////////////////////////////////////

	__forceinline C_Vector& MakeVector(C_Vertex& vertex) {return (C_Vector&) vertex;}
	__forceinline C_Vertex& MakePoint(C_Vector& vector) {return (C_Vertex&) vector;}
	__forceinline C_Normal& MakeNormal(C_Vector& normal) {return (C_Normal&) normal;}
	__forceinline const C_Vector& MakeVector(const C_Vertex& vertex) {return (C_Vector&) vertex;}
	__forceinline const C_Vertex& MakePoint(const C_Vector& vector) {return (C_Vertex&) vector;}
	__forceinline const C_Normal& MakeNormal(const C_Vector& normal) {return (C_Normal&) normal;}
	__forceinline C_Vector& MakeVector(HVec3& vertex) {return (C_Vector&) vertex;}
	__forceinline C_Vertex& MakePoint(HVec3& vector) {return (C_Vertex&) vector;}
	__forceinline C_Normal& MakeNormal(HVec3& normal) {return (C_Normal&) normal;}
	__forceinline const C_Vector& MakeVector(const HVec3& vertex) {return (C_Vector&) vertex;}
	__forceinline const C_Vertex& MakePoint(const HVec3& vector) {return (C_Vertex&) vector;}
	__forceinline const C_Normal& MakeNormal(const HVec3& normal) {return (C_Normal&) normal;}

	__forceinline C_Vertex Min(const C_Vertex& v1, const C_Vertex& v2) {return v1.Min(v2);}
	__forceinline C_Vertex Max(const C_Vertex& v1, const C_Vertex& v2) {return v1.Max(v2);}

	__forceinline HF32 Magnitude2(const C_Vector& vector) {return vector.LengthSquared();}
	__forceinline HF32 Magnitude(const C_Vector& vector) {return vector.Length();}

	__forceinline HMat33 operator ~ (const HMat33& m) {return m.Transposed();}

	__forceinline HF32 Trace(const HMat33& m) {return m.Trace();}

	__forceinline C_Normal Rotated(const C_Normal& v, const C_Normal& n, const HF32& angle)
	{
		HF32 cos, sin;
		Maths::SinCos(&sin, &cos, -angle);
		return MakeNormal(v * cos + ((n | v) * (Maths::Constants<HF32>::One() - cos)) * n + (v ^ n) * sin);
	}

	__forceinline C_Normal Normalised(const C_Vector& vector)
	{
		return vector.Normalised().Normal();
	}

	__forceinline C_Vector ComponentMultiply(const C_Vector v1, const C_Vector v2) 
	{
		HVec4& hv1 = (HVec4&) v1;
		HVec4& hv2 = (HVec4&) v2;
		return C_Vector(hv1 * hv2);
	}

	__forceinline C_Vector ComponentMultiply(const C_Normal v1, const C_Normal v2) 
	{
		HVec4& hv1 = (HVec4&) v1;
		HVec4& hv2 = (HVec4&) v2;
		return C_Vector(hv1 * hv2);
	}

	__forceinline C_Vector ComponentMultiply(const C_Normal v1, const C_Vector v2) 
	{
		HVec4& hv1 = (HVec4&) v1;
		HVec4& hv2 = (HVec4&) v2;
		return C_Vector(hv1 * hv2);
	}

	__forceinline C_Vector ComponentMultiply(const C_Vector v1, const C_Normal v2) 
	{
		HVec4& hv1 = (HVec4&) v1;
		HVec4& hv2 = (HVec4&) v2;
		return C_Vector(hv1 * hv2);
	}

	__forceinline bool Finite(const C_Vector& v) {return Finite(v.X()) & Finite(v.Y()) & Finite(v.Z());}
	__forceinline bool Finite(const C_Vertex& v) {return Finite(v.X()) & Finite(v.Y()) & Finite(v.Z());}
	__forceinline bool Finite(const C_Normal& v) {return Finite(v.X()) & Finite(v.Y()) & Finite(v.Z());}

	////////////////////////////////////////////////////////////////////

	__forceinline void SetIdentity(HMat33* matrix) {matrix->SetIdentity();}
	void Normalise(HMat33* m);

	__forceinline void Diagonal(HMat33* m, const C_Vector& v)
	{
		m->AX(v.X() * Constants<HVec3>::UnitAxisPositiveX());
		m->AY(v.Y() * Constants<HVec3>::UnitAxisPositiveY());
		m->AZ(v.Z() * Constants<HVec3>::UnitAxisPositiveZ());
	}

	__forceinline void InverseDiagonal(HMat33* m, const C_Vector& v)
	{
		m->AX(Rcp(v.X()) * Constants<HVec3>::UnitAxisPositiveX());
		m->AY(Rcp(v.Y()) * Constants<HVec3>::UnitAxisPositiveY());
		m->AZ(Rcp(v.Z()) * Constants<HVec3>::UnitAxisPositiveZ());
	}

	__forceinline void Clear(C_Vector* v) {*v = Constants<C_Vector>::Zero();}
	__forceinline void Clear(C_Vertex* v) {*v = Constants<C_Vertex>::Origin();}

	// update 3D minimum and maximum extents using input position
	__forceinline void UpdateBounds(C_Vertex* min, C_Vertex* max, const C_Vertex& p)
	{
		Maths::HF32 minX, minY, minZ;
		Maths::HF32 maxX, maxY, maxZ;

		minX = (Maths::Min(min->X(), p.X()));
		minY = (Maths::Min(min->Y(), p.Y()));
		minZ = (Maths::Min(min->Z(), p.Z()));
		maxX = (Maths::Max(max->X(), p.X()));
		maxY = (Maths::Max(max->Y(), p.Y()));
		maxZ = (Maths::Max(max->Z(), p.Z()));

		*min = C_Vertex(minX, minY, minZ);
		*max = C_Vertex(maxX, maxY, maxZ);
	}

	__forceinline void Set(HVec4* v, C_Vector xyz, HF32 w)
	{
		*v = HVec4(xyz);
		v->W(w);
	}

	////////////////////////////////////////////////////////////////////
}
