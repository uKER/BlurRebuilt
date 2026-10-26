#include <stdafx.h>

#define GROUPING_LEVEL  64

float f[5][2] = { { 0.0, 0.0 }, {   60.0 / 64.0,  0.0 }, {  115.0 / 64.0, -52.0 / 64.0 }, {   98.0 / 64.0, -55.0 / 64.0 }, {  122.0 / 64.0, -60.0 / 64.0 } };
float samples[GROUPING_LEVEL];

__int64 bitcount = 0;
int mfputc(char c, FILE *stream)
{
        static char prev = 0;
        int diff = c - prev;
        prev = c;
        bitcount += abs(diff);
        return fputc( c, stream );
}
int mfgetc(FILE *stream)
{
        return fgetc( stream );
}

int FileLength( FILE *fp )
{
        int oldpos = ftell( fp );
        int length;
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, oldpos, SEEK_SET);
        return length;

}

int Depack( FILE *compressed, FILE *pcm )
{
    int predict_nr, shift_factor;
    int i;
    int d, s;
    static float s_1 = 0.0;
    static float s_2 = 0.0;
    while( 1 )
    {
        predict_nr = fgetc( compressed );
        shift_factor = predict_nr & 0xf;
        predict_nr >>= 4;
        if (feof( compressed )) break;
        for ( i = 0; i < GROUPING_LEVEL; i += 2 )
        {
            d = fgetc( compressed );
            s = ( d & 0xf ) << 12;
            if ( s & 0x8000 )
            {
                s |= 0xffff0000;
            }
            samples[i] = (float) ( s >> shift_factor  );
            s = ( d & 0xf0 ) << 8;
            if ( s & 0x8000 )
            {
                s |= 0xffff0000;
            }
            samples[i+1] = (float) ( s >> shift_factor  );
        }

        for ( i = 0; i < GROUPING_LEVEL; i++ )
        {
            samples[i] = samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
            s_2 = s_1;
            s_1 = samples[i];
            d = (int) ( samples[i] + 0.5 );
            fputc( d & 0xff, pcm );
            fputc( d >> 8, pcm );
        }
    }

    return( 0 );

}



static float fp[5][2] = { { 0.0, 0.0 }, {  -60.0 / 64.0, 0.0 }, { -115.0 / 64.0, 52.0 / 64.0 }, {  -98.0 / 64.0, 55.0 / 64.0 }, { -122.0 / 64.0, 60.0 / 64.0 } };


#define BUFFER_SIZE 128*GROUPING_LEVEL
short wave[BUFFER_SIZE];
void find_predict( short *samples, float *d_samples, int *predict_nr, int *shift_factor );
void pack( float *d_samples, short *four_bit, int predict_nr, int shift_factor );

int Pack( FILE *fp, FILE *compressed )
{
    short *ptr;
    float d_samples[GROUPING_LEVEL];
    short four_bit[GROUPING_LEVEL];
    int predict_nr;
    int shift_factor;
    int size;
    int i, j, k;
    unsigned char d;

    int sample_len;
    fseek(fp,0,SEEK_END);
    sample_len = ftell(fp) >> 1;
    fseek(fp,0,SEEK_SET);
    while( sample_len > 0 )
    {
        size = ( sample_len >= BUFFER_SIZE ) ? BUFFER_SIZE : sample_len;
        fread( wave, sizeof( short ), size, fp );
        i = size / GROUPING_LEVEL;
        if ( size % GROUPING_LEVEL )
        {
            for ( j = size % GROUPING_LEVEL; j < GROUPING_LEVEL; j++ )
            {
                wave[GROUPING_LEVEL*i+j] = 0;
            }
            i++;
        }

        for ( j = 0; j < i; j++ )
        {                                     // pack GROUPING_LEVEL samples
            ptr = wave + j * GROUPING_LEVEL;
            find_predict( ptr, d_samples, &predict_nr, &shift_factor );
            pack( d_samples, four_bit, predict_nr, shift_factor );
            d = ( predict_nr << 4 ) | shift_factor;
            mfputc( d, compressed );
            for ( k = 0; k < GROUPING_LEVEL; k += 2 )
            {
                d = ( ( four_bit[k+1] >> 8 ) & 0xf0 ) | ( ( four_bit[k] >> 12 ) & 0xf );
                mfputc( d, compressed );
            }
            sample_len -= GROUPING_LEVEL;
        }
    }
    return( 0 );

}



void find_predict( short *samples, float *d_samples, int *predict_nr, int *shift_factor )
{
    int i, j;
    float buffer[GROUPING_LEVEL][5];
    float min = 1e10;
    float max[5];
    float ds;
    int min2;
    int shift_mask;
    static float _s_1 = 0.0;                            // s[t-1]
    static float _s_2 = 0.0;                            // s[t-2]
    float s_0, s_1, s_2;
    for ( i = 0; i < 5; i++ )
    {
        max[i] = 0.0;
        s_1 = _s_1;
        s_2 = _s_2;
        for ( j = 0; j < GROUPING_LEVEL; j ++ )
        {
            s_0 = (float) samples[j];                      // s[t-0]
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
    _s_1 = s_1;
    _s_2 = s_2;
    for ( i = 0; i < GROUPING_LEVEL; i++ )
    {
        d_samples[i] = buffer[i][*predict_nr];
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

void pack( float *d_samples, short *four_bit, int predict_nr, int shift_factor )
{
    float ds;
    int di;
    float s_0;
    static float s_1 = 0.0;
    static float s_2 = 0.0;
    int i;

    for ( i = 0; i < GROUPING_LEVEL; i++ )
    {
        s_0 = d_samples[i] + s_1 * fp[predict_nr][0] + s_2 * fp[predict_nr][1];
        ds = s_0 * (float) ( 1 << shift_factor );
        di = ( (int) ds + 0x800 ) & 0xfffff000;
        if ( di > 32767 )
            di = 32767;
        if ( di < -32768 )
            di = -32768;
        four_bit[i] = (short) di;
        di = di >> shift_factor;
        s_2 = s_1;
        s_1 = (float) di - s_0;
    }
}


void test(char* fileName,char* sOutputFile,char* sADPCMFile)
{
        printf("Pack\n");
        FILE *fp;
        FILE *fpo;
        fp = fopen(fileName,"rb");
        fpo = fopen(sADPCMFile,"wb");
        Pack( fp, fpo );
        int original = FileLength(fp);
        int pack = FileLength(fpo);
        fclose(fp);
        fclose(fpo);
        printf("Depack\n");
        fp = fopen(sADPCMFile,"rb");
        fpo = fopen(sOutputFile,"wb");
        Depack( fp, fpo );
        fclose(fp);
        fclose(fpo);
        int percent = (pack * 100 / original);
		printf("Done %d% Ratio %4.2f:1\n",percent,100.0f/percent);
		printf("Press Return key to exit\n");
		getchar();

}
//---------------------------------------------------------------------------

