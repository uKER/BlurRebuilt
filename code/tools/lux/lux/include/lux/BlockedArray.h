#pragma once

template<class T, int logBlockSize> class BlockedArray
{
public:
	// BlockedArray Public Methods
	BlockedArray(int nu, int nv, const T *d = NULL)
	{
		uRes = nu;
		vRes = nv;
		uBlocks = RoundUp(uRes) >> logBlockSize;
		int nAlloc = RoundUp(uRes) * RoundUp(vRes);
		data = (T *)AllocAligned(nAlloc * sizeof(T));
		for (int i = 0; i < nAlloc; ++i)
			new (&data[i]) T();
		if (d)
			for (int v = 0; v < nv; ++v)
				for (int u = 0; u < nu; ++u)
					(*this)(u, v) = d[v * uRes + u];
	}
	int BlockSize() const { return 1 << logBlockSize; }
	int RoundUp(int x) const
	{
		return (x + BlockSize() - 1) & ~(BlockSize() - 1);
	}
	int uSize() const { return uRes; }
	int vSize() const { return vRes; }
	~BlockedArray() {
		for (int i = 0; i < uRes * vRes; ++i)
			data[i].~T();
		FreeAligned(data);
	}
	int Block(int a) const { return a >> logBlockSize; }
	int Offset(int a) const { return (a & (BlockSize() - 1)); }
	T &operator()(int u, int v)
	{
		int bu = Block(u), bv = Block(v);
		int ou = Offset(u), ov = Offset(v);
		int offset = BlockSize() * BlockSize() *
		             (uBlocks * bv + bu);
		offset += BlockSize() * ov + ou;
		return data[offset];
	}
	const T &operator()(int u, int v) const
	{
		int bu = Block(u), bv = Block(v);
		int ou = Offset(u), ov = Offset(v);
		int offset = BlockSize() * BlockSize() * (uBlocks * bv + bu);
		offset += BlockSize() * ov + ou;
		return data[offset];
	}
	void GetLinearArray(T *a) const
	{
		for (int v = 0; v < vRes; ++v)
			for (int u = 0; u < uRes; ++u)
				*a++ = (*this)(u, v);
	}
private:
	// BlockedArray Private Data
	T *data;
	int uRes, vRes, uBlocks;
};