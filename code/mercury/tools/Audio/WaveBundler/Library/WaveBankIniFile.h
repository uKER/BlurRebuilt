#pragma once

#include "wavebundler.h"
#include <system/xml/XmlParser.h>

namespace Audio
{
	class WaveBankIniFile : public Sys::C_XmlContent
	{
	public:
		enum eFormat
		{
			E_FORMAT_NONE=-1,
			E_FORMAT_ADPCM=0,
			E_FORMAT_XMA,
			E_FORMAT_MP3,
			E_FORMAT_PCM_WAV_S16,
			E_FORMAT_PCM_WAV_S16_TO_F32,
			E_FORMAT_PCM_WAV_S16_TO_XMA,
			E_FORMAT_PCM_WAV_S16_TO_XMA2,
			E_FORMAT_PCM_WAV_S16_TO_ADPCM,		
			NUM_FORMATS,
		};
		enum eLoop
		{
			E_LOOP_NOT_SPECIFIED = -1,		// loop was not specified in ini files
			E_LOOP_YES,						// wave should loop
			E_LOOP_NO,						// wave should not loop
		};
		enum eQuantizeMethod
		{
			E_QUANTIZE_RESAMPLE = 0,
			E_QUANTIZE_INSERT_DELETE,
		};
	public:
		static const int DEFAULT_XMA_COMPRESSION_QUALITY = 75;
		static const int DEFAULT_NUM_START_PADDING_SAMPLES = 1201;
		static const int VALUE_NOT_SPECIFIED = -1;
		struct Wave
		{
			Wave(WaveBankIniFile* iniFile) 
			{ 
				m_fDefaultLevel = 1.0f; 
				m_eLoop=E_LOOP_NOT_SPECIFIED; 

				m_eQuantizeMethod = iniFile->m_eDefaultQuantizeMethod;
				m_eFormat = iniFile->m_eDefaultFormat; 
				m_iCompressionQuality = iniFile->m_iDefaultCompressionQuality;
				m_iPitchShiftUp = iniFile->m_iDefaultPitchShiftUp;
				m_iPitchShiftDown = iniFile->m_iDefaultPitchShiftUp;
				m_iPitchShiftQuality = 1;	// TODO
				m_iTargetSampleRate = iniFile->m_iDefaultTargetSampleRate;
				m_iTargetBitDepth = iniFile->m_iDefaultTargetBitDepth;
				m_uNumPaddingSamplesAtStart = DEFAULT_NUM_START_PADDING_SAMPLES;
			}
		public:
			void 				SetFilename(const Util::C_String& str) { m_strFilename = str; }
			void 				SetInternalName(const Util::C_String& str) { m_strInternalName = str; }
			void 				SetFormat(eFormat format) { m_eFormat = format; }
			void 				SetDefaultLevel(float fVal){ m_fDefaultLevel = fVal; }
			void 				SetCompressionQuality(int iVal){ m_iCompressionQuality = iVal; }
			void 				SetPitchShiftUp(int iVal){ m_iPitchShiftUp = iVal; }
			void 				SetPitchShiftDown(int iVal){ m_iPitchShiftDown = iVal; }
			void 				SetPitchShiftQuality(int iVal){ m_iPitchShiftQuality = iVal; }
			void 				SetTargetSampleRate(int iVal){ m_iTargetSampleRate = iVal; }
			void 				SetTargetBitDepth(int iVal){ m_iTargetBitDepth = iVal; }
			void 				SetLoop(eLoop loop){ m_eLoop = loop; }
			//void				SetResamplerQuality(int iVal) { m_giResamplerQuality = iVal; }	// TODO
			void				SetQuantizeMethod(int iMethod) { m_eQuantizeMethod = (eQuantizeMethod)iMethod; }
			void				SetNumPaddingSamplesAtStart(S32 num) { m_uNumPaddingSamplesAtStart = num; }

			Util::C_String&		Filename()				{ return m_strFilename; }
			Util::C_String32	InternalName()			{ return Util::C_String32(m_strInternalName); }
			eFormat				Format()				{ return m_eFormat; }
			float				DefaultLevel()			{ return m_fDefaultLevel; }
			int					CompressionQuality()	{ return m_iCompressionQuality; }
			int					PitchShiftUp()			{ return m_iPitchShiftUp; }
			int					PitchShiftDown()		{ return m_iPitchShiftDown; }
			int					PitchShiftQuality()		{ return m_iPitchShiftQuality; }
			int					TargetSampleRate()		{ return m_iTargetSampleRate; }
			int					TargetBitDepth()		{ return m_iTargetBitDepth; }
			eLoop				Loop()					{ return m_eLoop; }
			eQuantizeMethod		QuantizeMethod()		{ return m_eQuantizeMethod; }
			U32					NumStartPaddingSamples() { return m_uNumPaddingSamplesAtStart; }


			bool				VerifyValues();

		private:
			Util::C_String		m_strFilename;
			Util::C_String		m_strInternalName;
			eFormat				m_eFormat;
			float				m_fDefaultLevel;
			int					m_iCompressionQuality;
			int					m_iPitchShiftUp;
			int					m_iPitchShiftDown;
			int					m_iPitchShiftQuality;
			int					m_iTargetSampleRate;
			int					m_iTargetBitDepth;
			//int					m_giResamplerQuality;
			eLoop				m_eLoop;
			eQuantizeMethod		m_eQuantizeMethod;
			U32					m_uNumPaddingSamplesAtStart;
		};
	public:
								WaveBankIniFile() 
								{ 
									m_iEndian = 1;
									m_bStreaming = false;
									m_iDefaultTargetSampleRate = VALUE_NOT_SPECIFIED;
									m_iDefaultTargetBitDepth = VALUE_NOT_SPECIFIED;
									m_iDefaultCompressionQuality = DEFAULT_XMA_COMPRESSION_QUALITY;
									m_eDefaultFormat = E_FORMAT_NONE;
									m_iDefaultPitchShiftUp = 1;
									m_iDefaultPitchShiftDown = 1;
									m_eDefaultQuantizeMethod = (eQuantizeMethod)gSettings.giQuantizeMethod;	// set to global quantize method
									m_uNumPaddingSamplesAtStart = DEFAULT_NUM_START_PADDING_SAMPLES;
									//m_giResamplerQuality = gSettings.giResamplerQuality;						// set to global resampler quality
								}
								WaveBankIniFile::~WaveBankIniFile(); 

		Util::C_String&			Filename() { return m_strFilename; }
		Util::C_String32&		BankName() { return m_strBankName; }
		Util::C_String&			Path() { return m_strPath; }
		
		int						Endian() { return m_iEndian; }
		int						Streaming() { return m_bStreaming; }

		int						NumWaves() {  return m_vpWaves.Size(); }
		Wave*					GetWave(U32 iWave) 	{ return (m_vpWaves.Size()>=(int)iWave) ? m_vpWaves[iWave] : NULL; }

		void					SetPath(Util::C_String& strPath) { m_strPath = strPath; }
		void					SetFilename(const Util::C_String& strFilename);

		U32						NumStartPaddingSamples() { return m_uNumPaddingSamplesAtStart; }
	private:
		bool					GetFormatEnum(Util::C_String& strFormat, eFormat& eFormat);

		bool					StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);
		bool					EndElement(const Util::C_String& name);

		Wave*					LookupByName(const Util::C_String& strName);	// lookup by internal name

	public:
		int						Load();										// load the ini file and parse it
		bool					Parse(const char* pchData, int numBytes);	// parse xml in memory

	private:
		Util::C_String			m_strFilename;			// filename of the ini file
		Util::C_String32		m_strBankName;			// name of the bank
		
		Util::C_String			m_strPath;				// path the ini file was located in

		int						m_iEndian;				// 0 for little endian, 1 for big
		
		bool					m_bStreaming;			// is this bank a streaming bank?

		S32						m_iDefaultTargetSampleRate;		// default sample rate for each wave in the bank
		S32						m_iDefaultTargetBitDepth;		// default BitDepth for each wave in the bank
		S32						m_iDefaultCompressionQuality;	// default xma compression level 
		eFormat					m_eDefaultFormat;				// default format
		S32						m_iDefaultPitchShiftUp;			// default setting for pitchshifting up
		S32						m_iDefaultPitchShiftDown;		// default setting for pitchshifting down
		eQuantizeMethod			m_eDefaultQuantizeMethod;		// default setting for quantize method
		U32						m_uNumPaddingSamplesAtStart;	// temp for mp3. how many samples does the mp3 encoder add to the start of the sound?
		
		Ut::Vector<Wave*>		m_vpWaves;	// the waves info from the xml file
	};
};

