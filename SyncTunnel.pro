QT       += core gui
QT       += network mqtt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

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
    ../../libary/Qt-AES/qaesencryption.cpp \
    main.cpp \
    mainwindow.cpp \
    modules/communication/communication.cpp \
    modules/signalling/signalling.cpp \
    modules/storage/storage.cpp

HEADERS += \
    ../../libary/Qt-AES/qaesencryption.h \
    mainwindow.h \
    modules/communication/communication.h \
    modules/signalling/signalling.h \
    modules/storage/storage.h

FORMS += \
    dialog_deviceList.ui \
    mainwindow.ui

#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/rapidjson/include/rapidjson
#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/websocketpp
#INCLUDEPATH += $$PWD/../../libary/boost_1_88_0/boost_1_88_0
#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/asio/asio/include
#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/Catch2-3.8.1/Catch2-3.8.1/src/catch2
INCLUDEPATH += $$PWD../../libary/Qt-AES
RC_FILE += rc_std.rc
LIBS += -luser32

#CONFIG += console  #仅调试

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=

DISTFILES += \
    rc_std.rc
