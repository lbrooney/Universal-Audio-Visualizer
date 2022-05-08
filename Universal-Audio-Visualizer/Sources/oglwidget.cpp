#include "oglwidget.h"
using namespace std;

OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);

    m_Recorder = new AudioRecorder();
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
    m_ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();
    Prism* p = new Prism(1.0f, 0.0f, 0.0f, 3);
    objList.push_back(p);
    objList.push_back(new Cube(0.0f, 1.0f, 0.0f));
    objList.push_back(new Sphere(0.0f, 0.0f, 1.0f));

    objList[0]->SetTranslation(-0.5f, 0.0f, 0.0f);
    objList[1]->SetTranslation(0.0f, 0.0f, 0.0f);
    objList[2]->SetTranslation(0.5f, 0.0f, 0.0f);

    objList[0]->SetScale(0.5f, 0.5f, 0.5f);
    objList[1]->SetScale(0.5f, 0.5f, 0.5f);
    objList[2]->SetScale(0.5f, 0.5f, 0.5f);

    objList[0]->AssignFrequencyBin(500, m_Recorder->sampleRate, N);
    objList[1]->AssignFrequencyBin(5000, m_Recorder->sampleRate, N);
    objList[2]->AssignFrequencyBin(10000, m_Recorder->sampleRate, N);

    /*int binCounter = 0;
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
    objList[0]->freqBin = 1;*/

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
        float magnitude = m_Recorder->mag[objList[i]->freqBin];
        cout << m_Recorder->mag[objList[0]->freqBin] << " ";
        magnitude = clamp(magnitude, 0.0f, 15.0f);
        objList[i]->SetScale(objList[i]->m_Scale.x, magnitude, objList[i]->m_Scale.z);
        objList[i]->DrawShape(&m_program);
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
