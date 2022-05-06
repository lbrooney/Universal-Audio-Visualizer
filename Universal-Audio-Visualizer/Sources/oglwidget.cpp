#include "oglwidget.h"
#pragma comment(lib, "winmm.lib")
using namespace std;

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

IMMDeviceEnumerator* pEnumerator = NULL;
IMMDevice* pDevice = NULL;
IAudioClient* pAudioClient = NULL;
IAudioCaptureClient* pCaptureClient = NULL;

BOOL bDone = FALSE;
WAVEFORMATEX* pwfx = NULL;
int framePointer = 0;

DWORD WINAPI AudioThread(void* audio_semaphore);

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);

    HANDLE audio_sema = CreateSemaphore(NULL, 0, 10, NULL);
    HANDLE thread = CreateThread(NULL, 0, AudioThread, &audio_sema, NULL, NULL);

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

OGLWidget::~OGLWidget()
{
    for(int i = objList.size() - 1; i >= 0; i--)
    {
        delete objList[i];
    }
    pAudioClient->Stop();  // Stop recording.
    CoUninitialize();
}

void OGLWidget::initializeGL()
{
    float apsectRatio = (float)width() / (float)height();
    m_PerspectiveMatrix = glm::perspective(glm::radians(60.0f), apsectRatio, 0.1f, 1000.0f);
    m_ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    /*Sphere* sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, 0.75f, 0.0f);
    sphere->freqBin = 1;
    objList.push_back((sphere));
    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, 0.0f, -1.0f);
    sphere->freqBin = 3;
    objList.push_back((sphere));
    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, -0.75f, 1.0f);
    sphere->freqBin = 5;
    objList.push_back((sphere));

    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, 0.75f, 0.0f);
    sphere->freqBin = 8;
    objList.push_back((sphere));
    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, 0.0f, 0.0f);
    sphere->freqBin = 16;
    objList.push_back((sphere));
    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.0f, -0.75f, 0.0f);
    sphere->freqBin = 32;
    objList.push_back((sphere));

    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.75f, 0.75f, 0.0f);
    sphere->freqBin = 86;
    objList.push_back((sphere));
    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.75f, 0.0f, 0.0f);
    sphere->freqBin = 130;
    objList.push_back((sphere));
    sphere = new Sphere(1.0f, 0.0f, 0.0f);
    sphere->SetTranslation(0.75f, -0.75f, 0.0f);
    sphere->freqBin = 200;
    objList.push_back((sphere));*/
    int binCounter = 0;
    float xPos = -1.0f;
    for(int i = 0; i < 200; i++)
    {
       Cube* s = new Cube(1.0f, 0.0f, 0.0f);

       s->SetTranslation(xPos, 0.0f, 0.0f);
       s->SetScale(0.01f, 0.01f, 0.01f);
       objList.push_back(s);
       s->freqBin = binCounter;
       binCounter += 2;
       xPos += 0.01f;
    }
    objList[0]->freqBin = 1;
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int u_lightColor = glGetUniformLocation(m_program.programId(), "u_lightColor");
    glUniform3f(u_lightColor, 1.0f, 1.0f, 1.0f);
    unsigned int u_lightDirection = glGetUniformLocation(m_program.programId(), "u_lightDirection");
    glUniform3f(u_lightDirection, 1.0f, 1.0f, 1.0f);

    unsigned int u_projMatrix = glGetUniformLocation(m_program.programId(), "u_ProjMatrix");
    glUniformMatrix4fv(u_projMatrix, 1, GL_FALSE, glm::value_ptr(m_PerspectiveMatrix));

    unsigned int u_ViewMatrix = glGetUniformLocation(m_program.programId(), "u_ViewMatrix");
    glUniformMatrix4fv(u_ViewMatrix, 1, GL_FALSE, glm::value_ptr(m_ViewMatrix));

    for(int i = 0; i < objList.size(); i++)
    {
        float magnitude = mag[objList[i]->freqBin];
        magnitude = clamp(magnitude, 0.01f, 10.0f);
        objList[i]->SetScale(objList[i]->m_Scale.x, magnitude, objList[i]->m_Scale.z);

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


void OGLWidget::RecordAudioStream()
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

void OGLWidget::ProcessData(BYTE* pData)
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

DWORD WINAPI AudioThread(void *audio_semaphore) {
    audio_semaphore = HANDLE(audio_semaphore);
    WaitForSingleObject(audio_semaphore, INFINITE);
    return 0;
}
