
#define BATCH_SIZE  64


class adpcm64
{
	unsigned int mNrChannels;
	unsigned int mNrSamples;

	float pack_prev_1L;
    float pack_prev_2L;
	float pack_prev_1R;
    float pack_prev_2R;

	float predict_prev_1L;
    float predict_prev_2L;
	float predict_prev_1R;
    float predict_prev_2R;
	unsigned char decodeBuffer[ ((BATCH_SIZE * 2) >> 1) + 2];
	int decodeBufferCount;

	unsigned int batchCount;

	int mfputc(char c, FILE *stream);
	int mfgetc(FILE *stream);
	int FileLength( FILE *fp );
	int Depack( FILE *compressed, FILE *pcm );
	void find_predict( float *samples, int *predict_nr, int *shift_factor, float *predictPrev1, float *predictPrev2 );
	void pack( float *samples, short *four_bit, int predict_nr, int shift_factor, float *packPrev1, float *packPrev2 );
	void test();

public:	
	adpcm64();
	void _StartPack( int nrSampleFrames, int nrChannels );
	int _PackWrite( short *inData, int nrInFrames, unsigned char *outData, int nrOutBytes );
	int _PackRead( unsigned char *inData, int nrInBytes, short *outData, int nrOutFrames );
	void _PackClose();

	int GetNrChannels() { return mNrChannels; }
	int GetNrSamples() { return mNrSamples; }
};

