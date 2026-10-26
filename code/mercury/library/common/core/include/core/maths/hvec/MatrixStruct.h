#ifndef MATHS_MATRIXSTRUCT_H
#define MATHS_MATRIXSTRUCT_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Maths 
{

//"MatrixStruct"
//
// matrices of constant format across platforms.
//
//
#ifdef USER_WALTERL
#pragma message("MATRIX STRUCT INCLUDED")
#endif
//
// ---------------------------------------------------------
// |--	HmatCol43: 4x3 Matrix-of-columns (all platforms) --|
// ---------------------------------------------------------
//
// "optimized for Smaller Data Size & passing to Skinning" 
// Struct to emphasise direct component acess is meaningfull.
// but with acessors to interoperate with HMat43
//
// note - using struct/public to emphasise component acess is exposed.
//


struct	HMatCol43
{
	HVec4	m_CX,m_CY,m_CZ;
	inline void GetAxes(HVec4& ax, HVec4& ay, HVec4& az) const;
	inline void SetAxes(	const HVec4& ax,
				const HVec4& ay,
				const HVec4& az);

	inline HVec3	AX() const;
	inline HVec3	AY() const;
	inline HVec3	AZ() const;
	inline void	AX(const HVec3& src) const;
	inline void	AY(const HVec3& src) const;
	inline void	AZ(const HVec3& src) const;

	void	SetAxesScaled(
			const HVec4& ax,
			const HVec4& ay,
			const HVec4& az,
			const HVec4& aw,
			const HVec4& scaleXYZ);

};


//
// -------------------------------------------------
// |---	3x4 Matrix-Of-Axes (all platforms)	---|
// -------------------------------------------------
//
// "Optimized for axis manipulation fastest/smallest code"
//
// note - using struct/public to emphasise component acess is exposed.
//


struct	HMatAxes34
{
	HVec4	m_AX,m_AY,m_AZ;

	// Support similar component interface to HMat43,
	// 
	inline	void	GetAxes(HVec4& ax,HVec4& ay, HVec4& az) const {
		ax=m_AX; ay=m_AY; az=m_AZ;
	}
	inline void	SetAxes(const HVec4& ax, const HVec4& ay, const HVec4& az) {
		m_AX = ax; m_AY = ay; m_AZ = az;
	}
	inline void AX(const HVec3& ax) {	m_AX = ax;}
	inline void AY(const HVec3& ay) {	m_AY = ay;}
	inline void AZ(const HVec3& az) {	m_AZ = az;}
	inline HVec4 AX() const { return m_AX; }
	inline HVec4 AY() const { return m_AY; }
	inline HVec4 AZ() const { return m_AZ; }
};

//
// -------------------------------------------------
// |---	4x4 Matrix-Of-Axes (all platforms)	---|
// -------------------------------------------------
//
//
// "Optimized for Axis Manipulation fastest/Smallest code"
//	
// note - using struct/public to emphasise component acess is exposed.
//

struct	HMatAxes44 : public HMatAxes34
{
	HVec4	m_AW;

	inline	void	GetAxes(HVec4& ax,HVec4& ay, HVec4& az, HVec4& aw) const {
		ax=m_AX; ay=m_AY; az=m_AZ; aw=m_AW;
	}
	inline HVec4 AW() const		{	return m_AW;	}
	inline void AW(const HVec4& aw)	{	m_AW = aw;	}

	inline void	SetAxesScaled(
			const HVec4& ax,
			const HVec4& ay,
			const HVec4& az,
			const HVec4& aw,
			const HVec4& scaleXYZ) 
	{
		HVec4	sx,sy,sz;
		VecSplatXYZ(scaleXYZ, sx,sy,sz);
		m_AX = ax * sx;
		m_AY = ay * sy;
		m_AZ = az * sz;
		m_AW = aw;
	}
};


}

#endif





