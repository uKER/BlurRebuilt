#ifndef NEXUS_RENDER_DEFINITIONS_H
#define NEXUS_RENDER_DEFINITIONS_H

//#define LOAD_SHADERS_FROM_FILE

namespace NexusRender
{
	class RenderNode;

	typedef Ut::UtVector<Ut::DynamicVector<RenderNode* > >	RenderNodeList;

	enum E_SelectionMode
	{
		E_SelectionMode_Object,
		E_SelectionMode_Face,
		E_SelectionMode_Point,

		E_SelectionMode_Count
	};

	enum E_FillMode
	{
		E_FillMode_Solid,
		E_FillMode_WireFrame,
		E_FillMode_Point
	};
}

#endif
