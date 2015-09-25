#include <QtGui>

#include "glwidget.h"
#include "window.h"


Window::Window()
{
    QGLFormat fmt;
    fmt.setDoubleBuffer(true);
    fmt.setSwapInterval(true);
    QGLFormat::setDefaultFormat(fmt);

    glWidget = new GLWidget();


    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget);

    setLayout(mainLayout);


    setWindowTitle(tr("VTViewer"));
}
