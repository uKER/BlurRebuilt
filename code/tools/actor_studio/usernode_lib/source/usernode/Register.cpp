#include "stdafx.h"

#include <EditorLib/Core.h>
#include <UserNode/Register.h>

namespace UserNode
{

C_Register::C_Register()
{
}

C_Register::~C_Register()
{
}

void C_Register::Initialise()
{
	REGISTER_DG_NODES(AnimationProp);
	REGISTER_CONTEXTS(AnimationProp);
	REGISTER_RENDER_NODES(AnimationProp);

	REGISTER_DG_NODES(BoxProp);
	REGISTER_CONTEXTS(BoxProp);
	REGISTER_RENDER_NODES(BoxProp);

	REGISTER_DG_NODES(SphereProp);
	REGISTER_CONTEXTS(SphereProp);
	REGISTER_RENDER_NODES(SphereProp);

	REGISTER_DG_NODES(MatrixProp);
	REGISTER_CONTEXTS(MatrixProp);
	REGISTER_RENDER_NODES(MatrixProp);

	REGISTER_DG_NODES(SplineProp);
	REGISTER_CONTEXTS(SplineProp);
	REGISTER_RENDER_NODES(SplineProp);

	REGISTER_DG_NODES(PolyProp);
	REGISTER_CONTEXTS(PolyProp);
	REGISTER_RENDER_NODES(PolyProp);

	REGISTER_DG_NODES(LinkProp);
	REGISTER_RENDER_NODES(LinkProp);

	REGISTER_DG_NODES(CoverNodeProp);
	REGISTER_CONTEXTS(CoverNodeProp);
	REGISTER_RENDER_NODES(CoverNodeProp);

	REGISTER_DG_NODES(RectangleProp);
	REGISTER_CONTEXTS(RectangleProp);
	REGISTER_RENDER_NODES(RectangleProp);

	REGISTER_DG_NODES(DistanceProp);
	REGISTER_RENDER_NODES(DistanceProp);

	REGISTER_DG_NODES(RadiusProp);
	REGISTER_CONTEXTS(RadiusProp);
	REGISTER_RENDER_NODES(RadiusProp);
}

void C_Register::Shutdown()
{
}

}
