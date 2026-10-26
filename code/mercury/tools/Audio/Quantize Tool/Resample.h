typedef struct {
   double  *fImpulseResponse;
   double  *fImpulseResponseDeltas;
   double   LpScl;
   UWORD   wNumFilterCoeffsMultiplier;
   UWORD   wFilterWingLength;
   double  minFactor;
   double  maxFactor;
   UWORD   wUnprocessedBufferSize;
   double  *pUnprocessedBuffer;
   UWORD   wUnprocessedBufferPosition; /* Current "now"-sample pointer for input */
   UWORD   Xread; /* Position to put new samples */
   UWORD   wUnprocessedBufferOffset;
   UWORD   wProcessedBufferSize;
   double  *pProcessedBuffer;
   UWORD   wProcessedBufferPosition;
   double  Time;
} ResampleData;

void PitchShift(BYTE* pDataIn, DWORD dwSizeIn, BYTE* pDataOut, DWORD dwSizeOut, char uChannels);

void *resample_open(int highQuality, double minFactor, double maxFactor);
int resample_process(void   *handle,
                     double  factor,
                     short  *inBuffer,
                     int     inBufferLen,
                     int     lastFlag,
                     int    *inBufferUsed, /* output param */
                     short  *outBuffer,
                     int     outBufferLen);

void resample_close(void *handle);

int resample_get_filter_width(const void   *handle);

double lrsFilterUp(double		fImpulseResponse[],
				  double		fImpulseResponseDeltas[],
				  UWORD		wFilterWingLength,
				  BOOL		Interp,
				  double		*wUnprocessedBufferPosition,
				  double	dfPhase,
				  int		Inc);

double lrsFilterUD(double		fImpulseResponse[],
				  double		fImpulseResponseDeltas[],
				  UWORD		wFilterWingLength,
				  BOOL		Interp,
                  double		*wUnprocessedBufferPosition,
				  double	dfPhase,
				  int		Inc,
				  double	dhb);

void lrsLpFilter(double c[], int N, double frq, double Beta, int Num);

int lrsSrcUp(double pUnprocessedBuffer[],
             double pProcessedBuffer[],
             double factor,
             double *TimePtr,
             UWORD Nx,
             UWORD wFilterWingLength,
             double LpScl,
             double fImpulseResponse[],
             double fImpulseResponseDeltas[],
             BOOL Interp);

int lrsSrcUD(double pUnprocessedBuffer[],
             double pProcessedBuffer[],
             double factor,
             double *TimePtr,
             UWORD Nx,
             UWORD wFilterWingLength,
             double LpScl,
             double fImpulseResponse[],
             double fImpulseResponseDeltas[],
             BOOL Interp);