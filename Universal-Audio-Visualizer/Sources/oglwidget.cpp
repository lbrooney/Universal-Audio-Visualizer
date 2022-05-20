#include "oglwidget.h"
using namespace std;

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_Recorder = new AudioRecorder();
    recording_thread = m_Recorder->RecordThread(exit_recording_thread_flag);
}

OGLWidget::~OGLWidget()
{
    if(recording_thread.joinable())
    {
        exit_recording_thread_flag = true;
        recording_thread.join();
    }

    objList.clear();
    delete m_Recorder;
}

void OGLWidget::initializeGL()
{
    float apsectRatio = (float)width() / (float)height();
    m_PerspectiveMatrix = glm::perspective(glm::radians(60.0f), apsectRatio, 0.1f, 1000.0f);
    m_ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    unsigned int u_lightColor = glGetUniformLocation(m_program.programId(), "u_lightColor");
    glUniform3f(u_lightColor, 1.0f, 1.0f, 1.0f);
    unsigned int u_lightDirection = glGetUniformLocation(m_program.programId(), "u_lightDirection");
    glUniform3f(u_lightDirection, 1.0f, 1.0f, 1.0f);

    unsigned int u_projMatrix = glGetUniformLocation(m_program.programId(), "u_ProjMatrix");
    glUniformMatrix4fv(u_projMatrix, 1, GL_FALSE, glm::value_ptr(m_PerspectiveMatrix));

    unsigned int u_ViewMatrix = glGetUniformLocation(m_program.programId(), "u_ViewMatrix");
    glUniformMatrix4fv(u_ViewMatrix, 1, GL_FALSE, glm::value_ptr(m_ViewMatrix));

    loadPreset(0);
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_Recorder->dataSemaphore.acquire();
    //if(!m_Recorder->dataQueue.empty())
    m_Recorder->ProcessData();

    if(!showSpectrum)
    {
        float volume = m_Recorder->GetVolume();
        int objCount = 0;
        int updateCycle = 5;
        for(int i = 0; i < objList.size(); i++)
        {
            double magnitude = objList[i]->m_Magnitude * objList[i]->intensityScale;
            if(drawCycleCount == updateCycle)
            {
                //update max objects on screen
                magnitude = clamp(m_Recorder->mag[objList[i]->freqBin], 0.0, maxMagnitude) * objList[i]->intensityScale;
                objList[i]->m_Magnitude = clamp(m_Recorder->mag[objList[i]->freqBin], 0.0, maxMagnitude);

                //update rotation
                float xRot = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/360.0f));
                float yRot = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/360.0f));
                float zRot = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/360.0f);
                objList[i]->SetRotation(xRot, yRot, zRot);

                //update position
                float xPos = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
                float yPos = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
                float zPos = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                zPos *= -2;
                objList[i]->SetTranslation(xPos, yPos, zPos);
            }
            if(objCount >= magnitude)
            {
                int maxObjects = maxMagnitude * objList[i]->intensityScale;
                i += maxObjects - objCount;
                objCount = 0;
            }
            else
            {
                objList[i]->SetScale(volume);
                objList[i]->SetColor(determineColor(m_Recorder->bpm));
                objList[i]->DrawShape(&m_program);
                objCount++;
            }
        }
        if(drawCycleCount == updateCycle)
            drawCycleCount = 0;

    }
    else
    {
        int updateCycle = 3;
        for(int i = 0; i < objList.size(); i++)
        {
            float magnitude;
            if(drawCycleCount == updateCycle)
            {
                magnitude = m_Recorder->mag[objList[i]->freqBin] / 30;
            }
            else
            {
                magnitude = objList[i]->m_Scale.y;
            }

            magnitude = clamp(magnitude, 0.01f, 10.0f);
            objList[i]->SetScale(objList[i]->m_Scale.x, magnitude, objList[i]->m_Scale.z);
            objList[i]->SetColor(determineColor(m_Recorder->bpm));

            objList[i]->DrawShape(&m_program);
        }
        if(drawCycleCount == updateCycle)
            drawCycleCount = 0;
    }

    drawCycleCount++;
    update();
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

QVector3D OGLWidget::determineColor(float bpm)
{
    QVector3D color = QVector3D(1, 1 , 1);
    if(bpm < 100.0f)
        color = QVector3D(0, 1, 0);
    else if(bpm < 125.0f)
        color = QVector3D(0, 1, 1);
    else if(bpm < 150.0f)
        color = QVector3D(0, 0, 1);
    else if(bpm < 175.0f)
        color = QVector3D(1, 0, 1);
    else if(bpm < 200.0f)
        color = QVector3D(1, 0, 0);
    return color;
}

void OGLWidget::loadPreset(int preset)
{
    float defaultIntensity = 0.5f;
    int count = static_cast<int>(maxMagnitude * defaultIntensity);
    switch(preset)
    {
    case 1:
    {
        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Prism(1.0f, 0.0f, 0.0f, 3));
            objList[i]->SetScale(0.2f);
            objList[i]->intensityScale = defaultIntensity;
            objList[i]->AssignFrequencyBin(150, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Cube(0.0f, 1.0f, 0.0f));
            objList[i]->SetScale(0.2f);
            objList[i]->intensityScale = defaultIntensity;
            objList[i]->AssignFrequencyBin(1000, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Sphere(0.0f, 0.0f, 1.0f));
            objList[i]->SetScale(0.2f);
            objList[i]->intensityScale = defaultIntensity;
            objList[i]->AssignFrequencyBin(5000, m_Recorder->sampleRate, N);
        }


        showSpectrum = false;
        break;
    }
    case 2:{
        //Bass with triangle prism
        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Prism(1.0f, 0.0f, 0.0f, 3));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(50, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Prism(0.0f, 1.0f, 0.0f, 3));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(100, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Prism(0.0f, 0.0f, 1.0f, 3));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(250, m_Recorder->sampleRate, N);
        }

        showSpectrum = false;
        break;
    }
    case 3:{
        //Mids with cube
        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Cube(1.0f, 0.0f, 0.0f));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(500, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Cube(0.0f, 1.0f, 0.0f));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(1000, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Cube(0.0f, 0.0f, 1.0f));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(3000, m_Recorder->sampleRate, N);
        }

        showSpectrum = false;
        break;
    }
    case 4:{
        //Highs with sphere
        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Sphere(1.0f, 0.0f, 0.0f));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(5000, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Sphere(0.0f, 1.0f, 0.0f));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(7500, m_Recorder->sampleRate, N);
        }

        for(int i = 0; i < count; i++)
        {
            objList.push_back(new Sphere(0.0f, 0.0f, 1.0f));
            objList[i]->intensityScale = 0.5f;
            objList[i]->SetScale(0.2f);
            objList[i]->AssignFrequencyBin(10000, m_Recorder->sampleRate, N);
        }

        showSpectrum = false;
        break;
    }
    default:
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
        showSpectrum = true;
    }
}

