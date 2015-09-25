#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QtOpenGL>

class QtLogo;

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;


protected:
    void _setXPosition(float position);
    void _setYPosition(float position);
    void _setZoom(float zoom);
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    QGLShaderProgram* loadShader(QString name, QString prelude);
    void renderQuad();

private slots:
    void animate();

private:
    float xPos;
    float yPos;
    float zoom;
    QPoint lastPos;
    QGLShaderProgram *readbackShader;
    QGLShaderProgram *renderVTShader;
    GLuint texture;
};
#endif
