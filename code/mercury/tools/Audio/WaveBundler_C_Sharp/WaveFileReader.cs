using System;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Specialized;


namespace WaveBundler
{
	/// <summary>
	/// This class gives you read/write access to a wave file.
	/// </summary>
	public class WaveFileReader : IDisposable
	{
		BinaryReader reader;

		riffChunk mainfile;
		fmtChunk format;
		factChunk fact;
		dataChunk data;

#region General Utilities
		/*
		 * WaveFileReader(string) - 2004 July 28
		 * A fairly standard constructor that opens a file using the filename supplied to it.
		 */
		public WaveFileReader(string filename)
		{
			reader = new BinaryReader(new FileStream(filename, FileMode.Open, FileAccess.Read, FileShare.Read));
		}

		/*
		 * long GetPosition() - 2004 July 28
		 * Returns the current position of the reader's BaseStream.
		 */
		public long GetPosition()
		{
			return reader.BaseStream.Position;
		}

		/*
		 * string GetChunkName() - 2004 July 29
		 * Reads the next four bytes from the file, converts the 
		 * char array into a string, and returns it.
		 */
		public string GetChunkName()
		{
			return new string(reader.ReadChars(4));
		}

		/*
		 * void AdvanceToNext() - 2004 August 2
		 * Advances to the next chunk in the file.  This is fine, 
		 * since we only really care about the fmt and data 
		 * streams for now.
		 */
		public void AdvanceToNext()
		{
			long NextOffset = (long) reader.ReadUInt32(); //Get next chunk offset
			//Seek to the next offset from current position
			reader.BaseStream.Seek(NextOffset,SeekOrigin.Current);
		}
#endregion
#region Header Extraction Methods
		/*
		 * WaveFileFormat ReadMainFileHeader - 2004 July 28
		 * Read in the main file header.  Not much more to say, really.
		 * For XML serialization purposes, I "correct" the dwFileLength
		 * field to describe the whole file's length.
		 */
		public riffChunk ReadMainFileHeader()
		{
			mainfile = new riffChunk();

			mainfile.sGroupID = new string(reader.ReadChars(4));
			mainfile.dwFileLength = reader.ReadUInt32()+8;
			mainfile.sRiffType = new string(reader.ReadChars(4));
			return mainfile;
		}

		public fmtChunk ReadFormatHeader()
		{
			format = new fmtChunk();

			format.sChunkID = "fmt ";
			format.dwChunkSize = reader.ReadUInt32();
			format.wFormatTag = reader.ReadUInt16();
			format.wChannels = reader.ReadUInt16();
			format.dwSamplesPerSec = reader.ReadUInt32();
			format.dwAvgBytesPerSec = reader.ReadUInt32();
			format.wBlockAlign = reader.ReadUInt16();
			format.dwBitsPerSample = reader.ReadUInt32();
			return format;
		} 

		public factChunk ReadFactHeader()
		{
			fact = new factChunk();

			fact.sChunkID = "fact";
			fact.dwChunkSize = reader.ReadUInt32();
			fact.dwNumSamples = reader.ReadUInt32();
			return fact;
		} 

		public dataChunk ReadDataHeader()
		{
			data = new dataChunk();

			data.sChunkID = "data";
			data.dwChunkSize = reader.ReadUInt32();
			data.lFilePosition = reader.BaseStream.Position;
			if (!fact.Equals(null))
				data.dwNumSamples = fact.dwNumSamples;
			else
				data.dwNumSamples = data.dwChunkSize / (format.dwBitsPerSample/8 * format.wChannels);
			//The above could be written as data.dwChunkSize / format.wBlockAlign, but I want to emphasize what the frames look like.
			data.dwMinLength = (data.dwChunkSize / format.dwAvgBytesPerSec) / 60;
			data.dSecLength = ((double)data.dwChunkSize / (double)format.dwAvgBytesPerSec) - (double)data.dwMinLength*60;
			return data;
		} 
#endregion
#region IDisposable Members

		public void Dispose() 
		{
			if(reader != null) 
				reader.Close();
		}

#endregion


	}
}
