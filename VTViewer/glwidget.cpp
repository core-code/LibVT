#include <QtGui>
#include <QtOpenGL>

#include "glwidget.h"

#include "LibVT_Internal.h"
#include "LibVT.h"
#include "LibVT_Config.h"



GLWidget::GLWidget(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    xPos = 0;
    yPos = 0;
    zoom = 10;
}

GLWidget::~GLWidget()
{
    vtShutdown();
}

void GLWidget::initializeGL()
{
    qglClearColor(Qt::black);

    glEnable(GL_TEXTURE_2D);

    // texture = bindTexture(QPixmap(QString("/Users/julian/Documents/Development/VirtualTexturing/_texdata_sources/texture_1k.png")), GL_TEXTURE_2D);




    char *prelude = vtGetShaderPrelude();

    readbackShader = loadShader(QString("readback"), QString(prelude));
    renderVTShader = loadShader(QString("renderVT"), QString(prelude));

    renderVTShader->setUniformValue("mipcalcTexture", TEXUNIT_FOR_MIPCALC);

    free(prelude);

    vtPrepare(readbackShader->programId(), renderVTShader->programId());

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(20);
}

void GLWidget::animate()
{
    updateGL();
}

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-5.0, +5.0, -5.0 * (float)height / (float)width, +5.0 * (float)height / (float)width, 0.1, 9999.0);

    glMatrixMode(GL_MODELVIEW);

    vtReshape(width, height, 0.0f, 0.1f, 9999.0f);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(xPos * zoom / 100.0, yPos * zoom / 100.0, 0);
    glColor3f(1.0, 1.0, 1.0);

    vtPrepareReadback();

    readbackShader->bind();

#if !USE_MIPCALC_TEXTURE
    readbackShader->setUniformValue("mip_calculation_bias", 0.0f - PREPASS_RESOLUTION_REDUCTION_SHIFT);
#endif
    readbackShader->setUniformValue("mip_debug_bias", 0.0f);

    renderQuad();



    vtPerformReadback();

    vtExtractNeededPages(NULL);

    vtMapNewPages();


    renderVTShader->bind();

    renderQuad();


    //   glBindTexture(GL_TEXTURE_2D, texture);
    //    renderQuad();
}

void GLWidget::renderQuad()
{
    int size = zoom;
    GLshort vertices[] = {-size,  size, -101,	-size, -size, -101,	size,  size, -101,     size, -size, -101};
    const GLfloat texCoords[] = { 0.0,1.0,          0.0,0.0,            1.0,1.0,                    1.0,0.0};
    const GLubyte indices[] = {0,1,2, 1,2,3};


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    glVertexPointer(3, GL_SHORT, 0, vertices);


    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    float dx = event->x() - lastPos.x();
    float dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        _setXPosition(xPos + dx / zoom);
        _setYPosition(yPos + -dy / zoom);
    } else if (event->buttons() & Qt::RightButton) {
        _setZoom(zoom + dx / 5.0);
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent (QWheelEvent *event)
{
    if (event->delta() > 0)
        _setZoom(zoom * 1.1);
    else
        _setZoom(zoom * 0.9);

}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

void GLWidget::_setXPosition(float position)
{
    if (position > 100) position = 100;
    if (position < -100) position = -100;

    if (position != xPos) {
        xPos = position;
        updateGL();
    }
}

void GLWidget::_setYPosition(float position)
{
    if (position > 100) position = 100;
    if (position < -100) position = -100;

    if (position != yPos) {
        yPos = position;
        updateGL();
    }
}

void GLWidget::_setZoom(float _zoom)
{
    //if (_zoom > 200) _zoom = 120;
    if (_zoom < 1.3) _zoom = 1.3f;

    if (_zoom != zoom) {
        zoom = _zoom;
        updateGL();
    }
}

QGLShaderProgram* GLWidget::loadShader(QString name, QString prelude)
{
    QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
    QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);

    QFile vertFile(QString(":/%1.vert").arg(name));
    vertFile.open(QIODevice::ReadOnly);
    QString vertString = QString::fromUtf8(vertFile.readAll());
    vertString.prepend(prelude);

    vshader->compileSourceCode(vertString);

    QFile fragFile(QString(":/%1.frag").arg(name));
    fragFile.open(QIODevice::ReadOnly);
    QString fragString =  QString::fromUtf8(fragFile.readAll());
    fragString.prepend(prelude);

    fshader->compileSourceCode(fragString);

    QGLShaderProgram *shader = new QGLShaderProgram(this);
    shader->addShader(vshader);
    shader->addShader(fshader);
    shader->link();
    shader->bind();

    return shader;
}
