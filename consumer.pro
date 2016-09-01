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
    frame-data.cpp

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
    tmp.hpp


CONFIG += C++11

FORMS    += mainwindow.ui \
    addstreamdialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/release/ -lavcodec
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/debug/ -lavcodec
else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lavcodec

INCLUDEPATH += $$PWD/ffmpeg/include
DEPENDPATH += $$PWD/ffmpeg/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/release/libavcodec.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/debug/libavcodec.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/release/avcodec.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/debug/avcodec.lib
else:unix: PRE_TARGETDEPS += $$PWD/ffmpeg/lib/libavcodec.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/release/ -lavutil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/debug/ -lavutil
else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lavutil

INCLUDEPATH += $$PWD/ffmpeg/include
DEPENDPATH += $$PWD/ffmpeg/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/release/libavutil.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/debug/libavutil.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/release/avutil.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/debug/avutil.lib
else:unix: PRE_TARGETDEPS += $$PWD/ffmpeg/lib/libavutil.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/release/ -lpostproc
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/debug/ -lpostproc
else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lpostproc

INCLUDEPATH += $$PWD/ffmpeg/include
DEPENDPATH += $$PWD/ffmpeg/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/release/libpostproc.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/debug/libpostproc.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/release/postproc.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/ffmpeg/lib/debug/postproc.lib
else:unix: PRE_TARGETDEPS += $$PWD/ffmpeg/lib/libpostproc.a

LIBS += -lboost_thread -lboost_system -lboost_chrono -lboost_regex -lndn-cpp -lpthread -lprotobuf -lsqlite3 -lcrypto -ldl

OTHER_FILES += \
    README
