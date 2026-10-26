template<typename DATA_TYPE>
INLINE C_StaticPlug<DATA_TYPE>* CreateScalar(
	I_DgNode* pParent,
	void(*pOnChangedCallback)(I_Plug*),
	E_PlugType type,
	const char* name,
	const DATA_TYPE& value)
{
	C_StaticPlug<DATA_TYPE>* pPlug = new C_StaticPlug<DATA_TYPE>(pParent, pOnChangedCallback, type, name);
	pPlug->SetValue(value);
	return pPlug;
}

template <class T, int N>
INLINE void SetCompoundValue(C_NullPlug& plug, const UtMaths::BaseVector<T, N>& value, bool triggerTopLevelParentOnChanged)
{
	for (U32 i = 0; i < value.SIZE; i++)
		((C_StaticPlug<T>*)plug.GetChild(i))->SetValue(value[i], triggerTopLevelParentOnChanged);
}

template <class T, int N>
INLINE void GetCompoundValue(const I_Plug& plug, UtMaths::BaseVector<T, N>& value, bool update)
{
	for (U32 i = 0; i < value.SIZE; i++)
		plug.GetChild(i)->GetValue(value[i], update);
}
