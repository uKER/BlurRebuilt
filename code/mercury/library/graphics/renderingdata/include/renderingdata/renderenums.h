#ifndef RD_RENDER_ENUMS_H
#define RD_RENDER_ENUMS_H
#ifdef _MSC_VER
#pragma once
#endif


namespace RenderingData
{
	// Type of sorting used for a primitive.
	enum SortType
	{
		SORT_TYPE_KEY, // Sort according to the sort key.
		SORT_TYPE_BACK_FRONT, // Sort from back to front.
		SORT_TYPE_POST_RESOLVE, // Render after the first resolve of the frame buffer.
		SORT_TYPE_RACIFICATION //Special Sort type only for blur racifications 
	};

	// The render pass is not set explicitly but worked out from the sort type and blend mode.
	// (Z-write is then inferred from the render pass.)
	enum RenderPass
	{								// SortType			alpha-blend alpha-test	z-write
		RENDER_PASS_OPAQUE,			// KEY				no			no			yes
		RENDER_PASS_ALPHA_TEST,		// KEY				no			yes			yes
		RENDER_PASS_OVERLAY,		// KEY				yes			-			no
		RENDER_PASS_BACK_FRONT,		// BACK_FRONT		yes			-			no
		RENDER_PASS_POST_RESOLVE,	// POST_RESOLVE		no			-			yes
		RENDER_PASS_RACIFICATION,	// RACIFICATION		yes			-			no
		
		RENDER_PASS_MAX
	};

	// The type of primitive in an index buffer.
	enum PrimitiveType
	{
		PRIMITIVE_TYPE_TRIANGLE_LIST,
		PRIMITIVE_TYPE_TRIANGLE_STRIP,
		PRIMITIVE_TYPE_POINT_LIST, // Non-indexed but we can make an index buffer anyway and not share vertices.
		PRIMITIVE_TYPE_QUAD_LIST, // Can convert to triangles if quads are not supported by the GPU.
		PRIMITIVE_TYPE_QUAD_STRIP
	};

	enum RenderObjectType
	{
		RENDER_OBJECT_UNKNOWN = -1,
		RENDER_OBJECT_MODEL,
		RENDER_OBJECT_INSTANCE,
		RENDER_OBJECT_INSTANCE_BATCH,
		RENDER_OBJECT_CUSTOM = 100
	};
}

#endif
