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

    uint_t hop_size = 512;
    aubioIn = new_fvec(hop_size);
    aubioOut = new_fvec(1);
    aubioTempo = new_aubio_tempo("default", 1024, hop_size, sampleRate);

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
}

AudioRecorder::~AudioRecorder()
{
    pAudioClient->Stop();  // Stop recording.
    CoUninitialize();

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    del_aubio_tempo(aubioTempo);
    del_fvec(aubioIn);
    del_fvec(aubioOut);
}

void AudioRecorder::Record(std::atomic_bool &exit_flag)
{
    UINT32 numFramesAvailable;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags;

    int frameCounter = 0;
    BYTE* byteArray = new BYTE[N];

    while(!exit_flag)
    {
        pCaptureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0 && !exit_flag)
        {
            pCaptureClient->GetBuffer(
                        &pData,
                        &numFramesAvailable,
                        &flags, NULL, NULL);

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;  // Tell CopyData to write silence.
            }
            //copy data the in buffer
            for(int i = 0; i < numFramesAvailable; i++, frameCounter++)
            {
                if(pData)
                    byteArray[frameCounter] = pData[i];
                else
                    byteArray[frameCounter] = 0;

                if(frameCounter == N - 1)
                {
                    frameCounter = 0;
                    dataQueue.push(byteArray);
                    byteArray = new BYTE[N];
                }
            }

            const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pData);
            for(int i = 0; i < numFramesAvailable; i++)
            {
                float sample = *reinterpret_cast<const float*>(ptr);
                tempoQueue.push(sample);
                ptr += sizeof(float);
            }

            pCaptureClient->ReleaseBuffer(numFramesAvailable);
            pCaptureClient->GetNextPacketSize(&packetLength);
        }
    }
}

void AudioRecorder::ProcessData()
{
    BYTE* data = dataQueue.front();
    int aubioIndex = 0;
    for(int dataIndex = 0; dataIndex < N; dataIndex++)
    {
        //apply Hann window function to captured data
        double multiplier = 0.5 * (1 - cos(2 * 3.1416 * dataIndex) / (N - 1));
        in[dataIndex][0] =  data[dataIndex] * multiplier;
        in[dataIndex][1] = 0;
    }

    for(int i = 0; i < N && !tempoQueue.empty(); i++, aubioIndex++)
    {
        fvec_set_sample(aubioIn, tempoQueue.front(), aubioIndex);
        tempoQueue.pop();
        if(aubioIndex == 511)
        {
            aubio_tempo_do(aubioTempo, aubioIn, aubioOut);
            if (aubioOut->data[0] != 0) {
                std::cout << "Realtime Tempo: " << aubio_tempo_get_bpm(aubioTempo) << " " << aubio_tempo_get_confidence(aubioTempo) << std::endl;
            }
            aubioIndex = -1;
        }
    }

    delete[] data;
    dataQueue.pop();

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
