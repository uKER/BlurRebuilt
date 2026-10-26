/* testc - Test adpcm coder */

#include "adpcm.h"
#include <stdio.h>

struct adpcm_state state;

#define NSAMPLES 1000

char	abuf[NSAMPLES/2];
short	sbuf[NSAMPLES];

main() {
		FILE *fpin,*fpout;


char filenameInput[500]; //temp filename buffer
char filenameOutput[500];
int n;
int i = 0;
int count = 0;
short byteBlock = 0; //
strcpy(filenameInput,"tests\\skyline.raw");
strcpy(filenameOutput,"tests\\skyline.adpcm");


printf("\n%s",filenameInput);
if ((fpin = fopen(filenameInput, "rb"))==NULL)
	{
		printf("Cannot open infile \n");
	}

	


if ((fpout = fopen(filenameOutput, "wb"))==NULL)
	{
		printf("Cannot open outfile  \n");
	}
	

		
	
	
    
//!!!!!!open I/O files here
    while(1) 
	{

			//for(i=0;i<1000;i++)
		//	{
				
				
		//	}

			//	for(i=0;i<1000;i++)
			//{
			//	printf("\n%d",sbuf[i]);
		//	}


			//!!!!!!!!read in raw samples (short) here,	place in sbuf
			//!!!!!try sample by sample
		//n	= read(0, sbuf,	NSAMPLES*2);
		n =	fread(&sbuf,sizeof(char),NSAMPLES*2,fpin);

			if ( n < 0 ) {
				perror("input file error");
				exit(1);
			}
			if ( n == 0	)
			{
				printf("\nEOF!");
				break;
			}
			
			count++;//count bytes going in


		//adpcm_coder(sbuf, abuf,	n/2, &state);
			adpcm_coder(sbuf, abuf,	n/2, &state);

		//write(1, abuf, n/4);
		//fwrite(&abuf[i],sizeof(char),n/4,fpout);//reconstruct wav header
		fwrite(&abuf,sizeof(char),n/4,fpout);//reconstruct wav header

		//!!!!!!write abuf codes (char)	to outfile.adpcm
    }


    fprintf(stderr, "Final valprev=%d, index=%d\n",
	    state.valprev, state.index);

	//!!!!!!close files
	printf("\n%d",count);
	
	fclose(fpin);
	fclose(fpout);
	getchar();
    exit(0);
}
