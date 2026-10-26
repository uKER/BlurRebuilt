inline Scene::Scene(const Scene& scene)
{
	m_name = scene.m_name;
	m_rootNode = scene.m_rootNode;
	DeregisterNode(&m_rootNode);
	m_rootNode.SetScenePtr(this);
}

inline Scene& Scene::operator = (const Scene& scene)
{
	m_name = scene.m_name;
	m_rootNode = scene.m_rootNode;
	DeregisterNode(&m_rootNode);
	m_rootNode.SetScenePtr(this);
	return *this;
}
