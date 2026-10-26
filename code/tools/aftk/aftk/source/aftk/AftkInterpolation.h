#ifndef AFTK_INTERPOLATION_H
#define AFTK_INTERPOLATION_H

namespace Aftk
{
	void BuildRotationMatrix(
		Matrix44& rotationMatrix,
		const C_SceneNode* pSceneNode, //Used to acquire values for axes that are not driven.
		const I_Object* pRotXSourceConnection, //Optional: x animation curve/blend weighted
		const I_Object* pRotYSourceConnection, //Optional: y animation curve/blend weighted
		const I_Object* pRotZSourceConnection);//Optional: z animation curve/blend weighted;

	bool DoWeSlerpThisNode(const C_SceneNode* pSceneNode,
		const I_Object* pRotXSourceConnection, //Optional: x animation curve/blend weighted
		const I_Object* pRotYSourceConnection, //Optional: y animation curve/blend weighted
		const I_Object* pRotZSourceConnection);//Optional: z animation curve/blend weighted;
}

#endif
