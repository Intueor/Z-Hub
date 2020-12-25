unix {
QMAKE_RPATHDIR += /usr/games/Z-Editor/lib
}
QT += core gui widgets

TARGET = Z-Hub
TEMPLATE = app

unix {
QMAKE_CXXFLAGS += -fno-strict-aliasing  -Wno-implicit-fallthrough
}

CONFIG += console
win32 {
CONFIG += no_batch
}

win32 {
LIBS += ..\\Z-Hub\\pthread\\lib\\pthreadVC2.lib \
	..\\Z-Hub\\win-libs\\Ws2_32.lib
}

win32 {
DEFINES += WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS
}

SOURCES += \
    Dialogs/set_proposed_bool_dialog.cpp \
    Dialogs/set_proposed_string_dialog.cpp \
    element.cpp \
    environment.cpp \
    group.cpp \
    link.cpp \
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
    Dialogs/set_server_dialog.cpp \
    Dialogs/message_dialog.cpp

HEADERS += \
    Dialogs/set_proposed_bool_dialog.h \
    Dialogs/set_proposed_string_dialog.h \
    element.h \
    environment.h \
    group.h \
    link.h \
    main-window.h \
    logger.h \
    main-hub-defs.h \
    p_buffer.h \
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
    Dialogs/set_server_dialog.h \
    Dialogs/message_dialog.h

FORMS += \
    Dialogs/set_proposed_bool_dialog.ui \
    Dialogs/set_proposed_string_dialog.ui \
    main-window.ui \
    Dialogs/set_server_dialog.ui \
    Dialogs/message_dialog.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    pthread/lib/pthreadVC2.lib \
    pthread/dll/pthreadVC2.dll \
    win-libs/WS2_32.Lib
