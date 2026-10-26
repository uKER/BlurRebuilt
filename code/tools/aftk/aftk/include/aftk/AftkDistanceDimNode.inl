template<class T>
bool C_DistanceDimNode::GetAdjacentKeysT(T& key1, T& key2) const
{
	key1.GetX() = 0;
	key1.GetValue() = 0;
	key2.GetX() = 0;
	key2.GetValue() = 0;

	for (U32 i = 0; i < 2; i++)
	{
		const I_Object* pObj = m_inputs[i]->GetSourceConnectionPtr();
		if (pObj->HasType(E_ObjType_IEvaluatable))
		{
			T tempKey1;
			T tempKey2;
			if (!((const I_EvaluatableNode*)pObj)->GetAdjacentKeys(tempKey1, tempKey2))
				return false;

			key1.GetX()		+= tempKey1.GetX();
			key1.GetValue() += tempKey1.GetValue();
			key2.GetX()		+= tempKey2.GetX();
			key2.GetValue() += tempKey2.GetValue();
		}
		else if (pObj->HasType(E_ObjType_IPlug))
		{
			//Attempt to evaluate plug.
			F32 result;
			if (((const I_Plug*)pObj)->GetValue(result))
                result;

			key1.GetValue() += result;
			key2.GetValue() += result;
		}
	}

	key1.GetX() = key1.GetX() / 2;
	key1.GetValue() = key1.GetValue() / 2;
	key2.GetX() = key2.GetX() / 2;
	key2.GetValue() = key2.GetValue() / 2;
	return true;
}