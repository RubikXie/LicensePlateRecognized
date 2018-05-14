#-------------------------------------------------
#
# Project created by QtCreator 2018-05-12T09:34:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GraduationDesignDisplay
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    chinese.cpp \
    myocr.cpp \
    platelocation.cpp \
    segment.cpp \
    svm.cpp

HEADERS  += mainwidget.h \
    chinese.h \
    myocr.h \
    platelocation.h \
    segment.h \
    svm.h

FORMS    += mainwidget.ui


INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/include/opencv
INCLUDEPATH += /usr/local/include/opencv2
INCLUDEPATH += /usr/local/include/tesseract
LIBS += -L/usr/local/lib \
 -lopencv_core \
 -lopencv_highgui \
 -lopencv_imgproc \
 -lopencv_imgcodecs \
 -lopencv_ml \
 -ltesseract.3 \
 -ltesseract \
