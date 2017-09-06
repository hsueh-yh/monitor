#-------------------------------------------------
#
# Project created by QtCreator 2016-07-05T08:42:51
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app


SOURCES += \
    renderer.cpp \
    myTimer.cpp \
    mainwindow.cpp \
    main.cpp \
    addstreamdialog.cpp \
    config.cpp

HEADERS  += \
    simulator.h \
    renderer.h \
    myTimer.h \
    mainwindow.h \
    addstreamdialog.h \
    config.h


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
    -lconfig++ \
    -lmtndn

OTHER_FILES += \
    README \
    client.pro.user


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../usr/local/lib/release/ -lconfig
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../usr/local/lib/debug/ -lconfig
else:unix: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -lconfig

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../usr/local/lib/release/ -lconfig++
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../usr/local/lib/debug/ -lconfig++
else:unix: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -lconfig++

INCLUDEPATH += $$PWD/../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../usr/local/include
