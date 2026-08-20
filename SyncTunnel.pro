QT       += core gui
QT       += network mqtt multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

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
    businesslogic.cpp \
    core/basic/observable.cpp \
    core/basic/qcachedbytearray.cpp \
    core/basic/utils.cpp \
    core/services/schedule.cpp \
    dialogs/dialog_help.cpp \
    dialogs/dialog_remotefile.cpp \
    dialogs/dialog_schedule.cpp \
    dialogs/dialog_selectsyncdst.cpp \
    dialogs/wizard_startup.cpp \
    main.cpp \
    mainwindow.cpp \
    modules/communication/communication.cpp \
    modules/remotecontrol/remotecontrolengine.cpp \
    modules/remotecontrol/remotecontrolwidget.cpp \
    modules/rpepengine/congestioncontrol/congestioncontrol.cpp \
    modules/rpepengine/rpepengine.cpp \
    modules/signalling/signalling.cpp \
    modules/storage/storage.cpp \
    modules/transmissionengine/transmissionengine.cpp \
    viewmodel.cpp

HEADERS += \
    ../../libary/Qt-AES/qaesencryption.h \
    businesslogic.h \
    core/basic/observable.h \
    core/basic/qcachedbytearray.h \
    core/basic/utils.h \
    core/services/schedule.h \
    dialogs/dialog_help.h \
    dialogs/dialog_remotefile.h \
    dialogs/dialog_schedule.h \
    dialogs/dialog_selectsyncdst.h \
    dialogs/wizard_startup.h \
    general.h \
    mainwindow.h \
    modules/communication/communication.h \
    modules/remotecontrol/remotecontrolengine.h \
    modules/remotecontrol/remotecontrolwidget.h \
    modules/rpepengine/congestioncontrol/congestioncontrol.h \
    modules/rpepengine/rpepengine.h \
    modules/signalling/signalling.h \
    modules/storage/storage.h \
    modules/transmissionengine/transmissionengine.h \
    passport.h \
    viewmodel.h

FORMS += \
    dialog_remotefile.ui \
    dialogs/dialog_help.ui \
    dialogs/dialog_schedule.ui \
    dialogs/dialog_selectsyncdst.ui \
    dialogs/wizard_startup.ui \
    mainwindow.ui

#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/rapidjson/include/rapidjson
#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/websocketpp
#INCLUDEPATH += $$PWD/../../libary/boost_1_88_0/boost_1_88_0
#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/asio/asio/include
#INCLUDEPATH += $$PWD/../../libary/socket.io-client-cpp/lib/Catch2-3.8.1/Catch2-3.8.1/src/catch2
INCLUDEPATH += $$PWD/../../libary/Qt-AES
INCLUDEPATH += $$PWD/core/
INCLUDEPATH += $$PWD/core/services
INCLUDEPATH += $$PWD/core/basic
INCLUDEPATH += $$PWD
RC_FILE += rc_std.rc
win32{
    LIBS += -luser32 -lwinmm
}
win32: CONFIG -= console
win32: CONFIG += windows
#win32: DEFINES += QT_NEEDS_QMAIN
win32: QMAKE_LFLAGS += -Wl,-subsystem=windows
win32: QMAKE_LFLAGS += -Wl,-entry=WinMainCRTStartup
android{
    # QT += androidextras
}

#翻译设置
TRANSLATIONS += \
    trans_en_US.ts

#CONFIG += console  #仅调试
#DEFINES += NNPYRO_USE_CONSOLE   #仅调试
debug:DEFINES += DEBUG_NO_ENCRYPTION     #仅调试
DEFINES += NNPYRO_COLORFULCON   #控制台彩色输出
# debug:QMAKE_CXXFLAGS += -fsanitize=address
# debug:QMAKE_LFLAGS += -fsanitize=address
debug:QT+=testlib
debug:DEFINES += NNPYRO_PERFORMANCE_ANALYSIS  #性能分析

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rc1.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    rc_std.rc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
android {
    # 指定所有支持的ABI
    ANDROID_ABIS = armeabi-v7a arm64-v8a x86 x86_64
}
android: include($$PWD/../../libary/android_openssl-master/openssl.pri)
android: include(F:/Qt/Qt-android/openssl/android_openssl-master/openssl.pri)
