#pragma once

namespace ResourceBundler
{
	class I_ExportResourceHeader;

	class C_Bundler : public Tools::C_ToolsApplication
	{
	public:
		C_Bundler(
			const char *szName,			// the name of the application
			const char *szSubtitle,		// subtitle e.g. "bizarre creations lighting tool"
			const char *szLogger,		// name of the base logger for this application
			const char *szDmpPath,		// path where .dmp files are to be written out to
			bool bExecuter = false);	// true if the application needs to send executer feedback

		virtual ~C_Bundler();

		int Main(int argc, char *argv[], Progress::I_ProgressBar *pProgress);

		const Btl::Vector<Btl::String>& GetInputs() {return m_Inputs;}

		bool GetPath(Btl::String& path) const;

		void AddResource(I_ExportResourceHeader* pResource);

		bool WriteData(U32& compressedSize, const U8 *pData, U32 numBytes);

		U32 GetDataOffset() const;

		const C_BundlerOutputOptions& GetOutputOptions() const;

	protected:
		void _ShowUsage();

		bool _ParseArguments(int argc, char *argv[]);

		bool _WriteXPRFile();

		void _InitialiseOutputOptions(Display::ResourceBundler::E_Platform platform);

		U32 _EstimateCompressedDataSize(const U32 inputDataSize) const;

		bool _CompressData(
			U8*& pCompressedData,
			U32& compressedDataSize,
			const U8* pInputData,
			const U32 inputDataSize) const;

		void _PadToAlignment(
			U8* pOutputData, 
			U32& outputSize,
			U32 align);

		bool _GetAlignedCopy(
			U8** ppOutputData,
			const U8* pInputData, 
			U32& outputSize,
			U32 align);

		bool _WriteData(
			U8* pOutputData, 
			U32& outputSize,
			const U8* pInputData, 
			U32 inputSize) const;

		C_BundlerOutputOptions	m_outputOptions;

		Btl::String					m_BasePath;
		Btl::Vector<Btl::String>	m_Inputs;
		Btl::String					m_OutputFileName;

		Btl::Vector<I_ExportResourceHeader*>	m_Resources;

		U32					m_dataReserveSize;

		U8*					m_pRawData;
		U32					m_rawDataSize;

		// This magic number identifies the file as a texture bundle file.
		static const U32 m_magicNumber = 'BZR2';
	};
}
