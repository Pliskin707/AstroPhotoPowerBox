QT += quick
QT += network

CONFIG += c++11
CONFIG += qmltypes

QML_IMPORT_NAME = Pliskin.Astroboxgui.Backend
QML_IMPORT_MAJOR_VERSION = 1

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        backend.cpp \
        esp_comm/esp_comm.cpp \
        logger/logger.cpp \
        main.cpp \
        settings.cpp

RESOURCES += qml.qrc

TRANSLATIONS += \
    AstroBoxGUI_de_DE.ts
CONFIG += lrelease
CONFIG += embed_translations

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    backend.hpp \
    esp_comm/esp_comm.hpp \
    logger/logger.h \
    settings.hpp

DEFINES= QZEROCONF_STATIC

INCLUDEPATH += $$PWD/qtzeroconf
DEPENDPATH += $$PWD/qtzeroconf

include(qtzeroconf/qtzeroconf.pri)
