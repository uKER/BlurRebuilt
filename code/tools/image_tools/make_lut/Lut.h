
#pragma once

namespace Lut
{

class C_Value
{
public:
	C_Value();
	C_Value(F32 r, F32 g, F32 b);

	void SetX8R8G8B8(U32 u32value);
	U32 GetX8R8G8B8() const;

public:
	F32 m_R;
	F32 m_G;
	F32 m_B;
};

C_Value operator+ (const C_Value& a, const C_Value& b);
C_Value operator* (const C_Value& a, const F32 x);

class C_Lut
{
public:
	C_Lut();
	void SetIdentity();

	C_Value& GetValue(S32 x, S32 y, S32 z);
	const C_Value& GetValue(S32 x, S32 y, S32 z) const;

	C_Value Apply(const C_Value& src) const;

	bool ExtractStrip(const Attk::C_Image& image);
	bool EmbedStrip(Attk::C_Image& image);
	bool MakeStrip(Attk::C_Image& image);

	bool WriteDDS(IDirect3DDevice9* pD3dDevice, const Util::C_String& filename);
	bool ReadDDS(IDirect3DDevice9* pD3dDevice, const Util::C_String& filename);
	void Apply(Attk::C_Image& image);

	bool CreateVolumeTexture(IDirect3DDevice9* pD3dDevice, IDirect3DVolumeTexture9** ppVolumeTexture );
	bool SetupFromVolumeTexture(IDirect3DDevice9* pD3dDevice, IDirect3DVolumeTexture9* pVolumeTexture );

private:
	S32 m_Size;
	Ut::Vector<C_Value> m_Data;
};

} // namespace Lut