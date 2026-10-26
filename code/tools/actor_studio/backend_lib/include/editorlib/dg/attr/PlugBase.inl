//From I_Plug
template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
I_Plug* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetSourceConnection()
{
	return CONNECTION_POLICY::_GetSourceConnection();
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
const I_Plug* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetSourceConnection() const
{
	return CONNECTION_POLICY::_GetSourceConnection();
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
U32	C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetNumChildren() const
{
	return CHILD_POLICY::_GetNumChildren();
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
I_Plug* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetParentPlug()
{
	return m_flags.isTopLevelPlug ? 0 : m_parent.pPlug;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
const I_Plug* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetParentPlug() const
{
	return m_flags.isTopLevelPlug ? 0 : m_parent.pPlug;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
I_DgNode* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetParentNode()
{
	return m_flags.isTopLevelPlug ? m_parent.pDgNode : 0;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
const I_DgNode* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetParentNode() const
{
	return m_flags.isTopLevelPlug ? m_parent.pDgNode : 0;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
I_Plug* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetChild(U32 idx)
{
	return CHILD_POLICY::_GetChild(idx);
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
const I_Plug* C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetChild(U32 idx) const
{
	return CHILD_POLICY::_GetChild(idx);
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
void C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::SetSourceConnection(I_Plug* pSource, bool triggerTopLevelParentOnChanged)
{
	CONNECTION_POLICY::_SetSourceConnection(pSource, this, triggerTopLevelParentOnChanged);
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
C_ConnectedPlugs& C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetConnections()
{
	return m_connections;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
const C_ConnectedPlugs& C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::GetConnections() const
{
	return m_connections;
}

//From C_PlugBase
template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
void C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::InvokeDependentOnChanged(bool triggerTopLevelParentOnChanged)
{
	//Dirty nodes dependent upon us.
	if (!triggerTopLevelParentOnChanged && IsNodeDirty())
		//If parent already flagged as dirty, return.
		return;
	else if (triggerTopLevelParentOnChanged && m_pOnInputPlugChangedCallback)
		m_pOnInputPlugChangedCallback(this);

	//Dirty child plugs
	for (U32 i = 0; i < CHILD_POLICY::_GetNumChildren(); ++i)
		CHILD_POLICY::_GetChild(i)->InvokeDependentOnChanged(triggerTopLevelParentOnChanged);

	//Dirty plugs connected to us.
	const Btl::List<I_Plug*>& connections = m_connections.GetPlugs();
	for (Btl::List<I_Plug*>::ConstIterator it(connections.Begin()); it != connections.End(); ++it)
	{
		I_Plug* pDestination = *it;
		pDestination->InvokeDependentOnChanged(true);
	}
}

//From C_PlugBase
template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
void C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::ResetDirty()
{
	for (U32 i = 0; i < CHILD_POLICY::_GetNumChildren(); ++i)
		CHILD_POLICY::_GetChild(i)->ResetDirty();
}


template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
bool C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::IsNodeDirty() const
{
	return m_flags.isDirtyOutput;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
bool C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::IsTopLevel() const
{
	return m_flags.isTopLevelPlug;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::~C_PlugBase()
{
	//Make others aware of our destruction.
	CONNECTION_POLICY::_SetSourceConnection(0, this, false);
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::C_PlugBase(
	I_DgNode* pParent,
	void(*pOnInputPlugChangedCallback)(I_Plug* pThis),
	E_PlugType type,
	const char* name)
	:
	m_pOnInputPlugChangedCallback(pOnInputPlugChangedCallback),
	NAME_POLICY(name),
	CONNECTION_POLICY()
{
	m_flags.type = type;
	m_flags.isDirtyOutput = true;
	m_flags.isTopLevelPlug = true;
	BIZ_ASSERT(pParent);
	m_parent.pDgNode = pParent;
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::C_PlugBase(
	I_Plug* pParent,
	void(*pOnInputPlugChangedCallback)(I_Plug* pThis),
	E_PlugType type,
	const char* name)
	:
	m_pOnInputPlugChangedCallback(pOnInputPlugChangedCallback),
	NAME_POLICY(name),
	CONNECTION_POLICY()
{
	m_flags.type = type;
	m_flags.isDirtyOutput = true;
	m_flags.isTopLevelPlug = false;
	BIZ_ASSERT(pParent);
	m_parent.pPlug = pParent;
	pParent->AddChild(this);
}

template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
bool C_PlugBase<NAME_POLICY, CHILD_POLICY, CONNECTION_POLICY>::AddChild(I_Plug* pChild)
{
	return _AddChild(pChild);
}