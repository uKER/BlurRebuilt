
#include "stdafx.h"
#include "app.h"

// Write a 16-bit Wav header:
static void WavHeader(FILE *file, int channels, int rate)
{
	unsigned char header[0x2c]=
	{
		'R', 'I', 'F', 'F',   0,   0,   0,   0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0, 'd', 'a', 't', 'a',   0,   0,   0,   0
	};

	header[0x10] = 0x10; // ?
	header[0x14] = 1; // Format (PCM)
	header[0x16] = (unsigned char)channels; // Channels
	int align = channels << 1; // Bytes per sample / block align
	header[0x18] = (unsigned char) rate;
	header[0x19] = (unsigned char)(rate >> 8);
	header[0x1a] = (unsigned char)(rate >> 16);
	int aps = rate * align; // Average bytes per second
	header[0x1c] = (unsigned char) aps;
	header[0x1d] = (unsigned char)(aps >> 8);
	header[0x1e] = (unsigned char)(aps >> 16);
	header[0x20] = (unsigned char)align; // Block align
	header[0x22] = 16; // Bits per sample
	fwrite(header, 1, sizeof(header), file);
}

static void LittleDword(FILE *file, int val)
{
	unsigned char tmp[4] = {0};
	tmp[0] = (unsigned char)val;
	tmp[1] = (unsigned char)(val >> 8);
	tmp[2] = (unsigned char)(val >> 16);
	tmp[3] = (unsigned char)(val >> 24);
	fwrite(tmp, 1, sizeof(tmp), file);
}

static void WavLengths(FILE *file)
{
	fseek(file, 0, SEEK_END);
	
	// Now fill in Wav lengths:
	int length = ftell(file);

	fseek(file, 4, SEEK_SET);
	LittleDword(file, length - 8);
	fseek(file, 0x28, SEEK_SET);
	LittleDword(file, length - 0x2c);
}

static FILE *WaveOpen(char *name, int channels)
{
	FILE *file = fopen(name, "rb+");
	if (file == 0)
	{
		// Create a new WAV file:
		file = fopen(name, "wb");
		if (file == 0) return 0;
		WavHeader(file, channels, 48000);
	}
	else
	{
		fseek(file, 0, SEEK_END);
	}
	return file;
}

static int AudioProcess2(unsigned char *data, int channels, int size)
{
	if (channels > 6) channels = 6;
	
	// Try and open the wave file:
	char text[384] = {0};
	sprintf(text,"%.260s\\Audio.wav", MainPath);
	FILE *file = WaveOpen(text, 2);
	
	if (file == NULL)
		return 1;

	int len = size << 2;
	unsigned char *output = new unsigned char[len];
	if (output == NULL) { fclose(file); return 1; }
	memset(output, 0, len);
	
	int i = 0;
	for ( ; i < size; i++)
	{
		int chan[6] = {0};
		
		// Go through each channel
		unsigned char *ps = data + (i << 1);
		int c = 0;
		for ( ; c < channels; c++, ps += (size << 1))
		{
			chan[c] = (int)(short)(ps[0] << 8) | ps[1];
		}

		int left  = chan[0] + chan[4] + chan[2];
		int right = chan[1] + chan[5] + chan[2];
		
#define CLIP(x) if ((x+0x8000)&0xffff0000) { if (x>0) x=0x7fff; else x=-0x8000; }
		CLIP(left)
		CLIP(right)
		
		unsigned char *po = output + (i << 2);
		po[0] = (unsigned char) left;
		po[1] = (unsigned char)(left >> 8);
		po[2] = (unsigned char) right;
		po[3] = (unsigned char)(right >> 8);
	}
	
	fwrite(output, 1, len, file);
	
	WavLengths(file);
	
	fclose(file);
	return 0;
}

static int AudioProcess51(unsigned char *data, int channels, int size)
{
	if (channels > 6) channels = 6;

	// Try and open the wave file:
	char text[384] = {0};
	sprintf(text,"%.260s\\Audio51.wav", MainPath);
	FILE *file = WaveOpen(text, 6);
	
	if (file == NULL)
		return 1;

	int len = size * 12;
	unsigned char *output = new unsigned char[len];
	if (output == NULL) { fclose(file); return 1; }
	memset(output, 0, len);
	
	int i = 0;
	for ( ; i < size; i++)
	{
		int chan[6] = {0};
		
		// Go through each channel
		unsigned char *ps = data + (i << 1);
		int c = 0;
		for ( ; c < channels; c++, ps += (size << 1))
		{
			chan[c] = (int)(short)(ps[0] << 8) | ps[1];
		}

		unsigned char *po = output + i * 12;
		
		for (c = 0 ; c < channels; c++)
		{
			int val = chan[c];
			CLIP(val)
			po[ c << 1     ] = (unsigned char) val;
			po[(c << 1) + 1] = (unsigned char)(val >> 8);
		}
	}
	
	fwrite(output, 1, len, file);
	
	WavLengths(file);
	
	fclose(file);
	return 0;
}

int AudioProcess(unsigned char *data, int channels, int size)
{
	AudioProcess2 (data, channels, size);
	AudioProcess51(data, channels, size);

	return 0;
}
