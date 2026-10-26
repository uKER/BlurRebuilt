template<typename STORAGE>
void ReverseKnots(STORAGE& storage)
{
	Btl::Vector<typename STORAGE::KnotType> tmp;
	tmp.Resize(storage.GetNumKnots());
	U32 iDst = 0;
	S32 iKnot = storage.GetNumKnots()-1;
	//Copy knots in reverse order.
	for (; iKnot >= 0; --iKnot, ++iDst)
	{
		tmp[iDst] = storage.AcquireKnot(iKnot);
		storage.ReleaseKnot();
	}

	//Write knots.
	for (S32 iKnot = 0; iKnot < storage.GetNumKnots(); ++iKnot)
	{
		typename STORAGE::KnotType& dst = storage.AcquireKnot(iKnot);
		dst = tmp[iKnot];
		dst.tangentOut = -dst.tangentOut;
		dst.tangentIn = -dst.tangentIn;
		storage.CommitKnot();
	}
}
