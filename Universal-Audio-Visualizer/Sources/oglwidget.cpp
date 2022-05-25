#include "oglwidget.h"
using namespace std;

OGLWidget::OGLWidget(QWidget *parent, AudioInterface* p)
    : QOpenGLWidget{parent}
{
    pInterface = p;
    pRecorder = pInterface->getRecorder();

    beatTimer = new QTimer(this);
    connect(beatTimer, SIGNAL(timeout()), this, SLOT(playBeat()));
}

OGLWidget::~OGLWidget()
{
    objList.clear();
}

void OGLWidget::initializeGL()
{
    float apsectRatio = (float)width() / (float)height();
    perspectiveMatrix = glm::perspective(glm::radians(60.0f), apsectRatio, 0.1f, 1000.0f);
    viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    unsigned int u_lightColor = glGetUniformLocation(shaderProgram.programId(), "u_lightColor");
    glUniform3f(u_lightColor, 1.0f, 1.0f, 1.0f);
    unsigned int u_lightDirection = glGetUniformLocation(shaderProgram.programId(), "u_lightDirection");
    glUniform3f(u_lightDirection, 1.0f, 1.0f, 1.0f);

    unsigned int u_projMatrix = glGetUniformLocation(shaderProgram.programId(), "u_ProjMatrix");
    glUniformMatrix4fv(u_projMatrix, 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));
    unsigned int u_ViewMatrix = glGetUniformLocation(shaderProgram.programId(), "u_ViewMatrix");
    glUniformMatrix4fv(u_ViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    loadPreset(0);
}

void OGLWidget::oglsetScale(float scale)
{
    this->defaultScale = scale;
}

void OGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pRecorder->dataSemaphore.acquire();
    pRecorder->ProcessData();

    if(pRecorder->bpm != 0 && !beatTimer->isActive() && !playBeatAnim)
    {
        smpl_t beatPeriod = 1 / (pRecorder->bpm / 60);
        beatTimer->start(beatPeriod * 1000);
    }

    if(!displayWaveform)
    {
        int objCount = 0;
        int updateCycle = 5;
        float newScale  = this->defaultScale;

        if(playBeatAnim)
        {
            newScale += this->defaultScale * pRecorder->GetVolume();
            playBeatAnim = false;
        }

        for(int i = 0; i < objList.size(); i++)
        {
            double magnitude = objList[i]->magnitude * objList[i]->intensityScale;
            if(drawCycleCount >= updateCycle)
            {
                //update max objects on screen
                magnitude = clamp(pRecorder->mag[objList[i]->freqBin], 0.0, maxMagnitude) * objList[i]->intensityScale;
                objList[i]->magnitude = clamp(pRecorder->mag[objList[i]->freqBin], 0.0, maxMagnitude);

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
                objList[i]->SetColor(determineColor(pRecorder->bpm));
                objList[i]->DrawShape(&shaderProgram);
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
                magnitude = pRecorder->mag[objList[i]->freqBin] / 20;
            }
            else
            {
                magnitude = objList[i]->scale.y;
            }

            magnitude = clamp(magnitude, 0.01f, 10.0f);
            objList[i]->SetScale(objList[i]->scale.x, magnitude, objList[i]->scale.z);
            objList[i]->SetColor(determineColor(pRecorder->bpm));

            objList[i]->DrawShape(&shaderProgram);
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
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vertex.glsl");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fragment.glsl");
    shaderProgram.link();
    shaderProgram.bind();
}

void OGLWidget::playBeat()
{
    beatTimer->stop();
    playBeatAnim = true;
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
    displayWaveform = false;
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

        break;
    }
    default:
        int binCounter = 0;
        float xPos = -1.0f;
        for(int i = 0; i < 200; i++)
        {
            Cube* c = new Cube(1.0f, 0.0f, 0.0f);

            c->SetTranslation(xPos, 0.0f, 0.0f);
            c->SetScale(0.01f, 0.01f, 0.01f);
            objList.push_back(c);
            c->freqBin = binCounter;
            binCounter += 2;
            xPos += 0.01f;

        }

        objList[0]->freqBin = 1;
        displayWaveform = true;
    }
}

void OGLWidget::createSphere(float r, float g, float b)
{
    Sphere* s = new Sphere(r, g, b);
    s->SetScale(0.2f);
    s->intensityScale = DEFAULTINTENSITY;
    s->AssignFrequencyBin(5000, pRecorder->sampleRate,
                             FRAMECOUNT);
    s->SetTranslation(-0.5f, 0.0f, 0.0f);
    objList.push_back(s);

}

void OGLWidget::createCube(float r, float g, float b)
{
    Cube *c = new Cube(r, g, b);
    c->SetScale(0.2f);
    c->intensityScale = DEFAULTINTENSITY;
    c->AssignFrequencyBin(1000, pRecorder->sampleRate,
                             FRAMECOUNT);
    c->SetTranslation(0.0f, 0.0f, 0.0f);
    objList.push_back(c);
}

void OGLWidget::createPrism(float r, float g, float b)
{
    Prism* p = new Prism(r, g, b);
    p->SetScale(0.2f);
    p->intensityScale = DEFAULTINTENSITY;
    p->AssignFrequencyBin(50, pRecorder->sampleRate,
                             FRAMECOUNT);
    p->SetTranslation(0.5f, 0.0f, 0.0f);
    objList.push_back(p);
}
