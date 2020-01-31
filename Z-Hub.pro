unix {
QMAKE_RPATHDIR += /usr/games/Z-Editor/lib
}
QT += core gui widgets

TARGET = Z-Editor
TEMPLATE = app

unix {
QMAKE_CXXFLAGS += -fno-strict-aliasing
}

CONFIG += console
win32 {
CONFIG += no_batch
}

win32 {
LIBS += ..\\Z-Editor\\pthread\\lib\\pthreadVC2.lib
}

win32 {
DEFINES += WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS
}

SOURCES += \
    main.cpp \
    main-window.cpp \
    main-hub.cpp \
    tester.cpp \
    TinyXML2/tinyxml2.cpp \
    dlfcn-win32/dlfcn.c \
    parser-ext.cpp \
    Server/net-hub.cpp \
    Server/proto-parser.cpp \
    Server/proto-util.cpp \
    Server/server.cpp \
    Dialogs/message-dialog.cpp

HEADERS += \
    main-window.h \
    logger.h \
    main-hub-defs.h \
    tester.h \
    dlfcn-win32/dlfcn.h \
    pthread/include/pthread.h \
    pthread/include/sched.h \
    pthread/include/semaphore.h \
    TinyXML2/tinyxml2.h \
    main-hub.h \
    parser-ext.h \
    Server/net-hub-defs.h \
    Server/net-hub.h \
    Server/proto-parser.h \
    Server/proto-util.h \
    Server/protocol.h \
    Server/server.h \
    z-hub-defs.h \
    Dialogs/message-dialog.h

FORMS += \
    main-window.ui \
    Dialogs/message-dialog.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    pthread/lib/pthreadVC2.lib \
    pthread/dll/pthreadVC2.dll
