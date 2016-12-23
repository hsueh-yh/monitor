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
    src/consumer.cpp \
    src/face-wrapper.cpp \
    src/ff-decoder.cpp \
    src/frame-buffer.cpp \
    src/frame-data.cpp \
    src/glogger.cpp \
    src/jitter-timing.cpp \
    src/mtndn-library.cpp \
    src/mtndn-manager.cpp \
    src/mtndn-namespace.cpp \
    src/mtndn-object.cpp \
    src/mtndn-utils.cpp \
    src/name-components.cpp \
    src/pipeliner.cpp \
    src/playout.cpp \
    src/simple-log.cpp \
    src/statistics.cpp \
    src/video-consumer.cpp \
    src/video-decoder.cpp \
    src/video-playout.cpp \
    src/video-renderer.cpp \
    client/addstreamdialog.cpp \
    client/main.cpp \
    client/mainwindow.cpp \
    client/myTimer.cpp \
    client/renderer.cpp

HEADERS  += \
    include/glogger.h \
    include/interfaces.h \
    include/mtndn-defines.h \
    include/mtndn-library.h \
    include/name-components.h \
    include/params.h \
    include/simple-log.h \
    include/statistics.h \
    src/consumer.h \
    src/face-wrapper.h \
    src/ff-decoder.h \
    src/frame-buffer.h \
    src/frame-data.h \
    src/jitter-timing.h \
    src/mtndn-common.h \
    src/mtndn-manager.h \
    src/mtndn-namespace.h \
    src/mtndn-object.h \
    src/mtndn-utils.h \
    src/pipeliner.h \
    src/playout.h \
    src/renderer.h \
    src/tdll.h \
    src/video-consumer.h \
    src/video-decoder.h \
    src/video-playout.h \
    src/video-renderer.h \
    client/addstreamdialog.h \
    client/mainwindow.h \
    client/myTimer.h \
    client/renderer.h \
    client/simulator.h


CONFIG += C++11

FORMS += \
    client/addstreamdialog.ui \
    client/mainwindow.ui

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
    -ldl

OTHER_FILES += \
    README \
    monitor.pro.user


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/release/ -lavdevice
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/debug/ -lavdevice
else:unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lavdevice

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
