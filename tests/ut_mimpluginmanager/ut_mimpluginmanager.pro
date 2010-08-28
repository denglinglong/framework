OBJECTS_DIR = .obj
MOC_DIR = .moc

INCLUDEPATH += ../../src \
               ../stubs \
               ../dummyimplugin \
               ../dummyimplugin3 \

SRC_DIR = ../../src


# Input
HEADERS += \
    ut_mimpluginmanager.h \
    ../stubs/mgconfitem_stub.h \
    ../stubs/fakegconf.h \
    ../stubs/minputcontextconnection_stub.h \
    $$SRC_DIR/mimpluginmanager_p.h \
    $$SRC_DIR/minputmethodplugin.h \
    $$SRC_DIR/minputcontextconnection.h \

SOURCES += \
    ut_mimpluginmanager.cpp \
    ../stubs/fakegconf.cpp \
    ../stubs/minputcontextconnection_stub.cpp \
    $$SRC_DIR/minputcontextconnection.cpp \


CONFIG += debug plugin meegotouch qdbus

LIBS += \
    ../../src/libmeegoimframework.so.0 \
    -L ../plugins/ \
    -ldummyimplugin \
    -ldummyimplugin3 \

include(../common_check.pri)
