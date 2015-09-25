#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QtOpenGL>

#include "window.h"
#include "LibVT.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    while (1)
    {
        QString fileName = QFileDialog::getExistingDirectory(0, QString("Open Virtual Texture Tile Store Directory"), QString(""));
        if (fileName == "") exit(1);

        char ext [5] = "    ";
        uint8_t border, length;
        uint32_t dim;
        bool success = vtScan(fileName.toUtf8(), ext, &border, &length, &dim);

        if (success)
        {
#if (LONG_MIP_CHAIN)
            if (length > 9)
#else
                if ((length > 0) && (length <= 9))
#endif
                {
                vtInit(fileName.toUtf8(), ext, border, length, dim);
                break;
            }
        }
    }
    Window window;
    window.showMaximized();

    return app.exec();
}
