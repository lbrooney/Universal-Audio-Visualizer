#include "oglwidget.h"
using namespace std;
bool showSpectrum = false;

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(75);

    m_Recorder = new AudioRecorder();
    //m_Recorder->Test();
}

OGLWidget::~OGLWidget()
{
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

    loadPreset(1);
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

    if(!showSpectrum){
        float volume = m_Recorder->GetVolume();
        for(int i = 0; i < objList.size(); i++)
        {
            if(objList[i]->enabled)
            {
                float magnitude = m_Recorder->mag[objList[i]->freqBin];
                magnitude = clamp(magnitude, 0.0f, 10.0f) * objList[i]->intensityScale;

                //draw shapes based on magnitude of assigned frequency
                for(int j = 0; j < magnitude; j++)
                {
                    //generate random rotation
                    float xRot = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/360.0f));
                    float yRot = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/360.0f));
                    float zRot = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/360.0f);
                    objList[i]->SetRotation(xRot, yRot, zRot);

                    //generate random positions for the shapes
                    float xPos = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
                    float yPos = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
                    float zPos = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    zPos *= -2;
                    objList[i]->SetTranslation(xPos, yPos, zPos);
                    objList[i]->SetScale(volume);

                    objList[i]->DrawShape(&m_program);
                }
            }
        }
    }
    else
    {
        for(int i = 0; i < objList.size(); i++)
        {
            float magnitude = m_Recorder->mag[objList[i]->freqBin] / 60;
            magnitude = clamp(magnitude, 0.01f, 10.0f);
            objList[i]->SetScale(objList[i]->m_Scale.x, magnitude, objList[i]->m_Scale.z);

            objList[i]->DrawShape(&m_program);
        }
    }

    m_Recorder->bDone = false;
    m_Recorder->Record();

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

void OGLWidget::loadPreset(int preset)
{
    switch(preset)
    {
    case 1:
        objList.push_back(new Prism(1.0f, 0.0f, 0.0f, 3));
        objList.push_back(new Cube(0.0f, 1.0f, 0.0f));
        objList.push_back(new Sphere(0.0f, 0.0f, 1.0f));

        objList[0]->enabled = true;
        objList[1]->enabled = true;
        objList[2]->enabled = true;

        objList[0]->intensityScale = 0.5f;
        objList[1]->intensityScale = 0.5f;
        objList[2]->intensityScale = 0.5f;

        objList[0]->SetScale(0.2f, 0.2f, 0.2f);
        objList[1]->SetScale(0.2f, 0.2f, 0.2f);
        objList[2]->SetScale(0.2f, 0.2f, 0.2f);

        objList[0]->AssignFrequencyBin(150, m_Recorder->sampleRate, N);
        objList[1]->AssignFrequencyBin(1000, m_Recorder->sampleRate, N);
        objList[2]->AssignFrequencyBin(5000, m_Recorder->sampleRate, N);
        showSpectrum = false;
        break;
    case 2:
        //Bass with triangle prism
        objList.push_back(new Prism(1.0f, 0.0f, 0.0f, 3));
        objList.push_back(new Prism(0.0f, 1.0f, 0.0f, 3));
        objList.push_back(new Prism(0.0f, 0.0f, 1.0f, 3));

        objList[0]->enabled = true;
        objList[1]->enabled = true;
        objList[2]->enabled = true;

        objList[0]->intensityScale = 0.5f;
        objList[1]->intensityScale = 0.5f;
        objList[2]->intensityScale = 0.5f;

        objList[0]->SetScale(0.2f, 0.2f, 0.2f);
        objList[1]->SetScale(0.2f, 0.2f, 0.2f);
        objList[2]->SetScale(0.2f, 0.2f, 0.2f);

        objList[0]->AssignFrequencyBin(50, m_Recorder->sampleRate, N);
        objList[1]->AssignFrequencyBin(100, m_Recorder->sampleRate, N);
        objList[2]->AssignFrequencyBin(250, m_Recorder->sampleRate, N);
        showSpectrum = false;
        break;
    case 3:
        //Mids with cube
        objList.push_back(new Cube(1.0f, 0.0f, 0.0f));
        objList.push_back(new Cube(0.0f, 1.0f, 0.0f));
        objList.push_back(new Cube(0.0f, 0.0f, 1.0f));

        objList[0]->enabled = true;
        objList[1]->enabled = true;
        objList[2]->enabled = true;

        objList[0]->intensityScale = 0.5f;
        objList[1]->intensityScale = 0.5f;
        objList[2]->intensityScale = 0.5f;

        objList[0]->SetScale(0.2f, 0.2f, 0.2f);
        objList[1]->SetScale(0.2f, 0.2f, 0.2f);
        objList[2]->SetScale(0.2f, 0.2f, 0.2f);

        objList[0]->AssignFrequencyBin(500, m_Recorder->sampleRate, N);
        objList[1]->AssignFrequencyBin(1000, m_Recorder->sampleRate, N);
        objList[2]->AssignFrequencyBin(3000, m_Recorder->sampleRate, N);
        showSpectrum = false;
        break;
    case 4:
        //Highs with sphere
        objList.push_back(new Sphere(1.0f, 0.0f, 0.0f));
        objList.push_back(new Sphere(0.0f, 1.0f, 0.0f));
        objList.push_back(new Sphere(0.0f, 0.0f, 1.0f));

        objList[0]->enabled = true;
        objList[1]->enabled = true;
        objList[2]->enabled = true;

        objList[0]->intensityScale = 0.5f;
        objList[1]->intensityScale = 0.5f;
        objList[2]->intensityScale = 0.5f;

        objList[0]->SetScale(0.2f, 0.2f, 0.2f);
        objList[1]->SetScale(0.2f, 0.2f, 0.2f);
        objList[2]->SetScale(0.2f, 0.2f, 0.2f);

        objList[0]->AssignFrequencyBin(5000, m_Recorder->sampleRate, N);
        objList[1]->AssignFrequencyBin(7500, m_Recorder->sampleRate, N);
        objList[2]->AssignFrequencyBin(10000, m_Recorder->sampleRate, N);
        showSpectrum = false;
        break;
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

