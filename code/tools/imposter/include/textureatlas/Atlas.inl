

template<typename PackStrategy>
C_Atlas<PackStrategy>::C_Atlas()
{
}

template<typename PackStrategy>
C_Atlas<PackStrategy>::~C_Atlas()
{
}

template<typename PackStrategy>
bool C_Atlas<PackStrategy>::AddImage(const Btl::String& fileName, const ImageSize& size)
{
	Btl::String f = fileName;
	f = f.Lower();
	f = Misc::FormatFileString(f);

	if (!f.Length())
		return false;

	//find duplicate
	for (S32 i=0; i<m_inputImageDesc.Size(); ++i)
		if (m_inputImageDesc[i].fileName == f)
			return true;

	m_inputImageDesc.PushBack();
	m_inputImageDesc.Back().fileName = f;
	m_inputImageDesc.Back().size = size;

	return true;
}


template<typename PackStrategy>
bool C_Atlas<PackStrategy>::RemoveImage(const Btl::String& fileName)
{
	Btl::String f = fileName;
	f = f.Lower();
	f = Misc::FormatFileString(f);

	if (!f.Length())
		return false;

	//find duplicates
	for (S32 i=0; i<m_inputImageDesc.Size(); ++i)
	{
		if (m_inputImageDesc[i].fileName == f)
		{
			m_inputImageDesc.Erase(i);
			return true;
		}
	}

	return false;
}

template<typename PackStrategy>
const InputImageDescArray& C_Atlas<PackStrategy>::GetInputImageDescArray() const
{
	return m_inputImageDesc;
}

template<typename PackStrategy>
const OutputImageDescArray& C_Atlas<PackStrategy>::GetOutputImageDescArray() const
{
	return m_outputImageDesc;
}

template <typename T>
bool IsPow2(T k)
{
	return (k & (k-1)) == 0;
}

template <typename T>
T UnsignedRoundUpPow2(T k)
{
        if (k == 0)
                return 1;
        k--;
        for (int i=1; i<sizeof(T)*CHAR_BIT; i<<=1)
                k = k | k >> i;
        return k+1;
}

template<typename PackStrategy>
bool C_Atlas<PackStrategy>::Pack(ImageSizeArray& result, U32 maxWidth, U32 maxHeight, U32 maxAtlases, U32 border)
{
	//estimate the number and size of atlases needed
	
	U32 maxX = 0, maxY = 0, minX = Maths::Constants<U32>::Max(), minY = Maths::Constants<U32>::Max();
	U32 area = 0;

	for (S32 i=0; i<m_inputImageDesc.Size(); ++i)
	{
		maxX = Maths::Max(maxX, m_inputImageDesc[i].size.x);
		maxY = Maths::Max(maxY, m_inputImageDesc[i].size.y);

		minX = Maths::Min(minX, m_inputImageDesc[i].size.x);
		minY = Maths::Min(minY, m_inputImageDesc[i].size.y);

		area += m_inputImageDesc[i].size.x*m_inputImageDesc[i].size.y;
	}

	if (!maxX || !maxY || maxX > maxWidth || maxY > maxHeight || maxAtlases ? area > maxWidth*maxHeight*maxAtlases : false)
		return false;

	minX = minX ? 1 : minX;
	minY = minY ? 1 : minY;

	bool success = false;
	result.Clear();
	U32 atlasArea = 0;

	U32 w = IsPow2(minX) ? minX: UnsignedRoundUpPow2(minX);
	U32 h = IsPow2(minY) ? minY : UnsignedRoundUpPow2(minY);

	result.PushBack(ImageSize(w,h));
	while (result.Back().x<=maxWidth && result.Back().y<=maxHeight)
	{
		ImageSize& lastSize = result.Back();
		if (atlasArea+lastSize.x*lastSize.y > area)
		{
			m_outputImageDesc.Clear();
			//attempt to pack atlas using current dimensions			
			PackStrategy p;
			if (p.Pack(m_inputImageDesc, m_outputImageDesc, result, border))
			{
				success = true;				
				break;
			}
		}

		//grow atlas		
		if (lastSize.x > lastSize.y)
			lastSize.y = Maths::Min(lastSize.y*2, maxHeight);
		else
			lastSize.x = Maths::Min(lastSize.x*2, maxWidth);

		if (lastSize.x>=maxWidth && lastSize.y>=maxHeight)
		{
			result.PushBack(ImageSize(w,h));
			atlasArea += maxWidth*maxHeight;
			if (maxAtlases && (U32)result.Size() > maxAtlases)
				break;
		}
	}

	return success;
}