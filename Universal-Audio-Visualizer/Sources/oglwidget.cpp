#include "oglwidget.h"
using namespace std;
OGLWidget::OGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    rotation = 0;
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);
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

    Cube* cube = new Cube(QVector3D(1.0f, 1.0f, 0.0f));
    cube->SetRotation(15, glm::vec3(1.0f, 1.0f, 0.0f));
    cube->SetTranslation(glm::vec3(0.5f, 0.5f, 0.0f));
    //cube->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

    Sphere* sphere = new Sphere(QVector3D(1.0f, 0.0f, 0.0f));
    sphere->SetTranslation(glm::vec3(0.0f, -0.5f, 0.0f));
    sphere->SetScale(glm::vec3(0.25f, 0.25f, 0.25f));

    Cylinder* cyl = new Cylinder(QVector3D(0.0f, 1.0f, 1.0f), 3);
    cyl->SetRotation(-45, glm::vec3(0.0f, 1.0f, 0.0f));
    cyl->SetTranslation(glm::vec3(-0.5f, 0.5f, 0.0f));
    cyl->SetScale(glm::vec3(0.35f, 0.35f, 0.35f));

    objList.push_back(cube);
    objList.push_back((sphere));
    objList.push_back(cyl);
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
        /*if(objList[i]->m_Scale.x() >= 1.0f)
            objList[i]->scaleFactor = 0.95f;
        else if(objList[i]->m_Scale.x() <= 0.25f)
            objList[i]->scaleFactor = 1.05f;*/

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_real_distribution<float> distr(0.25, 2); //define range
        objList[i]->scaleFactor = distr(gen);

        //clamp scale factor
        if(objList[i]->scaleFactor * objList[i]->m_Scale.x() > 1.25f || objList[i]->scaleFactor * objList[i]->m_Scale.x() < 0.25f)
            objList[i]->scaleFactor = 1.0f;

        objList[i]->SetScale(glm::vec3(objList[i]->scaleFactor, objList[i]->scaleFactor, objList[i]->scaleFactor));
        objList[i]->DrawShape(&m_program);
    }
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
