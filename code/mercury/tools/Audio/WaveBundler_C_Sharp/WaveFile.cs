namespace WaveBundler
{
	public class riffChunk 
	{
		public string FileName;
		//These three fields constitute the riff header
		public string sGroupID;         //RIFF
		public uint   dwFileLength;		//In bytes, measured from offset 8
		public string sRiffType;        //WAVE, usually
	}

	public class fmtChunk 
	{
		public string  sChunkID;    	//Four bytes: "fmt "
		public uint    dwChunkSize;     //Length of header
		public ushort  wFormatTag;  	//1 if uncompressed
		public ushort  wChannels;       //Number of channels: 1-5
		public uint    dwSamplesPerSec; //In Hz
		public uint    dwAvgBytesPerSec;//For estimating RAM allocation
		public ushort  wBlockAlign;     //Sample frame size in bytes
		public uint    dwBitsPerSample; //Bits per sample
	}

	/* The fact chunk is used for specifying the compression ratio of the data */
	public class factChunk 
	{
		public string sChunkID;    		//Four bytes: "fact"
		public uint   dwChunkSize;    	//Length of header
		public uint   dwNumSamples;    	//Number of audio frames;
		//numsamples/samplerate should equal file length in seconds.
	}

	public class dataChunk 
	{
		public string sChunkID;    		//Four bytes: "data"
		public uint   dwChunkSize;    	//Length of header

		//The following non-standard fields were created to simplify
		//editing.  We need to know, for filestream seeking purposes,
		//the beginning file position of the data chunk.  It's useful to
		//hold the number of samples in the data chunk itself.  Finally,
		//the minute and second length of the file are useful to output
		//to XML.
		public long	  lFilePosition;	//Position of data chunk in file
		public uint   dwMinLength;		//Length of audio in minutes
		public double dSecLength;		//Length of audio in seconds
		public uint   dwNumSamples;		//Number of audio frames
	}

	public class WaveFile
	{
		public riffChunk	maindata;
		public fmtChunk		format;
		public factChunk	fact;
		public dataChunk	data;
	}
}