//Function writes the ADPCM header

void writeADHeader(fpout,chans,sr,isLoop,dataBytesSize)
{
	int headerBlock[12];
	short BlockAlign = 16 / 8 * chans;
	int	AvgBytesPerSec = sr * BlockAlign;
	unsigned short compCode = 0xAAAA; //Bizarre ADPCM compression code
	short xTraFmtBytes = 2;
	
	

	int CodeAndChans = ShortToInt(chans,compCode);
	int bAlignAndBits = ShortToInt(16,BlockAlign);
	int fmtBytesAndLoop = ShortToInt(isLoop,xTraFmtBytes);
	int fileSize = (dataBytesSize + sizeof(headerBlock));


	headerBlock[0] = 0x46464952; //RIFF
	headerBlock[1] = fileSize -	8; //file size - 8 (header bytes)
	headerBlock[2] = 0x45564157;	//WAVE
	headerBlock[3] = 0x20746d66;// fmt
	headerBlock[4] = 20;//chunk	data size 16 + 4 extra format bytes
	headerBlock[5]	= CodeAndChans;
	headerBlock[6] = sr; //SR
	headerBlock[7] = AvgBytesPerSec;
	headerBlock[8]	= bAlignAndBits; //block align + bit depth 
	headerBlock[9] = fmtBytesAndLoop; //xtra format	bytes +	one	shot/loop 
	headerBlock[10]	= 0x61746164; //ta
	headerBlock[11]	= dataBytesSize; //data	chunk size 


	fwrite(&headerBlock,sizeof(int),12,fpout);//write the header

}



unsigned int ShortToInt(unsigned short p0,unsigned short p1)
{

	unsigned int a1 = 0;
	a1+=p0 << 16;
	a1 = a1 | p1;

	return a1;
}