TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
    netcommon.cpp \
    lock.cpp \
    cepoll.cpp \
    cbufque.cpp \
    ctcpserver.cpp

HEADERS += \
    common.h \
    netcommon.h \
    lock.h \
    cepoll.h \
    cbufque.h \
    ctcpserver.h

LIBS  += \
         -lpthread \

