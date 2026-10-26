// convert a hex digit into an int
INLINE int Utility::HexToI(char c)
{
	int v = c-'0';
	if (v>=0 && v<10)
		return v;
	v = c-'a';
	if (v>=0 && v<6)
		return v + 10;
	v = c-'A';
	if (v>=0 && v<6)
		return v + 10;
	return -1;
}

template<typename T>
INLINE bool Utility::ConvertHexToBin(const std::string &name, T *pData)
{
	if (name.length() != sizeof(T) * 2)
		return false; // not the expected length

	U8 *pBinData = (U8*)pData;

	// convert hex number to binary data
	const char *psz = name.c_str();
	for (int i=0; i<sizeof(T); i++, pBinData++, psz+=2)
	{
		int v1 = HexToI(psz[0]);
		if (v1 == -1)
			return false;
		int v2 = HexToI(psz[1]);
		if (v2 == -1)
			return false;
		*pBinData = (v1 << 4) + v2;
	}
	return true;
}

bool Utility::GetNodeSHLighting(const Aftk::C_SceneNode &node, LuxSamplePoint &sample)
{
	const Aftk::I_Plug *pPlug = node.GetPlugPtr("luxCustom");
	if (!pPlug)
		return false;

	std::string text;
	pPlug->GetValue(text);

	return ConvertHexToBin(text, &sample);
}

bool Utility::DoesNodeHaveSHLighting(const Aftk::C_SceneNode &node)
{
	const Aftk::I_Plug *pPlug = node.GetPlugPtr("luxCustom");
	if (!pPlug)
		return false;

	std::string text;
	pPlug->GetValue(text);

	return text.length() == sizeof(LuxSamplePoint)*2;
}

void Utility::SetupDefaultSHBlindData(Aftk::C_SceneNode* pSource, Aftk::C_SceneNode* pDest)
{
	// add default SH parameters
	Aftk::C_GenericAttribute SH_R;
	Aftk::C_GenericAttribute SH_G;
	Aftk::C_GenericAttribute SH_B;
	Aftk::Vector4F32 SHVector(2.0f*sqrtf(Maths::Pi), 0.0f, 0.0f, 0.0f);

	SH_R.SetName("SH_R");
	SH_R.SetValue(SHVector);

	SH_G.SetName("SH_G");
	SH_G.SetValue(SHVector);

	SH_B.SetName("SH_B");
	SH_B.SetValue(SHVector);

	const Aftk::C_BlindDataResource* pBDRes = pSource->GetBlindDataResourcePtr();
	Aftk::C_BlindDataNode BlindData;
	if (pBDRes)
	{
		BlindData = pBDRes->GetContent();
	}
	
	BlindData.AddAttribute(SH_R);
	BlindData.AddAttribute(SH_G);
	BlindData.AddAttribute(SH_B);

	const Aftk::C_BlindDataResource *ResourcePtr = pSource->GetScenePtr()->AddBlindDataNode(BlindData);
	pDest->SetBlindDataResourcePtr(ResourcePtr);
}