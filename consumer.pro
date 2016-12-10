#-------------------------------------------------
#
# Project created by QtCreator 2016-07-05T08:42:51
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = consumer
TEMPLATE = app


SOURCES += \
    src/consumer.cpp \
    src/decoder.cpp \
    src/face-wrapper.cpp \
    src/frame-buffer.cpp \
    src/frame-data.cpp \
    src/jitter-timing.cpp \
    src/logger.cpp \
    src/mtndn-library.cpp \
    src/mtndn-manager.cpp \
    src/name-components.cpp \
    src/namespacer.cpp \
    src/object.cpp \
    src/pipeliner.cpp \
    src/player.cpp \
    src/playout.cpp \
    src/statistics.cpp \
    src/utils.cpp \
    src/video-consumer.cpp \
    src/video-decoder.cpp \
    src/video-playout.cpp \
    src/video-renderer.cpp \
    client/renderer.cpp \
    client/addstreamdialog.cpp \
    client/controler.cpp \
    client/main.cpp \
    client/mainwindow.cpp \
    client/myTimer.cpp

HEADERS  += \
    include/interfaces.h \
    include/params.h \
    include/logger.h \
    include/mtndn-library.h \
    include/name-components.h \
    include/statistics.h \
    src/common.h \
    src/consumer.h \
    src/decoder.h \
    src/defines.h \
    src/face-wrapper.h \
    src/frame-buffer.h \
    src/frame-data.h \
    src/jitter-timing.h \
    src/logger.h \
    src/mtndn-manager.h \
    src/namespacer.h \
    src/object.h \
    src/pipeliner.h \
    src/player.h \
    src/playout.h \
    src/renderer.h \
    src/tdll.h \
    src/utils.h \
    src/video-consumer.h \
    src/video-decoder.h \
    src/video-playout.h \
    src/video-renderer.h \
    client/renderer.h \
    client/addstreamdialog.h \
    client/controler.h \
    client/mainwindow.h \
    client/myTimer.h \
    client/simulator.h \


CONFIG += C++11

FORMS += \
    client/addstreamdialog.ui \
    client/mainwindow.ui

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
    README \
    consumer.pro.user


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/release/ -lavdevice
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/debug/ -lavdevice
else:unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lavdevice

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
