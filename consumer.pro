#-------------------------------------------------
#
# Project created by QtCreator 2016-07-05T08:42:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = consumer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    consumer.cpp \
    controler.cpp \
    decoder.cpp \
    face-wrapper.cpp \
    frame-buffer.cpp \
    object.cpp \
    pipeliner.cpp \
    player.cpp \
    utils.cpp \
    addstreamdialog.cpp \
    myTimer.cpp \
    frame-data.cpp \
    name-components.cpp \
    namespacer.cpp \
    statistics.cpp

HEADERS  += mainwindow.h \
    common.h \
    consumer.h \
    controler.h \
    decoder.h \
    face-wrapper.h \
    frame-buffer.h \
    frame-data.h \
    object.h \
    pipeliner.h \
    player.h \
    tdll.h \
    utils.h \
    addstreamdialog.h \
    simulator.h \
    myTimer.h \
    name-components.h \
    namespacer.h \
    logger.hpp \
    statistics.hpp


CONFIG += C++11

FORMS    += mainwindow.ui \
    addstreamdialog.ui

LIBS +=  \
    -lboost_thread  \
    -lboost_system  \
    -lboost_chrono  \
    -lboost_regex  \
    -lndn-cpp  \
    -lpthread  \
    -lprotobuf  \
    -lsqlite3  \
    -lcrypto  \
    -lglog \
    -ldl

OTHER_FILES += \
    README
