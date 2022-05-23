#include "oglwidget.h"
using namespace std;

OGLWidget::OGLWidget(QWidget *parent, AudioInterface* p)
    : QOpenGLWidget{parent}
{
    pInterface = p;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(playBeat()));
}

OGLWidget::~OGLWidget()
{
    objList.clear();
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

void OGLWidget::oglsetScale(float scale)
{
    this->scale = scale;
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pInterface->getRecorder()->dataSemaphore.acquire();
    pInterface->getRecorder()->ProcessData();

    smpl_t bpm = pInterface->getRecorder()->bpm;
    if(bpm != 0)
    {
        smpl_t beatPeriod = 1 / (bpm / 60);

        if(!timerRunning && !playBeatAnim)
        {
            timerRunning = true;
            timer->start(beatPeriod * 1000);
        }
    }
    if(!showSpectrum)
    {
        int objCount = 0;
        int updateCycle = 5;
        float newScale  = this->scale;
        if(playBeatAnim)
        {
            newScale += this->scale * 0.2;
            playBeatAnim = false;
        }

        for(int i = 0; i < objList.size(); i++)
        {
            double magnitude = objList[i]->m_Magnitude * objList[i]->intensityScale;
            if(drawCycleCount >= updateCycle)
            {
                //update max objects on screen
                magnitude = clamp(pInterface->getRecorder()->mag[objList[i]->freqBin], 0.0, maxMagnitude) * objList[i]->intensityScale;
                objList[i]->m_Magnitude = clamp(pInterface->getRecorder()->mag[objList[i]->freqBin], 0.0, maxMagnitude);

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
            //if object count exceeds current max, skip to next object type
            if(objCount >= magnitude)
            {
                int maxObjects = maxMagnitude * objList[i]->intensityScale;
                i += maxObjects - objCount;
                objCount = 0;
            }
            //otherwise update color and draw
            else
            {
                objList[i]->SetScale(newScale);
                objList[i]->SetColor(determineColor(pInterface->getRecorder()->bpm));
                objList[i]->DrawShape(&m_program);
                objCount++;
            }
        }
        if(drawCycleCount >= updateCycle)
            drawCycleCount = 0;

    }
    else
    {
        int updateCycle = 3;
        for(int i = 0; i < objList.size(); i++)
        {
            float magnitude;
            if(drawCycleCount >= updateCycle)
            {
                magnitude = pInterface->getRecorder()->mag[objList[i]->freqBin] / 20;
            }
            else
            {
                magnitude = objList[i]->m_Scale.y;
            }

            magnitude = clamp(magnitude, 0.01f, 10.0f);
            objList[i]->SetScale(objList[i]->m_Scale.x, magnitude, objList[i]->m_Scale.z);
            objList[i]->SetColor(determineColor(pInterface->getRecorder()->bpm));

            objList[i]->DrawShape(&m_program);
        }
        if(drawCycleCount >= updateCycle)
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

void OGLWidget::playBeat()
{
    timer->stop();
    playBeatAnim = true;
    timerRunning = false;
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
    objList.clear();

    int count = static_cast<int>(maxMagnitude * DEFAULTINTENSITY);
    switch(preset)
    {
    case 1:
    {
        for(int i = 0; i < count; i++)
        {
            createPrism(1.0f, 0.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createCube(0.0f, 1.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createSphere(0.0f, 0.0f, 1.0f);
        }


        showSpectrum = false;
        break;
    }
    case 2:{
        //Bass with triangle prism
        for(int i = 0; i < count; i++)
        {
            createPrism(1.0f, 0.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createPrism(0.0f, 1.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createPrism(0.0f, 0.0f, 1.0f);
        }

        showSpectrum = false;
        break;
    }
    case 3:{
        //Mids with cube
        for(int i = 0; i < count; i++)
        {
            createCube(1.0f, 0.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createCube(0.0f, 1.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createCube(0.0f, 0.0f, 1.0f);
        }

        showSpectrum = false;
        break;
    }
    case 4:{
        //Highs with sphere
        for(int i = 0; i < count; i++)
        {
            createSphere(1.0f, 0.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createSphere(0.0f, 1.0f, 0.0f);
        }

        for(int i = 0; i < count; i++)
        {
            createSphere(0.0f, 0.0f, 1.0f);
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

void OGLWidget::createSphere(float r, float g, float b)
{
    Sphere* temp = new Sphere(r, g, b);
    temp->SetScale(0.2f);
    temp->intensityScale = DEFAULTINTENSITY;
    temp->AssignFrequencyBin(5000, pInterface->getRecorder()->sampleRate,
                             FRAMECOUNT);
    temp->SetTranslation(-0.5f, 0.0f, 0.0f);
    objList.push_back(temp);

}

void OGLWidget::createCube(float r, float g, float b)
{
    Cube *temp = new Cube(r, g, b);
    temp->SetScale(0.2f);
    temp->intensityScale = DEFAULTINTENSITY;
    temp->AssignFrequencyBin(1000, pInterface->getRecorder()->sampleRate,
                             FRAMECOUNT);
    temp->SetTranslation(0.0f, 0.0f, 0.0f);
    objList.push_back(temp);
}

void OGLWidget::createPrism(float r, float g, float b)
{
    Prism* temp = new Prism(r, g, b);
    temp->SetScale(0.2f);
    temp->intensityScale = DEFAULTINTENSITY;
    temp->AssignFrequencyBin(50, pInterface->getRecorder()->sampleRate,
                             FRAMECOUNT);
    temp->SetTranslation(0.5f, 0.0f, 0.0f);
    objList.push_back(temp);
}
