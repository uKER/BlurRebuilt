#ifndef TU_MATHS_CONVERSION_H
#define TU_MATHS_CONVERSION_H

namespace Tu
{
	inline Maths::HMat43 TemplateToNightmareMatrix43(
		const UtMaths::MMatrix< UtMaths::BasicMatrix<UtMaths::MVector< UtMaths::VectorXYZW<F32> >, 4 > >& rhs);

	inline Maths::HMat44 TemplateToNightmareMatrix44(
		const UtMaths::MMatrix< UtMaths::BasicMatrix<UtMaths::MVector< UtMaths::VectorXYZW<F32> >, 4 > >& rhs);

	inline Maths::HMat44 TemplateToNightmareMatrix44(
		const UtMaths::MMatrix< UtMaths::BasicMatrix<UtMaths::MVector< UtMaths::VectorXYZW<F32> >, 4 > >& rhs);

	inline UtMaths::MMatrix< UtMaths::BasicMatrix<UtMaths::MVector< UtMaths::VectorXYZW<F32> >, 4 > >
		NightmareToTemplateMatrix44(const Maths::HMat44& rhs);

	inline UtMaths::MMatrix< UtMaths::BasicMatrix<UtMaths::MVector< UtMaths::VectorXYZW<F32> >, 4 > >
		NightmareToTemplateMatrix43(const Maths::HMat43& rhs);

	inline Maths::HVec3 TemplateToNightmare( 
		const UtMaths::MVector< UtMaths::VectorXYZ<F32> >& rhs);

	inline Maths::HVec4 TemplateToNightmare( 
		const UtMaths::MVector< UtMaths::VectorXYZW<F32> >& rhs);

	inline Maths::HVec3 TemplateToNightmare( 
		const UtMaths::MPoint< UtMaths::VectorXYZ<F32> >& rhs);

	inline Maths::HVec3 TemplateToNightmare( 
		const UtMaths::MNormal< UtMaths::VectorXYZ<F32> >& rhs);

	inline UtMaths::MVector< UtMaths::VectorXYZ<F32> > NightmareToTemplate(
		const Maths::HVec3& rhs);

	inline UtMaths::MVector< UtMaths::VectorXYZW<F32> > NightmareToTemplate(
		const Maths::HVec4& rhs);


	__forceinline Maths::HMat43 Tu::TemplateToNightmareMatrix43(
		const Aftk::Matrix44& rhs)
	{
		return Maths::HMat43(
			Maths::HVec3(rhs[0][0], rhs[0][1], rhs[0][2]),
			Maths::HVec3(rhs[1][0], rhs[1][1], rhs[1][2]),
			Maths::HVec3(rhs[2][0], rhs[2][1], rhs[2][2]),
			Maths::HVec3(rhs[3][0], rhs[3][1], rhs[3][2]));
	}
	__forceinline Maths::HMat44 Tu::TemplateToNightmareMatrix44(
		const Aftk::Matrix44& rhs)
	{
		return Maths::HMat44(
			Maths::HVec4(rhs[0][0], rhs[0][1], rhs[0][2], rhs[0][3]),
			Maths::HVec4(rhs[1][0], rhs[1][1], rhs[1][2], rhs[1][3]),
			Maths::HVec4(rhs[2][0], rhs[2][1], rhs[2][2], rhs[2][3]),
			Maths::HVec4(rhs[3][0], rhs[3][1], rhs[3][2], rhs[3][3]));
	}

	__forceinline Aftk::Matrix44 
		Tu::NightmareToTemplateMatrix44(const Maths::HMat44& rhs)
	{
		Aftk::Matrix44 lhs;
		lhs[0][0] = rhs.AX().X(); lhs[0][1] = rhs.AX().Y(); lhs[0][2] = rhs.AX().Z(); lhs[0][3] = rhs.AX().W();
		lhs[1][0] = rhs.AY().X(); lhs[1][1] = rhs.AY().Y(); lhs[1][2] = rhs.AY().Z(); lhs[1][3] = rhs.AY().W();
		lhs[2][0] = rhs.AZ().X(); lhs[2][1] = rhs.AZ().Y(); lhs[2][2] = rhs.AZ().Z(); lhs[2][3] = rhs.AZ().W();
		lhs[3][0] = rhs.AW().X(); lhs[3][1] = rhs.AW().Y(); lhs[3][2] = rhs.AW().Z(); lhs[3][3] = rhs.AW().W();
		
		return lhs;
	}

	__forceinline Aftk::Matrix44 
		Tu::NightmareToTemplateMatrix43(const Maths::HMat43& rhs)
	{
		Aftk::Matrix44 lhs;
		lhs[0][0] = rhs.AX().X(); lhs[0][1] = rhs.AX().Y(); lhs[0][2] = rhs.AX().Z(); lhs[0][3] = 0;
		lhs[1][0] = rhs.AY().X(); lhs[1][1] = rhs.AY().Y(); lhs[1][2] = rhs.AY().Z(); lhs[1][3] = 0;
		lhs[2][0] = rhs.AZ().X(); lhs[2][1] = rhs.AZ().Y(); lhs[2][2] = rhs.AZ().Z(); lhs[2][3] = 0;
		lhs[3][0] = rhs.AW().X(); lhs[3][1] = rhs.AW().Y(); lhs[3][2] = rhs.AW().Z(); lhs[3][3] = 1.f;
		
		return lhs;
	}

	Maths::HVec3 Tu::TemplateToNightmare( 
		const UtMaths::MVector< UtMaths::VectorXYZ<F32> >& rhs)
	{
		return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
	}

	Maths::HVec4 Tu::TemplateToNightmare( 
		const UtMaths::MVector< UtMaths::VectorXYZW<F32> >& rhs)
	{
		return Maths::HVec4(rhs[0], rhs[1], rhs[2], rhs[3]);
	}

	Maths::HVec3 Tu::TemplateToNightmare( 
		const UtMaths::MPoint< UtMaths::VectorXYZ<F32> >& rhs)
	{
		return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
	}

	Maths::HVec3 Tu::TemplateToNightmare( 
		const UtMaths::MNormal< UtMaths::VectorXYZ<F32> >& rhs)
	{
		return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
	}

	UtMaths::MVector< UtMaths::VectorXYZ<F32> > Tu::NightmareToTemplate(
		const Maths::HVec3& rhs)
	{
		return UtMaths::MVector< UtMaths::VectorXYZ<F32> >(rhs[0], rhs[1], rhs[2]);
	}

	UtMaths::MVector< UtMaths::VectorXYZW<F32> > Tu::NightmareToTemplate(
		const Maths::HVec4& rhs)
	{
		return UtMaths::MVector< UtMaths::VectorXYZW<F32> >(rhs[0], rhs[1], rhs[2], rhs[3]);
	}

};

#endif
