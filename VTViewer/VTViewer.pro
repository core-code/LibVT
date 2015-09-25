TARGET = VTViewer
TEMPLATE = app
CONFIG += qt
QT += opengl
QMAKE_CXXFLAGS+=-Wno-missing-field-initializers

macx {
        LIBS += -framework ApplicationServices -L../Dependencies/lib-mac/ -lboost_thread-mt
 }


INCLUDEPATH += ../Dependencies/
INCLUDEPATH += ../Dependencies/include/
INCLUDEPATH += ../Dependencies/source/
INCLUDEPATH += ../LibVT/

HEADERS = glwidget.h \
	window.h \
	../LibVT/LibVT.h \
	../LibVT/LibVT_Config.h \
	../LibVT/LibVT_Internal.h

SOURCES = glwidget.cpp \
	main.cpp \
	window.cpp \
	../Dependencies/source/stb_image.cc \
	../LibVT/LibVT.cpp \
	../LibVT/LibVT_Utilities.cpp \
	../LibVT/LibVT_ImageDecompression.cpp \
	../LibVT/LibVT_PageLoadingThread.cpp \
	../LibVT/LibVT_PageTable.cpp \
	../LibVT/LibVT_Readback.cpp \
	../LibVT/LibVT_Cache.cpp
RESOURCES  = resources.qrc

win32 {
        SOURCES += ../../Core3D/Utilities/opengl_win32.cpp
        HEADERS += ../../Core3D/Utilities/opengl_win32.h
        INCLUDEPATH += ../../Core3D/Utilities/
        LIBS += -L../Dependencies/lib-win32/ -lboost_thread-vc90-mt-1_40
 }
