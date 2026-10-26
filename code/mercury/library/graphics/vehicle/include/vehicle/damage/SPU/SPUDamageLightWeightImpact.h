#ifndef SPUDAMAGELIGHTWEIGHTIMPACT_H_INCLUDED
#define SPUDAMAGELIGHTWEIGHTIMPACT_H_INCLUDED

namespace SpuDamage
{

	//! A single light weight impact event (enough for lattice deformation).
	struct ALIGN16 C_LightWeightImpact
	{
		float									local_position[4];			//!< Local position of the impact.
		float									local_normal[4];			//!< Local normal of the impact.
		float __attribute__((aligned(16)))		filter_deform_amount_code;	//!< Code-driven deformation value.
		float __attribute__((aligned(16)))		filter_deform_amount_art;	//!< Artist-driven deformation value.
		float __attribute__((aligned(16)))		impact_size;				//!< Size of the impact (radius?).
		uint16_t __attribute__((aligned(16)))	scratch_amount;				//!< Amount of scratch damage to do.
		uint16_t __attribute__((aligned(16)))	deform_parts;				//!< Non-zero if we should deform parts.
	};

};

#endif // #ifdef SPUDAMAGELIGHTWEIGHTIMPACT_H_INCLUDED

