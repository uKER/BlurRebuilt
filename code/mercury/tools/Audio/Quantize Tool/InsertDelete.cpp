#include "stdafx.h"

void Modify(BYTE* pDataIn, DWORD dwSizeIn, BYTE* pDataOut, DWORD dwSizeOut, char uChannels)
{
	DWORD InOffset = 0;
	DWORD OutOffset = 0;

	memset (pDataOut, 0, dwSizeOut);

	//	diff
	if (dwSizeIn != dwSizeOut)
	{
		int nChunks = (((int)dwSizeOut - (int)dwSizeIn) / uChannels) / 2;

		if (nChunks < 0)
		{
			//	remove
			nChunks *= -1;

			short* pSampleIn = (short*)pDataIn;
			short* pSampleOut = (short*)pDataOut;

			int iOffset = 0;

			if (g_bVerbose)
				printf ("from ", nChunks);

			float fChunkLen = (float)dwSizeIn/2 / (float)nChunks;

			//	chunk
			for (int iChunk=0; iChunk<nChunks; iChunk++)
			{
				int iGradient = 0xffff;
				int iStartAt = int((fChunkLen * iChunk + 0.5f) / uChannels) * uChannels;
				int iThisChunkLength = (int(((fChunkLen * (iChunk + 1))+0.5f) / uChannels) * uChannels) - iStartAt;
				
				//	find shallowest combined gradient for all channels
				for (int iCount = uChannels; iCount < iThisChunkLength - uChannels; iCount += uChannels)
				{
					short iTemp1stChannel = (abs(pSampleIn[iStartAt + iCount - uChannels] - pSampleIn[iStartAt + iCount]) + abs(pSampleIn[iStartAt + iCount] - pSampleIn[iStartAt + iCount+ uChannels]));
					short iTemp2ndChannel = 0;
					if (uChannels == 2)
						iTemp2ndChannel = (abs(pSampleIn[iStartAt + iCount - uChannels + 1] - pSampleIn[iStartAt + iCount + 1]) + abs(pSampleIn[iStartAt + iCount + 1] - pSampleIn[iStartAt + iCount + uChannels + 1]));
					int iTemp = iTemp1stChannel + iTemp2ndChannel;

					if (iTemp < iGradient)
					{
						iOffset = iCount;
						iGradient = iTemp;
					}
				}
				
				if (g_bVerbose)
				{
					printf ("%d", iStartAt + iOffset);
					if (iChunk < nChunks-1)
						printf (", ");
					else
						printf ("\n");
				}

				//	copy to output
				for (int iStart=0; iStart<iOffset; iStart += uChannels)
				{
					DWORD dwWritePos = iStartAt-(iChunk * uChannels)+iStart;
					if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))
						break;
					pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart];
					if (uChannels == 2)
					{
						dwWritePos = iStartAt-(iChunk * uChannels) + iStart + 1;
						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart + 1];
					}
				}

				for (int iEnd=iOffset; iEnd <= iThisChunkLength; iEnd += uChannels)
				{
					DWORD dwWritePos = iStartAt-(iChunk * uChannels)+iEnd;
					if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))
						break;
					pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd];
					if (uChannels == 2)
					{
						dwWritePos = iStartAt-(iChunk * uChannels)+iEnd + 1;
						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd+uChannels + 1];
					}
				}

//				printf("%d of %d", iStartAt-iChunk+iEnd, dwSizeOut/2);
			}
		}
		else
		{
			//	insert
			short* pSampleIn = (short*)pDataIn;
			short* pSampleOut = (short*)pDataOut;

			int iOffset = 0;

			if (g_bVerbose)
				printf ("at ");

			float fChunkLen = (float)(dwSizeIn / 2) / (float)nChunks;

			//	chunk
			for (int iChunk=0; iChunk<nChunks; iChunk++)
			{
				int iGradient = 0xffff;
				int iStartAt = int((fChunkLen * iChunk + 0.5f) / uChannels) * uChannels;

				int iThisChunkLength = (int(((fChunkLen * (iChunk + 1))+0.5f) / uChannels) * uChannels) - iStartAt;
			
				//	find shallowest gradient
				for (int iCount = uChannels; iCount < iThisChunkLength - uChannels; iCount += uChannels)
				{
					int iTemp;
					short iTemp1stChannel = (abs(pSampleIn[iStartAt + iCount - uChannels] - pSampleIn[iStartAt + iCount]) + abs(pSampleIn[iStartAt + iCount] - pSampleIn[iStartAt + iCount + uChannels]));
					short iTemp2ndChannel = 0;

					if (uChannels == 2)
					{
						iTemp2ndChannel = (abs(pSampleIn[iStartAt + iCount - uChannels + 1] - pSampleIn[iStartAt + iCount + 1]) + abs(pSampleIn[iStartAt + iCount + 1] - pSampleIn[iStartAt + iCount + uChannels + 1]));
					}

					iTemp = iTemp1stChannel + iTemp2ndChannel;
					
					if (iTemp < iGradient)
					{
						iOffset = iCount;		// remember this as the best candidate so far
						iGradient = iTemp;
					}
				}

				if (g_bVerbose)
				{
					printf ("%d", iStartAt + iOffset);
					//printf ("%2d :%d. ", iChunk+1, iStartAt + iOffset);
					//printf ("iStartAt: %d. ", iStartAt);
					//printf("chunklen = %4.2f %d\n", fChunkLen, iThisChunkLength);
		
					if (iChunk < nChunks-1)
						printf (", ");
					else
						printf ("\n\n");
				}
				//	copy beginning segment to output (ie everything up to the point we are inserting at)
				for (int iStart=0; iStart<=iOffset; iStart += uChannels)
				{
					DWORD dwWritePos = iStartAt+(iChunk * uChannels)+iStart;
					if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))	// bounds checking
						break;
					pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart];
					if (uChannels == 2)
					{
						dwWritePos = iStartAt+(iChunk * uChannels)+iStart + 1;
						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))	// bounds checking
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iStart + 1];
					}
				}

				//	copy end segment to output (ie after the point we inserted at - repeating the last sample)
				for (int iEnd=iOffset; iEnd < iThisChunkLength; iEnd += uChannels)
				{
					DWORD dwWritePos = iStartAt+(iChunk * uChannels)+iEnd + uChannels;

					if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))	// bounds checking
						break;
					pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd];
					if (uChannels == 2)
					{
						dwWritePos = iStartAt+(iChunk * uChannels)+iEnd + uChannels + 1;
						if (! (&(pSampleOut[dwWritePos]) >= (short*)pDataOut && &(pSampleOut[dwWritePos]) < (short*)(pDataOut + dwSizeOut)))	// bounds checking
							break;
						pSampleOut[dwWritePos] = pSampleIn[iStartAt+iEnd + 1];
					}
				}

				//printf("%d of %d", iStartAt+iChunk+iEnd+1, dwSizeOut/2);
			}
		}
	}

	//	same
	else
	{
		memcpy(pDataOut, pDataIn, dwSizeIn);
		if (g_bVerbose)
			printf ("\n\n");
	}
}