TEMPLATE = app
CONFIG += c++11 #console
CONFIG -= qt
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    graphics.h \
    logging.h \
    network.h

win32: LIBS += -lws2_32

win32: LIBS += -L$$PWD/'../../C++ resources/SDL2-2.0.9/x86_64-w64-mingw32/lib/' -lmingw32 -lSDL2main -lSDL2

INCLUDEPATH += $$PWD/'../../C++ resources/SDL2-2.0.9/x86_64-w64-mingw32/include/SDL2'
DEPENDPATH += $$PWD/'../../C++ resources/SDL2-2.0.9/x86_64-w64-mingw32/include/SDL2'

win32: LIBS += -L$$PWD/'../../C++ resources/SDL2_ttf-2.0.15/x86_64-w64-mingw32/lib/' -lSDL2_ttf

INCLUDEPATH += $$PWD/'../../C++ resources/SDL2_ttf-2.0.15/x86_64-w64-mingw32/include/SDL2'
DEPENDPATH += $$PWD/'../../C++ resources/SDL2_ttf-2.0.15/x86_64-w64-mingw32/include/SDL2'

win32: LIBS += -L$$PWD/'../../C++ resources/SDL2_mixer-2.0.4/x86_64-w64-mingw32/lib/' -lSDL2_mixer

INCLUDEPATH += $$PWD/'../../C++ resources/SDL2_mixer-2.0.4/x86_64-w64-mingw32/include/SDL2'
DEPENDPATH += $$PWD/'../../C++ resources/SDL2_mixer-2.0.4/x86_64-w64-mingw32/include/SDL2'

win32: LIBS += -L$$PWD/'../../C++ resources/SDL2_image-2.0.5/x86_64-w64-mingw32/lib/' -lSDL2_image

INCLUDEPATH += $$PWD/'../../C++ resources/SDL2_image-2.0.5/x86_64-w64-mingw32/include/SDL2'
DEPENDPATH += $$PWD/'../../C++ resources/SDL2_image-2.0.5/x86_64-w64-mingw32/include/SDL2'
