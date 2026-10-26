#ifndef SPUDAMAGELIGHTWEIGHTLAYER_H_INCLUDED
#define SPUDAMAGELIGHTWEIGHTLAYER_H_INCLUDED

namespace SpuDamage
{
	//! A single light-weight node.
	struct ALIGN128 C_LightWeightNode
	{
		float				position[4];				//!< Current position.
		float				original_position[4];		//!< Original position of the node.
		float				max_displacement[4];		//!< ???
		float				target_position[4];			//!< ???
		float				cached_position[4];			//!< ???
		float				offset_vector[4];			//!< ???
		float				original_deform;			//!< ???
		bool				can_deform;					//!< Set to true if can deform.
	};
	
	//! A single light-weight layer.
	struct ALIGN128 C_LightWeightLayer
	{
		C_LightWeightNode	nodes[16];					//!< Array of nodes.	
	};

};

#endif // #ifdef SPUDAMAGELIGHTWEIGHTLAYER_H_INCLUDED

