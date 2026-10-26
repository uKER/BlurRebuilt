#ifndef ANIMATION_BLENDMODES_H
#define ANIMATION_BLENDMODES_H

#ifdef _MSC_VER
#pragma once
#endif

// TODO: rename this file to CommonEnums or AnimationEnums.

namespace Animation
{
	/// Animation blend modes.
	enum E_BlendMode
	{
		E_BlendMode_Accumulate = 0,				///< Accumulative blend. Equivalent to a weighted sum.
		E_BlendMode_Lerp,						///< Linear blend. Equivalent to linear interpolation.
		E_BlendMode_Additive,					///< Additive blend.
		E_BlendMode_Count,
		E_BlendMode_Invalid = E_BlendMode_Count	///< Invalid blend mode.
	};
	
	/*
	Defines what happens when a cycle is evaluated at, or beyond, its end and start times.
	*/
	enum E_LoopMode
	{
		E_LoopMode_Continuous	= 0,	///< The cycle continuously loops.
		E_LoopMode_Once,				///< The cycle plays once through once and then stops at its start or end frame.
		E_LoopMode_Count
	};	

}

#endif
