#include <stdafx.h>
#include "adpcm64.h"


adpcm64::adpcm64()
{
}

int adpcm64::mfputc(char c, FILE *stream)
{
        return fputc( c, stream );
}
int adpcm64::mfgetc(FILE *stream)
{
        return fgetc( stream );
}

int adpcm64::FileLength( FILE *fp )
{
        int oldpos = ftell( fp );
        int length;
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, oldpos, SEEK_SET);
        return length;

}


void adpcm64::find_predict( float *samples, int *predict_nr, int *shift_factor, float *predictPrev1, float *predictPrev2 )
{
	const float fp[5][2] = { { 0.0, 0.0 }, {  -60.0 / 64.0, 0.0 }, { -115.0 / 64.0, 52.0 / 64.0 }, {  -98.0 / 64.0, 55.0 / 64.0 }, { -122.0 / 64.0, 60.0 / 64.0 } };
    int i, j;
    float buffer[BATCH_SIZE][5];
    float min = 1e10;
    float max[5];
    float ds;
    int min2;
    int shift_mask;
    float s_0, s_1, s_2;
    for ( i = 0; i < 5; i++ )
    {
        max[i] = 0.0;
        s_1 = *predictPrev1;
        s_2 = *predictPrev2;
        for ( j = 0; j < BATCH_SIZE; j ++ )
        {
            s_0 = samples[j];                      // s[t-0]
            if ( s_0 > 30719.0 )
                s_0 = 30719.0;
            if ( s_0 < - 30720.0 )
                s_0 = -30720.0;

            ds = s_0 + s_1 * fp[i][0] + s_2 * fp[i][1];
            buffer[j][i] = ds;
            if ( fabs( ds ) > max[i] )
                max[i] = fabs( ds );
                s_2 = s_1;                                  // new s[t-2]
                s_1 = s_0;                                  // new s[t-1]
        }
        if ( max[i] < min )
        {
            min = max[i];
            *predict_nr = i;
        }

        if ( min <= 7 )
        {
            *predict_nr = 0;
            break;
        }
    }
    *predictPrev1 = s_1;
    *predictPrev2 = s_2;
    for ( i = 0; i < BATCH_SIZE; i++ )
    {
        samples[i] = buffer[i][*predict_nr];
    }
    min2 = ( int ) min;
    shift_mask = 0x4000;
    *shift_factor = 0;

    while( *shift_factor < 12 )
    {
        if ( shift_mask  & ( min2 + ( shift_mask >> 3 ) ) )
            break;
        (*shift_factor)++;
        shift_mask = shift_mask >> 1;
    }
}

void adpcm64::pack( float *samples, short *four_bit, int predict_nr, int shift_factor, float *packPrev1, float *packPrev2 )
{
	const float fp[5][2] = { { 0.0, 0.0 }, {  -60.0 / 64.0, 0.0 }, { -115.0 / 64.0, 52.0 / 64.0 }, {  -98.0 / 64.0, 55.0 / 64.0 }, { -122.0 / 64.0, 60.0 / 64.0 } };
    float ds;
    int di;
    float s_0;
    int i;

    for ( i = 0; i < BATCH_SIZE; i++ )
    {
        s_0 = samples[i] + *packPrev1 * fp[predict_nr][0] + *packPrev2 * fp[predict_nr][1];
        ds = s_0 * (float) ( 1 << shift_factor );
        di = ( (int) ds + 0x800 ) & 0xfffff000;
        if ( di > 32767 )
            di = 32767;
        if ( di < -32768 )
            di = -32768;
        four_bit[i] = (short) di;
        di = di >> shift_factor;
        *packPrev2 = *packPrev1;
        *packPrev1 = (float) di - s_0;
    }
}


//---------------------------------------------------------------------------

void adpcm64::_StartPack( int nrSampleFrames, int nrChannels )
{
	mNrChannels = nrChannels;
	mNrSamples = nrSampleFrames;
// 
	pack_prev_1L = 0.0f;
	pack_prev_2L = 0.0f;
	pack_prev_1R = 0.0f;
	pack_prev_2R = 0.0f;

	predict_prev_1L = 0.0f;
	predict_prev_2L = 0.0f;
	predict_prev_1R = 0.0f;
	predict_prev_2R = 0.0f;
	
	batchCount = 0;
	decodeBufferCount = 0;
}

int adpcm64::_PackWrite( short *inData, int nrInSamples, unsigned char *outData, int nrOutBytes )
{
	assert( !(nrInSamples & 1) );

	short four_bit[BATCH_SIZE];
	float sampleBatchL[BATCH_SIZE];
	float sampleBatchR[BATCH_SIZE];
    int predict_nr;
    int shift_factor;
    int k;
	int sampleCount = 0;
	int outCount = 0;
	bool batchComplete = false;
    while (( sampleCount < nrInSamples ) || (batchComplete != true ))
    {
		batchComplete = false;
		sampleBatchL[ batchCount ] = inData[ sampleCount ];
		sampleCount++;
		if (mNrChannels == 2) 
		{
			sampleBatchR[ batchCount ] = inData[ sampleCount ];
			sampleCount++;
		}

		batchCount++;
		if (batchCount == BATCH_SIZE)
		{
            find_predict( sampleBatchL, &predict_nr, &shift_factor, &predict_prev_1L, &predict_prev_2L );
            pack( sampleBatchL, four_bit, predict_nr, shift_factor, &pack_prev_1L, &pack_prev_2L );
            outData[ outCount ] = ( predict_nr << 4 ) | shift_factor;
			outCount++;
            for ( k = 0; k < BATCH_SIZE; k += 2 )
            {
                outData[ outCount ] = ( ( four_bit[k+1] >> 8 ) & 0xf0 ) | ( ( four_bit[k] >> 12 ) & 0xf );
				outCount++;
				assert( outCount < nrOutBytes );
			}
			if (mNrChannels == 2) 
			{
				find_predict( sampleBatchR, &predict_nr, &shift_factor, &predict_prev_1R, &predict_prev_2R );
				pack( sampleBatchR, four_bit, predict_nr, shift_factor, &pack_prev_1R, &pack_prev_2R );
				outData[ outCount ] = ( predict_nr << 4 ) | shift_factor;
				outCount++;
				for ( k = 0; k < BATCH_SIZE; k += 2 )
				{
					outData[ outCount ] = ( ( four_bit[k+1] >> 8 ) & 0xf0 ) | ( ( four_bit[k] >> 12 ) & 0xf );
					outCount++;
					assert( outCount < nrOutBytes );
				}
			}
			batchCount = 0;
			batchComplete = true;
		}
	}
	return outCount;
}

int adpcm64::_PackRead( unsigned char *inData, int nrInBytes, short *outData, int nrOutSamples )
{
	// Decompress inData to outData 
	const float f[5][2] = { { 0.0, 0.0 }, {   60.0 / 64.0,  0.0 }, {  115.0 / 64.0, -52.0 / 64.0 }, {   98.0 / 64.0, -55.0 / 64.0 }, {  122.0 / 64.0, -60.0 / 64.0 } };
	int predict_nrL, shift_factorL;
	int predict_nrR, shift_factorR;
	float samplesL[ BATCH_SIZE ];
	float samplesR[ BATCH_SIZE ];
    int i;
    int d, s;
	int inCount = 0;
	int outCount = 0;
	int inBytesRemaining = nrInBytes;
	int decodeBufferFillSize = ((BATCH_SIZE * mNrChannels) / 2) + mNrChannels;
	
	while( inBytesRemaining )
    {
		decodeBuffer[ decodeBufferCount++ ] = inData[ inCount++ ];
		inBytesRemaining--;

		if (decodeBufferCount == decodeBufferFillSize)
		{
			decodeBufferCount = 0;
			predict_nrL = decodeBuffer[ decodeBufferCount++ ];
			shift_factorL = predict_nrL & 0xf;
			predict_nrL >>= 4;

			for ( i = 0; i < BATCH_SIZE; i += 2 )
			{
				d = decodeBuffer[decodeBufferCount++];
				s = ( d & 0xf ) << 12;
				if ( s & 0x8000 ) s |= 0xffff0000;
				samplesL[i] = (float) ( s >> shift_factorL  );
				s = ( d & 0xf0 ) << 8;
				if ( s & 0x8000 ) s |= 0xffff0000;
				samplesL[i+1] = (float) ( s >> shift_factorL  );
			}
			if (mNrChannels == 2)
			{
				predict_nrR = decodeBuffer[decodeBufferCount++];
				shift_factorR = predict_nrR & 0xf;
				predict_nrR >>= 4;
				for ( i = 0; i < BATCH_SIZE; i += 2 )
				{
					d = decodeBuffer[decodeBufferCount++];
					s = ( d & 0xf ) << 12;
					if ( s & 0x8000 ) s |= 0xffff0000;
					samplesR[i] = (float) ( s >> shift_factorR  );
					s = ( d & 0xf0 ) << 8;
					if ( s & 0x8000 ) s |= 0xffff0000;
					samplesR[i+1] = (float) ( s >> shift_factorR  );
				}
			}

			for ( i = 0; i < BATCH_SIZE; i++ )
			{
				samplesL[i] = samplesL[i] + pack_prev_1L * f[predict_nrL][0] + pack_prev_2L * f[predict_nrL][1];
				pack_prev_2L = pack_prev_1L;
				pack_prev_1L = samplesL[i];
				d = (int) ( samplesL[i] + 0.5f );
				outData[ outCount++ ] = d;

				if (mNrChannels == 2)
				{
					samplesR[i] = samplesR[i] + pack_prev_1R * f[predict_nrR][0] + pack_prev_2R * f[predict_nrR][1];
					pack_prev_2R = pack_prev_1R;
					pack_prev_1R = samplesR[i];
					d = (int) ( samplesR[i] + 0.5f );
					outData[ outCount++ ] = d;
				}
			}

	    	decodeBufferCount = 0;
		}
	}

	return outCount;
}

void adpcm64::_PackClose()
{

}
