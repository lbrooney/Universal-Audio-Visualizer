#include "AudioRecorder.h"
using namespace std;
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

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

    pDevice->Activate(
                IID_IAudioEndpointVolume, CLSCTX_ALL,
                NULL, (void**)&pEndpointVolume);

    pAudioClient->GetMixFormat(&pwfx);

    sampleRate = pwfx->nSamplesPerSec;

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

    uint_t hop_size = 256;
    aubioIn = new_fvec(hop_size);
    aubioOut = new_fvec(1);
    aubioTempo = new_aubio_tempo("default", 1024, hop_size, sampleRate);
}

AudioRecorder::~AudioRecorder()
{
    pAudioClient->Stop();  // Stop recording.
    CoUninitialize();

}

void AudioRecorder::Record()
{
    UINT32 numFramesAvailable;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    int frameCounter = 0;
    while(!bDone)
    {
        pCaptureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0)
        {
            // Sleep for half the buffer duration.
            //Sleep(25);
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
            for(int i = 0; i < numFramesAvailable && frameCounter < N; i++, frameCounter++)
            {
                //apply Hann window function to captured data
                double multiplier = 0.5 * (1 - cos(2 * 3.1416 * i) / (numFramesAvailable - 1));
                if(pData != NULL){         
                    in[frameCounter][0] = pData[i] * multiplier;
                    in[frameCounter][1] = 0;
                }
                else
                {
                    in[frameCounter][0] = 0;
                    in[frameCounter][1] = 0;
                }
            }

            //tempo stuff
            for(int i = 0; i < 256; i++)
            {
                fvec_set_sample(aubioIn, pData[i], i);
            }
            aubio_tempo_do(aubioTempo, aubioIn, aubioOut);
            if (aubioOut->data[0] != 0) {
                std::cout << "Tempo: " << aubio_tempo_get_bpm(aubioTempo) << " " << aubio_tempo_get_confidence(aubioTempo) << std::endl;
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
        mag[j] = log(sqrt((r * r) + (i * i))) * 20;
    }
}

float AudioRecorder::GetVolume()
{
    float vol = 0.0f;
    pEndpointVolume->GetMasterVolumeLevelScalar(&vol);
    return vol;
}


void AudioRecorder::Test()
{
    uint_t hop_size = 256;
    aubio_source_t * source = new_aubio_source("C:/song.wav", sampleRate, hop_size);
    aubioIn = new_fvec(hop_size);
    aubioOut = new_fvec(1);
    aubioTempo = new_aubio_tempo("default", 1024, hop_size, sampleRate);
    uint_t read = 0;

    do {
        // put some fresh data in input vector
        aubio_source_do(source, aubioIn, &read);
        // execute tempo
        aubio_tempo_do(aubioTempo,aubioIn,aubioOut);
        // do something with the beats
        if (aubioOut->data[0] != 0) {
            std::cout << aubio_tempo_get_bpm(aubioTempo) << std::endl;
        }
    } while (read == hop_size);
}
