#ifndef MAYA_IO_INSTANCE_TRACKER_H
#define MAYA_IO_INSTANCE_TRACKER_H

#ifdef _MSC_VER_
#pragma once
#endif

class Aftk::C_SceneNode;

namespace MayaIo
{
	class C_InstanceTracker
	{
	public:
		MDagPath		m_dagPath;
		U32				m_remainingInstances;
		Aftk::C_SceneNode*	m_pAftkSceneNode;
	};
}

#endif

