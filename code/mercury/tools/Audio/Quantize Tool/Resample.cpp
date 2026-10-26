#include "stdafx.h"

#include "resample.h"

// THIS CODE IS NOW REDUNDANT. WE NOW USE LIBSAMPLERATE/SECRET RABBIT CODE INSTEAD

#ifdef DONKEY_KONG_ATE_MY_WIFE

//#define DEBUG 1
#define Npc 4096
#define IzeroEPSILON 1E-21               /* Max error acceptable in Izero */
const float PI = 3.14159265358979232846f;

void Resample(BYTE* pDataIn, DWORD dwSizeIn, BYTE* pDataOut, DWORD dwSizeOut);

void PitchShift(BYTE* pDataIn, DWORD dwSizeIn, BYTE* pDataOut, DWORD dwSizeOut, char uChannels)
{
	if (uChannels == 1)
		Resample(pDataIn, dwSizeIn, pDataOut, dwSizeOut);
	else
	{
		
		BYTE* pSrc;
		BYTE* pDst;
		BYTE* pTmpDataIn=0;
		BYTE* pTmpDataOut=0;
		DWORD dwTmpSizeIn=dwSizeIn/2;
		DWORD dwTmpSizeOut=dwSizeOut/2;

		// allocate a src and dest buffer big enough for a single channel
		pTmpDataIn = (BYTE*)malloc(dwTmpSizeIn * sizeof(BYTE));
		if (pTmpDataIn==NULL)
		{
			printf("\nFAILED to allocate single channel buffer of %d bytes\n", dwTmpSizeIn);
			return;
		}

		pTmpDataOut = (BYTE*)malloc(dwTmpSizeOut * sizeof(BYTE));
		if (pTmpDataOut==NULL)
		{
			printf("\nFAILED to allocate single channel buffer of %d bytes\n", dwTmpSizeOut);
			return;
		}

		//printf("done\n");

		memset(pDataOut, 0, dwSizeOut);

		for (int ch=0; ch<2; ch++)
		{
			memset(pTmpDataOut, 0, dwTmpSizeOut);
			memset(pTmpDataIn, 0, dwTmpSizeIn);

			// copy the (interleaved) data into a contiguous block
			pSrc = pDataIn+ch*2;
			pDst = pTmpDataIn;
			
			while (pSrc < pDataIn+dwSizeIn)
			{
				*pDst = *pSrc;
				*(pDst+1) = *(pSrc+1);

				pSrc+=4;
				pDst+=2;
			}

			Resample(pTmpDataIn, dwTmpSizeIn, pTmpDataOut, dwTmpSizeOut);

			// copy resampled data back into the output interleaved buffer
			pSrc = pTmpDataOut;
			pDst = pDataOut+ch*2;
			int bytesCopied=0;
			
			while (pDst < pDataOut+dwSizeOut)
			{
				*pDst = *pSrc;
				*(pDst+1) = *(pSrc+1);

				bytesCopied+=2;
				pSrc+=2;
				pDst+=4;
			}
		}

		free(pTmpDataOut);
		free(pTmpDataIn);
	}
}

void Resample(BYTE* pDataIn, DWORD dwSizeIn, BYTE* pDataOut, DWORD dwSizeOut)
{
	double factor = (float(dwSizeOut) / float(dwSizeIn));

	ResampleData* handle = (ResampleData*)(resample_open(1, factor, factor));
	int bufferpos = 0;
	int outcount = 0;
	int out = 0;
	DWORD srcpos = 0;
	int srcused = 0;

	dwSizeIn /= 2;
	dwSizeOut /= 2;

	while(srcpos < dwSizeIn)
	{
		int srcBlock = dwSizeIn - srcpos;

		int lastFlag = (srcBlock == dwSizeIn - srcpos);

		out = resample_process(handle, factor,
							(short*)(&(pDataIn[srcpos])), srcBlock,
							lastFlag, &srcused,
							(short*)(&(pDataOut[out])), min(dwSizeOut - out, dwSizeOut));

		srcpos += srcused;
	}

	resample_close(handle);
}

void *resample_open(int highQuality, double minFactor, double maxFactor)
{	
   double *Imp64;
   double Rolloff, Beta;
   ResampleData *hp;
   UWORD   Xoff_min, Xoff_max;
   int i;

   /* Just exit if we get invalid factors */
   if (minFactor <= 0.0 || maxFactor <= 0.0 || maxFactor < minFactor) {
      #if DEBUG
      fprintf(stderr,
              "libresample: "
              "minFactor and maxFactor must be positive real numbers,\n"
              "and maxFactor should be larger than minFactor.\n");
      #endif
      return 0;
   }

   hp = (ResampleData *)malloc(sizeof(ResampleData));

   hp->minFactor = minFactor;
   hp->maxFactor = maxFactor;
 
   if (highQuality)
      hp->wNumFilterCoeffsMultiplier = 91;//35;	// make this big else we often get a high pitched wibble -jkc
   else
      hp->wNumFilterCoeffsMultiplier = 11;

   hp->LpScl = 1.0;
   hp->wFilterWingLength = Npc*(hp->wNumFilterCoeffsMultiplier-1)/2; /* # of filter coeffs in right wing */

   Rolloff = 0.90;
   Beta = 6;

   Imp64 = (double *)malloc(hp->wFilterWingLength * sizeof(double));

   lrsLpFilter(Imp64, hp->wFilterWingLength, 0.5*Rolloff, Beta, Npc);

   hp->fImpulseResponse = (double *)malloc(hp->wFilterWingLength * sizeof(double));
   hp->fImpulseResponseDeltas = (double *)malloc(hp->wFilterWingLength * sizeof(double));
   for(i=0; i<hp->wFilterWingLength; i++)
      hp->fImpulseResponse[i] = Imp64[i];

   /* Storing deltas in fImpulseResponseDeltas makes linear interpolation
      of the filter coefficients faster */
   for (i=0; i<hp->wFilterWingLength-1; i++)
      hp->fImpulseResponseDeltas[i] = hp->fImpulseResponse[i+1] - hp->fImpulseResponse[i];

   /* Last coeff. not interpolated */
   hp->fImpulseResponseDeltas[hp->wFilterWingLength-1] = - hp->fImpulseResponse[hp->wFilterWingLength-1];

   free(Imp64);

   /* Calc reach of LP filter wing (plus some creeping room) */
   Xoff_min = UWORD((double(hp->wNumFilterCoeffsMultiplier+1) / 2.0f) * max(1.0f, 1.0f / minFactor)) + 10;
   Xoff_max = UWORD((double(hp->wNumFilterCoeffsMultiplier+1) / 2.0f) * max(1.0f, 1.0f / maxFactor)) + 10;
   hp->wUnprocessedBufferOffset = max(Xoff_min, Xoff_max);

   /* Make the inBuffer size at least 4096, but larger if necessary
      in order to store the minimum reach of the LP filter and then some.
      Then allocate the buffer an extra wUnprocessedBufferOffset larger so that
      we can zero-pad up to wUnprocessedBufferOffset zeros at the end when we reach the
      end of the input samples. */
   hp->wUnprocessedBufferSize = max(2*hp->wUnprocessedBufferOffset+10, 4096);
   hp->pUnprocessedBuffer = (double *)malloc((hp->wUnprocessedBufferSize + hp->wUnprocessedBufferOffset) * sizeof(double));
   hp->wUnprocessedBufferPosition = hp->wUnprocessedBufferOffset;
   hp->Xread = hp->wUnprocessedBufferOffset;
   
   /* Need wUnprocessedBufferOffset zeros at begining of pUnprocessedBuffer buffer */
   for(i=0; i<hp->wUnprocessedBufferOffset; i++)
      hp->pUnprocessedBuffer[i]=0;

   /* Make the outBuffer long enough to hold the entire processed
      output of one inBuffer */
   hp->wProcessedBufferSize = (int)(((double)hp->wUnprocessedBufferSize)*maxFactor+2.0);
   hp->pProcessedBuffer = (double *)malloc(hp->wProcessedBufferSize * sizeof(double));
   hp->wProcessedBufferPosition = 0;

   hp->Time = (double)hp->wUnprocessedBufferOffset; /* Current-time pointer for converter */
   
   return (void *)hp;
}

int resample_get_filter_width(const void   *handle)
{
   const ResampleData *hp = (const ResampleData *)handle;
   return hp->wUnprocessedBufferOffset;
}


int resample_process(void   *handle,
                     double  factor,
                     short  *inBuffer,
                     int     inBufferLen,
                     int     lastFlag,
                     int    *inBufferUsed, /* output param */
                     short  *outBuffer,
                     int     outBufferLen)
{
   ResampleData *hp = (ResampleData *)handle;
   double  *fImpulseResponse = hp->fImpulseResponse;
   double  *fImpulseResponseDeltas = hp->fImpulseResponseDeltas;
   double  LpScl = hp->LpScl;
   UWORD  wFilterWingLength = hp->wFilterWingLength;
   BOOL interpFilt = FALSE; /* TRUE means interpolate filter coeffs */
   int outSampleCount;
   UWORD Nout, Ncreep, Nreuse;
   int Nx;
   int i, len;

   #if DEBUG
   fprintf(stderr, "resample_process: in=%d, out=%d lastFlag=%d\n",
           inBufferLen, outBufferLen, lastFlag);
   #endif

   /* Initialize inBufferUsed and outSampleCount to 0 */
   *inBufferUsed = 0;
   outSampleCount = 0;

   if (factor < hp->minFactor || factor > hp->maxFactor) {
      #if DEBUG
      fprintf(stderr,
              "libresample: factor %f is not between "
              "minFactor=%f and maxFactor=%f",
              factor, hp->minFactor, hp->maxFactor);
      #endif
      return -1;
   }

   /* Start by copying any samples still in the pProcessedBuffer buffer to the output
      buffer */
   if (hp->wProcessedBufferPosition && (outBufferLen-outSampleCount)>0) {
      len = min(outBufferLen-outSampleCount, hp->wProcessedBufferPosition);
      for(i=0; i<len; i++)
         outBuffer[outSampleCount+i] = short(hp->pProcessedBuffer[i]);
      outSampleCount += len;
      for(i=0; i<hp->wProcessedBufferPosition-len; i++)
         hp->pProcessedBuffer[i] = hp->pProcessedBuffer[i+len];
      hp->wProcessedBufferPosition -= len;
   }

   /* If there are still output samples left, return now - we need
      the full output buffer available to us... */
   if (hp->wProcessedBufferPosition)
      return outSampleCount;

   /* Account for increased filter gain when using factors less than 1 */
   if (factor < 1)
      LpScl = LpScl*factor;

   for(;;) {

      /* This is the maximum number of samples we can process
         per loop iteration */

      #ifdef DEBUG
      printf("wUnprocessedBufferSize: %d wUnprocessedBufferOffset: %d Xread: %d wUnprocessedBufferPosition: %d lastFlag: %d\n",
             hp->wUnprocessedBufferSize, hp->wUnprocessedBufferOffset, hp->Xread, hp->wUnprocessedBufferPosition, lastFlag);
      #endif

      /* Copy as many samples as we can from the input buffer into pUnprocessedBuffer */
      len = hp->wUnprocessedBufferSize - hp->Xread;

      if (len >= (inBufferLen - (*inBufferUsed)))
         len = (inBufferLen - (*inBufferUsed));

      for(i=0; i<len; i++)
         hp->pUnprocessedBuffer[hp->Xread + i] = (double)(inBuffer[(*inBufferUsed) + i]);

      *inBufferUsed += len;
      hp->Xread += len;

      if (lastFlag && (*inBufferUsed == inBufferLen)) {
         /* If these are the last samples, zero-pad the
            end of the input buffer and make sure we process
            all the way to the end */
         Nx = hp->Xread - hp->wUnprocessedBufferOffset;
         for(i=0; i<hp->wUnprocessedBufferOffset; i++)
            hp->pUnprocessedBuffer[hp->Xread + i] = 0;
      }
      else
         Nx = hp->Xread - 2 * hp->wUnprocessedBufferOffset;

      #ifdef DEBUG
      fprintf(stderr, "new len=%d Nx=%d\n", len, Nx);
      #endif

      if (Nx <= 0)
         break;

      /* Resample stuff in input buffer */
      if (factor >= 1) {      /* SrcUp() is faster if we can use it */
         Nout = lrsSrcUp(hp->pUnprocessedBuffer, hp->pProcessedBuffer, factor, &hp->Time, Nx,
                         wFilterWingLength, LpScl, fImpulseResponse, fImpulseResponseDeltas, interpFilt);
      }
      else {
         Nout = lrsSrcUD(hp->pUnprocessedBuffer, hp->pProcessedBuffer, factor, &hp->Time, Nx,
                         wFilterWingLength, LpScl, fImpulseResponse, fImpulseResponseDeltas, interpFilt);
      }

      #ifdef DEBUG
      printf("Nout: %d\n", Nout);
      #endif
      
      hp->Time -= Nx;         /* Move converter Nx samples back in time */
      hp->wUnprocessedBufferPosition += Nx;           /* Advance by number of samples processed */

      /* Calc time accumulation in Time */
      Ncreep = (int)(hp->Time) - hp->wUnprocessedBufferOffset;
      if (Ncreep) {
         hp->Time -= Ncreep;  /* Remove time accumulation */
         hp->wUnprocessedBufferPosition += Ncreep;    /* and add it to read pointer */
      }

      /* Copy part of input signal that must be re-used */
      Nreuse = hp->Xread - (hp->wUnprocessedBufferPosition - hp->wUnprocessedBufferOffset);

      for (i=0; i<Nreuse; i++)
         hp->pUnprocessedBuffer[i] = hp->pUnprocessedBuffer[i + (hp->wUnprocessedBufferPosition - hp->wUnprocessedBufferOffset)];

      #ifdef DEBUG
      printf("New Xread=%d\n", Nreuse);
      #endif

      hp->Xread = Nreuse;  /* Pos in input buff to read new data into */
      hp->wUnprocessedBufferPosition = hp->wUnprocessedBufferOffset;
      
      /* Check to see if output buff overflowed (shouldn't happen!) */
      if (Nout > hp->wProcessedBufferSize) {
         #ifdef DEBUG
         printf("Nout: %d wProcessedBufferSize: %d\n", Nout, hp->wProcessedBufferSize);
         #endif
         fprintf(stderr, "libresample: Output array overflow!\n");
         return -1;
      }

      hp->wProcessedBufferPosition = Nout;

      /* Copy as many samples as possible to the output buffer */
      if (hp->wProcessedBufferPosition && (outBufferLen-outSampleCount)>0) {
         len = min(outBufferLen-outSampleCount, hp->wProcessedBufferPosition);
         for(i=0; i<len; i++)
            outBuffer[outSampleCount+i] = short(hp->pProcessedBuffer[i]);
         outSampleCount += len;
         for(i=0; i<hp->wProcessedBufferPosition-len; i++)
            hp->pProcessedBuffer[i] = hp->pProcessedBuffer[i+len];
         hp->wProcessedBufferPosition -= len;
      }

      /* If there are still output samples left, return now,
         since we need the full output buffer available */
      if (hp->wProcessedBufferPosition)
         break;
   }

   return outSampleCount;
}


void resample_close(void *handle)
{
   ResampleData *hp = (ResampleData *)handle;
   free(hp->pUnprocessedBuffer);
   free(hp->pProcessedBuffer);
   free(hp->fImpulseResponse);
   free(hp->fImpulseResponseDeltas);
   free(hp);
}


static double Izero(double x)
{
   double sum, u, halfx, temp;
   int n;

   sum = u = n = 1;
   halfx = x/2.0;
   do {
      temp = halfx/(double)n;
      n += 1;
      temp *= temp;
      u *= temp;
      sum += u;
   } while (u >= IzeroEPSILON*sum);
   return(sum);
}


void lrsLpFilter(double c[], int N, double frq, double Beta, int Num)
{
   double IBeta, temp, temp1, inm1;
   int i;

   /* Calculate ideal lowpass filter impulse response coefficients: */
   c[0] = 2.0*frq;
   for (i=1; i<N; i++) {
      temp = PI*(double)i/(double)Num;
      c[i] = sinf(float(2.0f*temp*frq)) /temp; /* Analog sinc function, cutoff = frq */
   }

   /* 
    * Calculate and Apply Kaiser window to ideal lowpass filter.
    * Note: last window value is IBeta which is NOT zero.
    * You're supposed to really truncate the window here, not ramp
    * it to zero. This helps reduce the first sidelobe. 
    */
   IBeta = 1.0/Izero(Beta);
   inm1 = 1.0/((double)(N-1));
   for (i=1; i<N; i++) {
      temp = (double)i * inm1;
      temp1 = 1.0 - temp*temp;
      temp1 = (temp1<0? 0: temp1); /* make sure it's not negative since
                                      we're taking the square root - this
                                      happens on Pentium 4's due to tiny
                                      roundoff errors */
      c[i] *= Izero(Beta*sqrt(temp1)) * IBeta;
   }
}


double lrsFilterUp(double fImpulseResponse[],  /* impulse response */
                  double fImpulseResponseDeltas[], /* impulse response deltas */
                  UWORD wFilterWingLength,  /* len of one wing of filter */
                  BOOL Interp,  /* Interpolate coefs using deltas? */
                  double *p_fCurrentSample,    /* Current sample */
                  double dfPhase,    /* Phase */
                  int Inc)    /* increment (1 for right wing or -1 for left) */
{
   double *Hp, *Hdp = NULL, *End;
   double a = 0;
   double fOutputValue, t;

   dfPhase *= Npc; /* Npc is number of values per 1/delta in impulse response */
   
   fOutputValue = 0.0; /* The output value */
   Hp = &fImpulseResponse[(int)dfPhase];
   End = &fImpulseResponse[wFilterWingLength];
   if (Interp) {
      Hdp = &fImpulseResponseDeltas[(int)dfPhase];
      a = dfPhase - floor(dfPhase); /* fractional part of Phase */
   }

   if (Inc == 1)		/* If doing right wing...              */
   {				      /* ...drop extra coeff, so when dfPhase is  */
      End--;			/*    0.5, we don't do too many mult's */
      if (dfPhase == 0)		/* If the phase is zero...           */
      {			         /* ...then we've already skipped the */
         Hp += Npc;		/*    first sample, so we must also  */
         Hdp += Npc;		/*    skip ahead in fImpulseResponse[] and fImpulseResponseDeltas[] */
      }
   }

   if (Interp)
      while (Hp < End) {
         t = *Hp;		/* Get filter coeff */
         t += (*Hdp)*a; /* t is now interp'd filter coeff */
         Hdp += Npc;		/* Filter coeff differences step */
         t *= *p_fCurrentSample;		/* Mult coeff by input sample */
         fOutputValue += t;			/* The filter output */
         Hp += Npc;		/* Filter coeff step */
         p_fCurrentSample += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      } 
   else 
      while (Hp < End) {
         t = *Hp;		/* Get filter coeff */
         t *= *p_fCurrentSample;		/* Mult coeff by input sample */
         fOutputValue += t;			/* The filter output */
         Hp += Npc;		/* Filter coeff step */
         p_fCurrentSample += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }
   
   return fOutputValue;
}


double lrsFilterUD(double fImpulseResponse[],  /* impulse response */
                  double fImpulseResponseDeltas[], /* impulse response deltas */
                  UWORD wFilterWingLength,  /* len of one wing of filter */
                  BOOL Interp,  /* Interpolate coefs using deltas? */
                  double *wUnprocessedBufferPosition,    /* Current sample */
                  double dfPhase,    /* Phase */
                  int Inc,    /* increment (1 for right wing or -1 for left) */
                  double dhb) /* filter sampling period */
{
   double a;
   double *Hp, *Hdp, *End;
   double v, t;
   double Ho;
    
   v = 0.0; /* The output value */
   Ho = dfPhase*dhb;
   End = &fImpulseResponse[wFilterWingLength];
   if (Inc == 1)		/* If doing right wing...              */
   {				      /* ...drop extra coeff, so when dfPhase is  */
      End--;			/*    0.5, we don't do too many mult's */
      if (dfPhase == 0)		/* If the phase is zero...           */
         Ho += dhb;		/* ...then we've already skipped the */
   }				         /*    first sample, so we must also  */
                        /*    skip ahead in fImpulseResponse[] and fImpulseResponseDeltas[] */

   if (Interp)
      while ((Hp = &fImpulseResponse[(int)Ho]) < End) {
         t = *Hp;		/* Get IR sample */
         Hdp = &fImpulseResponseDeltas[(int)Ho];  /* get interp bits from diff table*/
         a = Ho - floor(Ho);	  /* a is logically between 0 and 1 */
         t += (*Hdp)*a; /* t is now interp'd filter coeff */
         t *= *wUnprocessedBufferPosition;		/* Mult coeff by input sample */
         v += t;			/* The filter output */
         Ho += dhb;		/* IR step */
         wUnprocessedBufferPosition += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }
   else 
      while ((Hp = &fImpulseResponse[(int)Ho]) < End) {
         t = *Hp;		/* Get IR sample */
         t *= *wUnprocessedBufferPosition;		/* Mult coeff by input sample */
         v += t;			/* The filter output */
         Ho += dhb;		/* IR step */
         wUnprocessedBufferPosition += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }

   return v;
}


int lrsSrcUp(double pUnprocessedBuffer[],
             double pProcessedBuffer[],
             double factor,
             double *TimePtr,
             UWORD Nx,
             UWORD wFilterWingLength,
             double LpScl,
             double fImpulseResponse[],
             double fImpulseResponseDeltas[],
             BOOL Interp)
{
    double *wUnprocessedBufferPosition, *Ystart;
    double v;
    
    double CurrentTime = *TimePtr;
    double dt;                 /* Step through input signal */ 
    double endTime;            /* When Time reaches EndTime, return to user */
    
    dt = 1.0/factor;           /* Output sampling period */
    
    Ystart = pProcessedBuffer;
    endTime = CurrentTime + Nx;
    while (CurrentTime < endTime)
    {
        double LeftPhase = CurrentTime-floor(CurrentTime);
        double RightPhase = 1.0 - LeftPhase;

        wUnprocessedBufferPosition = &pUnprocessedBuffer[(int)CurrentTime]; /* Ptr to current input sample */
        /* Perform left-wing inner product */
        v = lrsFilterUp(fImpulseResponse, fImpulseResponseDeltas, wFilterWingLength, Interp, wUnprocessedBufferPosition,
                        LeftPhase, -1);
        /* Perform right-wing inner product */
        v += lrsFilterUp(fImpulseResponse, fImpulseResponseDeltas, wFilterWingLength, Interp, wUnprocessedBufferPosition+1, 
                         RightPhase, 1);

        v *= LpScl;   /* Normalize for unity filter gain */

        *pProcessedBuffer++ = v;               /* Deposit output */
        CurrentTime += dt;      /* Move to next sample by time increment */
    }

    *TimePtr = CurrentTime;
    return (int)(pProcessedBuffer - Ystart);        /* Return the number of output samples */
}

/* Sampling rate conversion subroutine */

int lrsSrcUD(double pUnprocessedBuffer[],
             double pProcessedBuffer[],
             double factor,
             double *TimePtr,
             UWORD Nx,
             UWORD wFilterWingLength,
             double LpScl,
             double fImpulseResponse[],
             double fImpulseResponseDeltas[],
             BOOL Interp)
{
    double *wUnprocessedBufferPosition, *Ystart;
    double v;

    double CurrentTime = (*TimePtr);
    double dh;                 /* Step through filter impulse response */
    double dt;                 /* Step through input signal */
    double endTime;            /* When Time reaches EndTime, return to user */
    
    dt = 1.0/factor;            /* Output sampling period */
    
    dh = min(Npc, factor*Npc);  /* Filter sampling period */
    
    Ystart = pProcessedBuffer;
    endTime = CurrentTime + Nx;
    while (CurrentTime < endTime)
    {
        double LeftPhase = CurrentTime-floor(CurrentTime);
        double RightPhase = 1.0 - LeftPhase;

        wUnprocessedBufferPosition = &pUnprocessedBuffer[(int)CurrentTime];     /* Ptr to current input sample */
        /* Perform left-wing inner product */
        v = lrsFilterUD(fImpulseResponse, fImpulseResponseDeltas, wFilterWingLength, Interp, wUnprocessedBufferPosition,
                        LeftPhase, -1, dh);
        /* Perform right-wing inner product */
        v += lrsFilterUD(fImpulseResponse, fImpulseResponseDeltas, wFilterWingLength, Interp, wUnprocessedBufferPosition+1, 
                         RightPhase, 1, dh);

        v *= LpScl;   /* Normalize for unity filter gain */
        *pProcessedBuffer++ = v;               /* Deposit output */
        
        CurrentTime += dt;      /* Move to next sample by time increment */
    }

    *TimePtr = CurrentTime;
    return (int)(pProcessedBuffer - Ystart);        /* Return the number of output samples */
}
#endif