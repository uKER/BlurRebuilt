using System;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;

namespace WaveBundler
{
	public class WaveBankFile
	{
		public const uint WAVE_BANK_VERSION = 1;
		public const int WAVE_NAME_SIZE=16;
	
		public struct WaveBankHeader
		{
			public byte[] uTag;
			public uint	uSize;			// header size
			public uint	uVersion;		// version number of this bank
			public uint	uNumWaves;		// number of waves in this bank
		}

		public struct WaveHeader
		{
			public byte[] uTag;
			public uint uSize;
			public uint uType;			
			public byte[] pchName;
			public uint uDataStart;
			public uint uDataLength;
			public float fDefaultLevel;
		};

		struct PCMWave
		{
			public uint uSampleRate;
			public uint uBitsPerSample;
			public uint uNumChannels;
			public uint uStride;
		};

		struct FloatWave
		{
			public uint uSampleRate;
			public uint uNumChannels;
			public uint uStride;
		};

		struct DataHeader
		{
			public uint uTag;
			public uint uSize;
		};

		enum Format 
		{ 
			eWaveFormatNone, 
			eWaveFormatPCM, 
			eWaveFormatFloat, 
			eWaveFormatXMA, 
			eWaveFormatVAG, 
			eNumWaveFormats 
		};

		public string sFileName;
		public byte[] wavData;
		
		// create a wavebank from an inifile
		public bool Write(WaveBankIniFile iniFile)
		{
			// make a bank header
			WaveBankHeader hdr = new WaveBankHeader();
			sFileName = iniFile.name+".baf";

			hdr.uTag = new byte[4];
			hdr.uTag = StrToByteArray("BANK");
			hdr.uVersion = WAVE_BANK_VERSION;
			hdr.uSize = (uint)Marshal.SizeOf(typeof(WaveBankHeader));
			hdr.uNumWaves = (uint)iniFile.Waves.Length;

			try
			{
				// idea to make neater: 
				// have a WriteBankHeader() function which creates a bank header and writes it to the bank file/ram?
				// need to read the data in and cache it to write out at the data chunk stage.
				// ReadWaveFileHeaders (from wave file) and WriteWaveHeader which writes it to the file/ram.
				// ReadWaveData needs to read the wave data and store it in Ram.
				// need a WriteWaveHeaders and WriteWaveData

				// open wavebank for writing and write header
				using(BinaryWriter bw = new BinaryWriter(new FileStream(sFileName, FileMode.Create, FileAccess.Write, FileShare.None)))
				{
					bw.Write(hdr.uTag, 0, 4);
					bw.Write(hdr.uSize);
					bw.Write(hdr.uVersion);
					bw.Write(hdr.uNumWaves);
				
					for (int i=0; i < iniFile.Waves.Length; i++)
					{
						WaveFileReader reader = new WaveFileReader(iniFile.Waves[i].filename);
						WaveFile contents = new WaveFile();
						contents.maindata = reader.ReadMainFileHeader(); 
						contents.maindata.FileName = iniFile.Waves[i].filename;

						Console.WriteLine("Opened file {0}", iniFile.Waves[i].filename); 

						string temp;
						while (reader.GetPosition() < (long) contents.maindata.dwFileLength)
						{
							temp = reader.GetChunkName();
							if (temp=="fmt ")
							{
								contents.format = reader.ReadFormatHeader();
								if (reader.GetPosition()+contents.format.dwChunkSize == contents.maindata.dwFileLength)
									break;
							}
							else if (temp=="fact")
							{
								contents.fact = reader.ReadFactHeader();
								if (reader.GetPosition()+contents.fact.dwChunkSize == contents.maindata.dwFileLength)
									break;
							}
							else if (temp=="data")
							{
								contents.data = reader.ReadDataHeader();
								if (reader.GetPosition()+contents.data.dwChunkSize == contents.maindata.dwFileLength)
									break;
							}
							else
							{	//This provides the required skipping of unsupported chunks.
								reader.AdvanceToNext();
							}
						}
						
						WaveHeader wHdr = new WaveHeader();
						
						// take out info we need and put in wavebank header
						wHdr.uTag = StrToByteArray("WAVE");
						wHdr.uSize = (uint)Marshal.SizeOf(typeof(WaveBankHeader));
						wHdr.uType = (uint)Format.eWaveFormatPCM;			
						wHdr.pchName = new byte[WAVE_NAME_SIZE];
						wHdr.pchName = StrToByteArray(iniFile.Waves[i].internalName);
						wHdr.uDataStart=0;
						wHdr.uDataLength=0;
						wHdr.fDefaultLevel = 1.0f;
	
						// write wave header
						bw.Write(wHdr.uTag, 0, 4);
						bw.Write(wHdr.uSize);
						bw.Write(wHdr.uType);
						bw.Write(wHdr.pchName, 0, WAVE_NAME_SIZE);
						bw.Write(wHdr.uDataStart);
						bw.Write(wHdr.uDataLength);
						bw.Write(wHdr.fDefaultLevel);
						
						Console.WriteLine("Wrote header");
					}
				}
			}

			catch (Exception e)
			{
				File.Delete(sFileName);	
				Console.WriteLine("\n\nError (Bank file was not created):\n  \"{0}\"\n", e.GetBaseException().Message);
				return false;
			}

			return true;
		}

		public static byte[] StrToByteArray(string str)
		{
			System.Text.ASCIIEncoding encoding=new System.Text.ASCIIEncoding();
			return encoding.GetBytes(str);
		}
	}
}