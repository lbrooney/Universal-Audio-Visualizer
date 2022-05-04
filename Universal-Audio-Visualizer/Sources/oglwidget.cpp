#include "oglwidget.h"
#pragma comment(lib, "winmm.lib")
using namespace std;

BOOL bDone = FALSE;
WAVEFORMATEX* pwfx = NULL;
int framePointer = 0;
double* in;
double mag[N/2];
fftw_complex* out;
fftw_plan p;

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
                  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
                  if ((punk) != NULL)  \
                    { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

DWORD WINAPI AudioThread(void* audio_semaphore);

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);

    clock();

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HANDLE audio_sema = CreateSemaphore(NULL, 0, 10, NULL);
    HANDLE thread = CreateThread(NULL, 0, AudioThread, &audio_sema, NULL, NULL);

    // Calls function to start recording
    RecordAudioStream();

    CoUninitialize();
}

OGLWidget::~OGLWidget()
{
    for(int i = objList.size() - 1; i >= 0; i--)
    {
        delete objList[i];
    }
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    Square* sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(-0.75f, 0.75f, 0.0f);
    sphere->freqBin = 400;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, 0.75f, 0.0f);
    sphere->freqBin = 350;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.75f, 0.75f, 0.0f);
    sphere->freqBin = 250;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(-0.75f, 0.0f, 0.0f);
    sphere->freqBin = 100;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, 0.0f, 0.0f);
    sphere->freqBin = 75;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.75f, 0.0f, 0.0f);
    sphere->freqBin = 50;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(-0.75f, -0.75f, 0.0f);
    sphere->freqBin = 20;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, -0.75f, 0.0f);
    sphere->freqBin = 15;
    objList.push_back((sphere));

    sphere = new Square(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.75f, -0.75f, 0.0f);
    sphere->freqBin = 10;
    objList.push_back((sphere));
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int u_lightColor = glGetUniformLocation(m_program.programId(), "u_lightColor");
    glUniform3f(u_lightColor, 1.0f, 1.0f, 1.0f);
    unsigned int u_lightDirection = glGetUniformLocation(m_program.programId(), "u_lightDirection");
    glUniform3f(u_lightDirection, 1.0f, 1.0f, -1.0f);

    for(int i = 0; i < objList.size(); i++)
    {
        objList[i]->scaleFactor = mag[objList[i]->freqBin] / 200;

        objList[i]->SetScale(objList[i]->scaleFactor, objList[i]->scaleFactor, objList[i]->scaleFactor);
        objList[i]->DrawShape(&m_program);
    }
    bDone = false;
    RecordAudioStream();

}

void OGLWidget::resizeGL(int w, int h)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(0, 0, w, h);
}

void OGLWidget::initShaders()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vertex.glsl");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fragment.glsl");
    m_program.link();
    m_program.bind();
}


HRESULT OGLWidget::RecordAudioStream()
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    // WAVEFORMATEX* pwfx = NULL;
    UINT32 packetLength = 0;

    BYTE* pData;
    DWORD flags;

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

        hr = pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

        hr = pDevice->Activate(
            IID_IAudioClient, CLSCTX_ALL,
            NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            hnsRequestedDuration,
            0,
            pwfx,
            NULL);
    EXIT_ON_ERROR(hr)

        // Get the size of the allocated buffer.
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->GetService(
            IID_IAudioCaptureClient,
            (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
        bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start recording.
    EXIT_ON_ERROR(hr)

        // Each loop fills about half of the shared buffer.
        while (bDone == FALSE)
        {
            // Sleep for half the buffer duration.
            Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)

                while (packetLength != 0)
                {
                    // Get the available data in the shared buffer.
                    hr = pCaptureClient->GetBuffer(
                        &pData,
                        &numFramesAvailable,
                        &flags, NULL, NULL);
                    EXIT_ON_ERROR(hr)

                        if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
                        {
                            pData = NULL;  // Tell CopyData to write silence.
                        }

                    // create buffers for FFT
                    in = (double*) fftw_malloc(sizeof(double) * numFramesAvailable);
                    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * numFramesAvailable);
                    p = fftw_plan_dft_r2c_1d(numFramesAvailable, in, out, FFTW_ESTIMATE);

                    // send captured audio data to be processed
                    hr = ProcessData(pData, numFramesAvailable, &bDone);

                    // free resources
                    fftw_destroy_plan(p);
                    fftw_free(in); fftw_free(out);

                        // pData, numFramesAvailable, &bDone, pwfx, (HMMIO)hFile);
                    EXIT_ON_ERROR(hr)

                        hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
                    EXIT_ON_ERROR(hr)

                        hr = pCaptureClient->GetNextPacketSize(&packetLength);
                    EXIT_ON_ERROR(hr)
                }
        }

    hr = pAudioClient->Stop();  // Stop recording.
    //framePointer = 0;
    EXIT_ON_ERROR(hr)

        //hr = FinishWaveFile((HMMIO)hFile, &ckData, &ckRIFF);
    if (FAILED(hr)) {
        // FinishWaveFile does it's own logging
        return hr;
    }

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pCaptureClient)

        return hr;
}

HRESULT OGLWidget::ProcessData(BYTE* pData, UINT32 NumFrames, BOOL* pDone)
{
    //apply Hann window to data and put into in buffer for FFT
    for(int j = 0; j < NumFrames; j++)
    {
        double multiplier = 0.5 * (1 - cos(2 * 3.1416 * j) / (NumFrames - 1));
        if(pData != NULL)
            in[j] = pData[j] * multiplier;
        else
            in[j] = 0;
    }
    //run FFT on data
    fftw_execute(p);

    //calculate log magnitude on transformed data
    for(int i = 0; i < NumFrames / 2; i++)
    {
        mag[i] = log(sqrt((out[i][0] * out[i][0]) + (out[i][1] * out[i][1]))) * 10;
    }

    if (clock() > 0.01 * CLOCKS_PER_SEC) //Record 0.01 seconds. From the first time call clock().
        *pDone = true;

    return S_OK;
}

DWORD WINAPI AudioThread(void *audio_semaphore) {
    audio_semaphore = HANDLE(audio_semaphore);
    WaitForSingleObject(audio_semaphore, INFINITE);
    return 0;
}
