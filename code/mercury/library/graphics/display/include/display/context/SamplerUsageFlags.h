#ifndef __DISPLAY_SAMPLERUSAGEFLAGS_H__
#define __DISPLAY_SAMPLERUSAGEFLAGS_H__

namespace Display
{

/// contains information about which samplers are enabled
class C_SamplerUsageFlags
{
public:
	/// constructor
	C_SamplerUsageFlags() : m_Flags(0) {}


	/// copy constructor
	C_SamplerUsageFlags(const C_SamplerUsageFlags &flags)
	{
		m_Flags = flags.m_Flags;
	}

	/// enable a sampler
	void SetEnabled(int iVertexShader, int iSampler)
	{
		assert(iVertexShader == 0 || iVertexShader == 1);
		assert(iSampler>=0 && iSampler<(iVertexShader?4:16));
		m_Flags |= (1<<(iSampler+(iVertexShader*16)));
	}

	/// get sampler enabled bit
	bool GetEnabled(int iVertexShader, int iSampler) const
	{
		assert(iVertexShader == 0 || iVertexShader == 1);
		assert(iSampler>=0 && iSampler<(iVertexShader?4:16));
		return (m_Flags & (1<<(iSampler+(iVertexShader*16))))?true:false;
	}

	/// get raw flags
	U32 GetRawFlags() const { return m_Flags; } 

	/// set raw flags
	void SetRawFlags(U32 flags) { m_Flags = flags; }

private:
	U32 m_Flags;
};


} // namespace Display 

#endif // __DISPLAY_CONTEXT_PS3_H__
