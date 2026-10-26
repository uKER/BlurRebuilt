#ifndef __RENDERDATACONVERSION_NODEINFORM_H__
#define __RENDERDATACONVERSION_NODEINFORM_H__

namespace RenderDataConversion
{

// virtual base class that can query information about an aftk node
class I_NodeInform
{
public:
	// returns the route number of the node 
	virtual S32 GetRouteNo(const Aftk::C_SceneNode *pSceneNode) const = 0;
};

}

#endif
