#-------------------------------------------------
#
# Project created by QtCreator 2016-02-23T21:30:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AndroidManager
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++1y

RC_FILE = resources.rc

SOURCES += main.cpp\
    mainwindow.cpp \
    copyprogress.cpp \
    fileitem.cpp \
    copyitem.cpp \
    copyqueue.cpp \
    filetable.cpp \
    fun.cpp

HEADERS  += mainwindow.h \
    copyprogress.h \
    fileitem.h \
    copyitem.h \
    copyqueue.h \
    filetable.h \
    fun.h

FORMS    += \
    mainwindow.ui
