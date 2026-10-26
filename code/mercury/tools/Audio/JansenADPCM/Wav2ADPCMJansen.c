/*
 * JansenADPCM
 * Version 2.1 (adpcm header added)
 * Jack Jansen ADPCM encoder
 *
 * Input restricted to 16-bit PCM , output is 4-bit ADPCMtests
 * This code is currently full of debug data, try and ignore it 
 */
#include <stdio.h>
#include "adpcm.h"



//struct adpcm_state state;
//struct adpcm_state state2;

#define NSAMPLES 2  //must be even number!!!

/*char	abuf[NSAMPLES/2];
short	sbuf[NSAMPLES];
char	abuf2[NSAMPLES/2];
short	sbuf2[NSAMPLES];*/
main(
	int			argc,
	char			*argv[])
{
	
	/*unsigned char		sample_char;
	short			sample_short;
	unsigned char		code;
	short			sample;
	int			resid;
	int			in_coding;
	int			in_size;
		int			out_coding;
	int			out_size;
	unsigned		*in_buf;
	int			(*enc_routine)();
	int			enc_bits;
		int			(*dec_routine)();
	int			dec_bits;*/

	/*int n;

		FILE *fpin,*fpout,*fpReformedData;
int test=0;
int bitDepth = 0;

char filenameInput[500]; //temp filename buffer
char filename1[500];
char filename2[500];
char filename3[500];
char filename4[500];
int byteBlock = 0;
char *tokenPtr;
long dataChunkSize;
long rawDataCount = 0; 


int nSamples; 


	int dataHex = 0x61746164; //"atad"
	int fmtHex = 0x20746D66; //" mtf"
	int fmtJUNK = 0x4B4E554A;

int blockCount = 0;

int tester = 0;*/
	
	int end = 0;
	int argCount = argc;
	
char	abuf[NSAMPLES/2];
short	sbuf[NSAMPLES];
char	abuf2[NSAMPLES/2];
short	sbuf2[NSAMPLES];

	char	abufPre[16/2]; //store first 8 bytes
short	sbufPre[16]; //store first 8 samples

	char	abufPost[16/2]; //store first 8 bytes
short	sbufPost[16]; //store first 8 samples



struct adpcm_state state;
struct adpcm_state state2;

char filenameInput[500]; //temp filename buffer
char filename1[500];
char filename2[500];
//char filename3[500];
//char filename4[500];

short chans = 0;
int sr = 0;




		FILE *fpin,*fpout,*fpout2/*,*fpReformedData*/; //reformeddata needed for oputput wav
printf("*****************************************\n");
printf("*     JansenADPCM.exe  4.0 19/5/07      *\n");
printf("*This program creates 2 files in        *\n");
printf("*the folder of the input file.          *\n");
printf("*The first is the 4-bit data file       *\n");
printf("*[filename].adpcm                       *\n");
printf("*The second is the uncompressed	        *\n");
printf("*soundfile [filename]_UNCOMPRESSED_#.wav*\n");
printf("*****************************************\n");


do{ //read in another wav

int nSamples = NSAMPLES;
	//reset all




int n;


int isLoop = -1;
int bLoopFlag = 0;
int isStereo = 0;
int test=0;
int bitDepth = 0;


int byteBlock = 0;
char *tokenPtr;
long dataChunkSize;
long rawDataCount = 0; 
long headerCount = 0;

int i;

int ADPCMByteCount = 0;


	int dataHex = 0x61746164; //"atad"
	int fmtHex = 0x20746D66; //" mtf"
	int fmtJUNK = 0x4B4E554A;
int end = 0;
int blockCount = 0;

int tester = 0;


//reset state of coders
state.index = 0;
state.valprev = 0;
state2.index=0;
state2.valprev=0;

//
//printf("\n\nEnter a 16-bit .wav filename (-1 to quit)\n");
//scanf("%s",&filenameInput);
//
// end = atoi( filenameInput );
//
//
//if(end == -1)
//{
//	break;
//}

//while((fpin = fopen(filenameInput, "rb"))==NULL)
//	{
//		printf("Cannot open file \n");
//		printf("Enter .wav filename\n");
//		scanf("%s",&filenameInput);
//		
//	}


strcpy(filename1,argv[argCount-1]);
printf("%s",filename1);
strcpy(filenameInput,filename1);

if((fpin = fopen(filenameInput, "rb"))==NULL)
{
	printf("Cannot open file \n");
	exit(1);
}


	tokenPtr = strtok(filenameInput,"."); //split string into tokens


strcpy(filename2,filenameInput);
//strcpy(filename3,filenameInput);

//strcat( filename3, "_UNCOMPRESSED_" );

//printf("\nEnter the output version number(e.g. 1,2,3 etc...\n");
//scanf("%s",&filename4);

//strcat( filename3, filename4 );
//strcat(filename3,".wav");

//strcat( filename2, "_" );
//strcat( filename2, filename4 );
strcat( filename2, ".adpcm" );


//while(isLoop != 0 && isLoop != 1)
//{
//printf("\nIs the .wav a seamless loop? Enter 0 for no, 1 for yes\n");
//scanf("%d",&isLoop);
//bLoopFlag = isLoop;
//}

//LOOP VERSION, 1 is hard coded for seamless loops, 0 for oneshots
isLoop = 0;
bLoopFlag = 0;

	/* Process encoding argument, if any */
	
	





	
	if ((fpout = fopen(filename2, "wb"))==NULL)
	{
		printf("Cannot open file %s\n",filename2);
			}
	

		//UNCOMPRESSED FILE	//reformeddata needed for oputput wav
	/*if ((fpReformedData = fopen(filename3, "wb"))==NULL)
	{
		printf("Cannot open file \n");
			}*/


	//discard header chunks in RIFF (.wav) file and write to output
while(test != 1 && !feof(fpin))
{
	fread(&byteBlock,sizeof(int),1,fpin);
	//reformeddata needed for oputput wav
//	fwrite(&byteBlock,sizeof(int),1,fpReformedData);//reconstruct wav header

	headerCount++;	

	if(byteBlock == fmtHex)
	{	

	

			fread(&byteBlock,sizeof(int),1,fpin);//write chunk data size
			//reformeddata needed for oputput wav
	//	fwrite(&byteBlock,sizeof(int),1,fpReformedData);
		headerCount++;	

						fread(&chans,sizeof(short),1,fpin);//write comp code
						//reformeddata needed for oputput wav
	//	fwrite(&chans,sizeof(short),1,fpReformedData);//

		fread(&chans,sizeof(short),1,fpin);//write chans

		if(chans > 1)
		{
			printf("stereo");		
			isStereo = 1;
		}
		//reformeddata needed for oputput wav
		//fwrite(&chans,sizeof(short),1,fpReformedData);//
		
		headerCount++;	

				fread(&sr,sizeof(int),1,fpin);//write sr
				//reformeddata needed for oputput wav
	//	fwrite(&sr,sizeof(int),1,fpReformedData);//
		headerCount++;	
	
	}




	

	if(byteBlock == dataHex)
	{

	
		
		test = 1; //don't re-enter loop

		//get size of raw data so we can discard chunks after samples, (they're just junk chunks);
		fread(&dataChunkSize,sizeof(int),1,fpin);
		//reformeddata needed for oputput wav
	//	fwrite(&dataChunkSize,sizeof(int),1,fpReformedData);//reconstruct wav header
		headerCount++;
		
		break;
	}


}
//end read header

//make space for header in ADPCM file
for(i = 0;i<12;i++)
{
	int temp = 0;
	fwrite(&temp,sizeof(int),1,fpout);
	//fwrite(&temp,sizeof(int),1,fpout2);
}

if(isLoop)
{
		//need to put last 8 bytes as first	data samples
		//navigate file
		fseek( fpin, dataChunkSize,	SEEK_CUR);


		//get last 8 bytes if data
		fseek( fpin, -16, SEEK_CUR);

	
		//read	last 8 samples to sbufPre
			fread(&sbufPre,sizeof(char),16,fpin);//
			
	
			//write codes
				adpcm_coder(sbufPre, abufPre,	8, &state);


		ADPCMByteCount +=fwrite(&abufPre,sizeof(char),4,fpout);
	//	fwrite(&abufPre,sizeof(char),4,fpout2);


		//reset	poniter	to start of	data
		fseek( fpin,headerCount*4, SEEK_SET);//(headerCount*4)(sizeof(int))

} // end isLoop


//calculate total sample blocks
blockCount = dataChunkSize / (NSAMPLES*2); //total samples =blockcount*Nsamples*2

if(!isLoop) //other wise there'll be a click
blockCount = blockCount-2;



while (rawDataCount <= blockCount+1) //   n!=-1tester != -1
{//only read sample data,ignore junk
	rawDataCount+=1;

	if(rawDataCount == blockCount+1)
	{
	
		nSamples= (dataChunkSize - (blockCount * NSAMPLES*2)) / 4; //get raw smpls only
			 
	}	 

	
	n =	fread(&sbuf,sizeof(char),nSamples*2,fpin);//read in samples to sbuf

			adpcm_coder(sbuf, abuf,	n/2, &state);
//encodes go to abuf, can try joint stereo here, if stereo, t

		
	
		if(isStereo)
		{
			ADPCMByteCount += fwrite(&abuf,sizeof(char),n/4,fpout);
		}
		else
			ADPCMByteCount += fwrite(&abuf,sizeof(char),n/4,fpout);

	}

	
	

	
//****************************************
//Write ADPCM header
fseek(fpout,0,SEEK_SET);
	writeADHeader(fpout,chans,sr,bLoopFlag,ADPCMByteCount);
	


	

	printf("\nEncoding Complete");
	
	fclose(fpin);
	fclose(fpout);
	//fclose(fpout2);


	//DECODE AND REFORM DATA
	//**************************************************************************************
//////
//////rawDataCount = 0;
//////
//////
//////	
//////
//////	if ((fpin = fopen(filename2, "rb"))==NULL)
//////	{
//////		printf("Cannot open file \n");
//////			}
//////
//////	
//////	//discard ADPCM header
//////fseek(fpin,48,SEEK_SET);
////////end discard ADPCM header
//////
//////
//////
////////start read/decode adpcm file
//////    while(1) 
//////	{
//////
//////	
//////		if(isLoop)
//////	{
//////		fread(&abufPost,sizeof(char),4,fpin); //chop of first 8 samples (4 byte codes)
//////		adpcm_decoder(abufPost, sbufPost, 8, &state2);//warm up decoder
//////		isLoop = 0; //don't re-enter after first 8 bytes
//////		
//////	}
//////	
//////		n =	fread(&abuf2,sizeof(char),NSAMPLES/2,fpin);
//////
//////
//////			if ( n < 0 )
//////			{
//////				perror("input file");
//////				exit(1);
//////			}
//////
//////			if ( n == 0	)
//////			{
//////							
//////				break;//end while loop
//////			}
//////
//////
//////
//////	adpcm_decoder(abuf2, sbuf2, n*2, &state2);
//////			
//////
//////
//////
//////
//////			fwrite(&sbuf2,sizeof(char),n*4,fpReformedData);
//////	
//////	}
//////		/* Read and unpack input codes and process them */
//////		//read write decode
//////	printf("\nDecoding Complete");
//////
//////
//////
//////
//////
//////	fclose(fpin);
//////	
//////	fclose(fpReformedData);

	argCount--;

	}while(argCount > 1);//end != -1

}//end main
