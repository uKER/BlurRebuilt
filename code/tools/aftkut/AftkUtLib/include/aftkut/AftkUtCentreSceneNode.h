#ifndef AFTK_CENTRE_ACTOR_H
#define AFTK_CENTRE_ACTOR_H


#ifdef _MSC_VER_
#pragma once
#endif


namespace AftkUt
{
	class C_CentreSceneNode
	{
	public:
		///
		///Centres pivot point of scene node
		static Aftk::C_Status CentreSceneNode(
			Aftk::C_SceneNode* pAftkMeshNode );

	private:
		///
		///Adjusts transformation of child nodes to compensate for
		///changes to parent transformation matrix
		static Aftk::C_Status CentreSceneNodeChildren(
			Aftk::C_SceneNode* pAftkSceneNode, 
			const Aftk::Position3& pivotOffset);
	};
}


#endif