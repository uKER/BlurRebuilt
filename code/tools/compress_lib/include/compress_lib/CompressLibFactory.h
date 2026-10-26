#pragma once

namespace CompressLib
{
	class C_CompressLibFactory
	{
	public:
		static const I_CompressLib* GetCompressLibPtr(const E_CompressionLibrary compressLib);

		static const char* GetCompressLibString(const E_CompressionLibrary compressLib);

		static E_CompressionLibrary GetCompressLibType(const char* compressLib);

	private:
		static const E_CompressionLibrary	m_compressLibEnumTable[];
		static const char*	m_compressLibStringTable[];
	};
}
