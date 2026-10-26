/* testd - Test adpcm decoder */

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
int count=0;
short byteBlock = 0; //
strcpy(filenameInput,"tests\\skylineloopExtra_1_8.adpcm");
strcpy(filenameOutput,"tests\\skylineloopExtra_1_8_UNCOMPRESSED.raw");


printf("\n%s",filenameInput);
if ((fpin = fopen(filenameInput, "rb"))==NULL)
	{
		printf("Cannot open infile \n");
	}

	


if ((fpout = fopen(filenameOutput, "wb"))==NULL)
	{
		printf("Cannot open outfile  \n");
	}





    while(1) 
	{
	
		//n = read(0, abuf, NSAMPLES/2);
		n =	fread(&abuf,sizeof(char),NSAMPLES/2,fpin);


			if ( n < 0 )
			{
				perror("input file");
				exit(1);
			}

			if ( n == 0	)
			{
				printf("\nEOF!");				break;
			}

	count++; //count bytes going in

	//adpcm_decoder(abuf, sbuf, n*2, &state);
	adpcm_decoder(abuf, sbuf, n*2, &state);
			


	//write(1, sbuf, n*4);
	//fwrite(&sbuf[i],sizeof(short),1,fpout);//reconstruct wav header
			fwrite(&sbuf,sizeof(char),n*4,fpout);//reconstruct wav header


    }



    fprintf(stderr, "Final valprev=%d, index=%d\n",
	    state.valprev, state.index);


printf("\n%d",count);
    	
	fclose(fpin);
	fclose(fpout);
	getchar();
    exit(0);
}
