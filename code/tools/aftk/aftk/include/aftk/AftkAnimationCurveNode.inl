template<class STORAGE>
void C_AnimationCurveNode::BinarySearchForKeyPair(
	S32& index1,
	S32& index2,
	const STORAGE& storage, 
	F32 x) const
{
	if (storage.GetNumKnots() == 0)
	{
		index1 = -1;
		index2 = -1;
		return;
	}
	if (storage.GetNumKnots() == 1)
	{
		index1 = 0;
		index2 = -1;
		return;
	}

	index1 = Curve::Helpers::GetKnotInterval(&storage, x);
	index2 = index1+1;
}


