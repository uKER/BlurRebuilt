#include "AudioPch.h"
#include "coreheaders.h"

/*
////////////////////////////////////////////////////////////////////////////////
//	structs
////////////////////////////////////////////////////////////////////////////////
struct RiffChunk
{
    DWORD idChunk;
    DWORD dwSize;
};

struct HeaderChunk
{
    RiffChunk riff;

    DWORD idType;
};

struct FormatChunk
{
    RiffChunk riff;

    U16 wFormat;
    U16 wChannels;
    DWORD dwSamplesPerSec;
    DWORD dwAvgBytesPerSec;
    U16 wBlockAlign;
    U16 wBitsPerSample;
};

////////////////////////////////////////////////////////////////////////////////
//	RenderToFile
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::WavRenderer::RenderToFile(char* pchFilename, Audio::Core::IGenerator* pInput, F32 fLength)
{
    //  Assert(pInput->Channels() == uChannelsOut);

//    DWORD dwWritten;

    U32 uSamples = (U32)(fLength * Core::uSampleRate);
    U32 uFrameLength = Core::uFrameLength;
    U32 uSampleRate = Core::uSampleRate;
    U32 uChannelsIn = pInput->Channels();
    U32 uChannelsOut = 2;//pInput->Channels();

    //  TODO: Assert(channels in = channels out)

    //  alloc frame memory
    m_pOutputBuffer = (S16*)malloc(uFrameLength * uChannelsOut * sizeof(S16));

    //  open output file
    //m_hFile = CreateFile(pchFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);

//    if (m_hFile == INVALID_HANDLE_VALUE)
//        return false;

    //  get audio data size
    U32 uDataSize = uSamples * 1 * sizeof(S16);

    //  create wave file structures
    HeaderChunk header;
    header.riff.idChunk = 0x46464952; // 'RIFF'
    header.riff.dwSize = sizeof(HeaderChunk) + sizeof(FormatChunk) + sizeof(RiffChunk) + uDataSize - 8;

    header.idType = 0x45564157; // 'WAVE'

    //  write header
//    if (TRUE != WriteFile(m_hFile, &header, sizeof(HeaderChunk), &dwWritten, NULL))
//        return false;

    FormatChunk fmt;
    fmt.riff.idChunk = 0x20746D66; // 'fmt '
    fmt.riff.dwSize = 16;

    fmt.wFormat = 1; // PCM
    fmt.wChannels = (U16)uChannelsOut;
    fmt.dwSamplesPerSec = Core::uSampleRate;
    fmt.wBitsPerSample = 16;
    fmt.wBlockAlign = fmt.wBitsPerSample / 8 * fmt.wChannels;
    fmt.dwAvgBytesPerSec = fmt.dwSamplesPerSec * fmt.wBlockAlign;

    //  write fmt chunk
//    if (TRUE != WriteFile(m_hFile, &fmt, sizeof(FormatChunk), &dwWritten, NULL))
//        return false;

    RiffChunk data;
    data.idChunk = 0x61746164; // "data"
    data.dwSize = uDataSize;

    //  write data chunk
//    if (TRUE != WriteFile(m_hFile, &data, sizeof(RiffChunk), &dwWritten, NULL))
//        return false;

    F32* afSamples = (F32*)pInput->FrameData();

    U32 uFrame = 0;

    //  create enough audio frames
    while (uFrame * uFrameLength < uSamples)
    {
        //AUDIO_SYS_DEBUG("Processing frame %d", uFrame);

        pInput->Process(uFrame, true);

        U32 uSampleOut = 0;
        for (U32 uSampleIn = 0; uSampleIn < uFrameLength; uSampleIn++)
        {
            m_pOutputBuffer[uSampleOut++] = (S16)(32767.f * afSamples[uSampleIn]);
            m_pOutputBuffer[uSampleOut++] = (S16)(32767.f * afSamples[uSampleIn]);    //TODO: offset by num channels in
            //m_pOutputBuffer[uSampleOut++] = (S16)(32767.f * afSamples[uFrameLength + uSampleIn]);    //TODO: offset by num channels in
        }

        //if (TRUE != WriteFile(m_hFile, m_pOutputBuffer, uFrameLength * uChannelsOut * sizeof(S16), &dwWritten, NULL))
        //    return false;

        uFrame++;
    }

    //  create remainder
    U32 uExtra = uSamples - ((uFrame - 1) * uFrameLength);

    //  extra samples (less than a whole frame)
    if (uExtra)
    {
        pInput->Process(uFrame, true);

        U32 uSampleOut = 0;
        for (U32 uSample = 0; uSample < uExtra; uSample++)
        {
            m_pOutputBuffer[uSample++] = (S16)(32767.f * afSamples[uSample]);
            m_pOutputBuffer[uSample++] = (S16)(32767.f * afSamples[uFrameLength + uSample]);    //TODO: offset by num channels in
        }

        //if (TRUE != WriteFile(m_hFile, m_pOutputBuffer, uExtra * uChannelsOut * sizeof(S16), &dwWritten, NULL))
        //    return false;
    }

    //  free audio memory
    free(m_pOutputBuffer);

    //  close file
    //if (TRUE != CloseHandle(m_hFile))
    //    return false;

    return true;
}
*/

