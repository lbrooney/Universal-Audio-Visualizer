#include "AudioRecorder.h"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);


AudioRecorder::AudioRecorder()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;

    CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);

    pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);

    pDevice->Activate(
            IID_IAudioClient, CLSCTX_ALL,
            NULL, (void**)&pAudioClient);

    pAudioClient->GetMixFormat(&pwfx);

    pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            hnsRequestedDuration,
            0,
            pwfx,
            NULL);

    pAudioClient->GetService(
            IID_IAudioCaptureClient,
            (void**)&pCaptureClient);

    pAudioClient->Start();
}

AudioRecorder::~AudioRecorder()
{
    pAudioClient->Stop();  // Stop recording.
    CoUninitialize();
}

void AudioRecorder::Record()
{
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags;

    // Get the size of the allocated buffer.
    pAudioClient->GetBufferSize(&bufferFrameCount);

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    int frameCounter = 0;
    while(!bDone)
    {
        pCaptureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0 && !bDone)
        {
            // Sleep for half the buffer duration.
            Sleep(25);
            // Get the available data in the shared buffer.
            pCaptureClient->GetBuffer(
                &pData,
                &numFramesAvailable,
                &flags, NULL, NULL);

           if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
           {
                pData = NULL;  // Tell CopyData to write silence.
           }

           //copy data the in buffer
           for(int j = 0; j < numFramesAvailable && frameCounter < 1024; j++, frameCounter++)
           {
               //apply Hann window function to captured data
               double multiplier = 0.5 * (1 - cos(2 * 3.1416 * j) / (numFramesAvailable - 1));
               if(pData != NULL){
                   in[frameCounter][0] = pData[j] * multiplier;
                   in[frameCounter][1] = 0;
               }
               else
               {
                   in[frameCounter][0] = 0;
                   in[frameCounter][1] = 0;
               }
           }

           // if in buffer is full, exit out of capture loop
           if(frameCounter == N)
               bDone = true;

           pCaptureClient->ReleaseBuffer(numFramesAvailable);
           pCaptureClient->GetNextPacketSize(&packetLength);
        }
    }

    //process capture audio data
    ProcessData(pData);

    // free resources
    fftw_destroy_plan(p);
    fftw_free(in); fftw_free(out);
}

void AudioRecorder::ProcessData(BYTE* pData)
{
    //run FFT on data
    fftw_execute(p);

    //calculate log magnitude on transformed data
    for(int j = 0; j < N / 2; j++)
    {
        float r = out[j][0] / N;
        float i = out[j][1] / N;
        mag[j] = log(sqrt((r * r) + (i * i))) / 2;
    }
}
