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
    m_ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    initShaders();

    objList.push_back(new Prism(1.0f, 0.0f, 0.0f, 3));
    objList.push_back(new Cube(0.0f, 1.0f, 0.0f));
    objList.push_back(new Sphere(0.0f, 0.0f, 1.0f));

    objList[0]->SetTranslation(-0.75f, 0.0f, 0.0f);
    objList[1]->SetTranslation(0.0f, 0.0f, 0.0f);
    objList[2]->SetTranslation(0.75f, 0.0f, 0.0f);

    objList[0]->SetScale(0.2f, 0.2f, 0.2f);
    objList[1]->SetScale(0.2f, 0.2f, 0.2f);
    objList[2]->SetScale(0.2f, 0.2f, 0.2f);

    objList[0]->AssignFrequencyBin(500, m_Recorder->sampleRate, N);
    objList[1]->AssignFrequencyBin(5000, m_Recorder->sampleRate, N);
    objList[2]->AssignFrequencyBin(10000, m_Recorder->sampleRate, N);
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
        int magnitude = m_Recorder->mag[objList[i]->freqBin];
        magnitude = clamp(magnitude, 0, 10);

        //draw shapes based on magnitude of assigned frequency
        for(int j = 0; j < magnitude; j++)
        {
            //generate random y/z positions for the shapes
            float y = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
            float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            z *= -2;

            objList[i]->SetTranslation(objList[i]->m_Position.x, y, z);
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
