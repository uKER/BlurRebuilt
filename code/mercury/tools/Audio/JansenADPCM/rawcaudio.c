/* testc - Test adpcm coder */

#include "adpcm.h"
#include <stdio.h>

struct adpcm_state state;

#define NSAMPLES 1000

char	abuf[NSAMPLES/2];
short	sbuf[NSAMPLES];

main() {


	//!!!!!!open I/O files here
    int n;

//	strcpy(filenameInput,"tests\\skyline.adpcm");
//strcpy(filenameOutput,"tests\\skyline_UNCOMPRESSED.raw");

    while(1) {

		//!!!!!!!!read in raw samples (short) here, place in sbuf
	//n = read(0, sbuf, NSAMPLES*2);
	n = read("tests\\skyline.raw", sbuf, NSAMPLES*2);
	if ( n < 0 ) {
	    perror("input file");
	    exit(1);
	}
	if ( n == 0 ) break;
	adpcm_coder(sbuf, abuf, n/2, &state);

	//write(1, abuf, n/4);
	write("tests\\skyline.adpcm", abuf, n/4);
	//!!!!!!write abuf codes (char) to outfile.adpcm
    }
    fprintf(stderr, "Final valprev=%d, index=%d\n",
	    state.valprev, state.index);

	//!!!!!!close files
	getchar();
    exit(0);
}
