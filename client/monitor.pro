#-------------------------------------------------
#
# Project created by QtCreator 2016-07-05T08:42:51
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = monitor
TEMPLATE = app


SOURCES += \
    renderer.cpp \
    myTimer.cpp \
    mainwindow.cpp \
    main.cpp \
    addstreamdialog.cpp

HEADERS  += \
    simulator.h \
    renderer.h \
    myTimer.h \
    mainwindow.h \
    addstreamdialog.h


CONFIG += C++11

FORMS += \
    addstreamdialog.ui \
    mainwindow.ui \
    mainwindow.ui \
    addstreamdialog.ui

LIBS +=  \
    -lboost_thread \
    -lboost_system \
    -lboost_chrono \
    -lboost_regex \
    -lndn-cpp \
    -lpthread \
    -lprotobuf \
    -lsqlite3 \
    -lcrypto \
    -lglog \
    -lmtndn

OTHER_FILES += \
    README \
    monitor.pro.user

