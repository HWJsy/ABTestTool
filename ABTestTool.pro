QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/ui
INCLUDEPATH += $$PWD/core
INCLUDEPATH += $$PWD/ui/new
INCLUDEPATH += $$PWD/export
SOURCES += \
    core/functiondatamanager.cpp \
    core/groupdatamanager.cpp \
    core/groupmappingdatamanager.cpp \
    core/toplevelmanager.cpp \
    core/versiondatamanager.cpp \
    export/LoadConfig.cpp \
    export/exportcpp.cpp \
    main.cpp \
    mainwindow.cpp \
    ui/functiontogglewindow.cpp \
    ui/groupmanagewindow.cpp \
    ui/groupupdatemappingwindow.cpp \
    ui/new/newfunctionwindow.cpp \
    ui/new/newgroupmappingwindow.cpp \
    ui/new/newgroupwindow.cpp \
    ui/new/newmutexswitchwindow.cpp \
    ui/new/newversionwindow.cpp \
    ui/projsetting.cpp \
    ui/versionmanagewindow.cpp

HEADERS += \
    core/functiondatamanager.h \
    core/groupdatamanager.h \
    core/groupmappingdatamanager.h \
    core/toplevelmanager.h \
    core/versiondatamanager.h \
    export/LoadConfig.h \
    export/exportcpp.h \
    mainwindow.h \
    ui/functiontogglewindow.h \
    ui/groupmanagewindow.h \
    ui/groupupdatemappingwindow.h \
    ui/new/newfunctionwindow.h \
    ui/new/newgroupmappingwindow.h \
    ui/new/newgroupwindow.h \
    ui/new/newmutexswitchwindow.h \
    ui/new/newversionwindow.h \
    ui/projsetting.h \
    ui/versionmanagewindow.h

FORMS += \
    mainwindow.ui \
    ui/functiontogglewindow.ui \
    ui/groupmanagewindow.ui \
    ui/groupupdatemappingwindow.ui \
    ui/new/newfunctionwindow.ui \
    ui/new/newgroupmappingwindow.ui \
    ui/new/newgroupwindow.ui \
    ui/new/newmutexswitchwindow.ui \
    ui/new/newversionwindow.ui \
    ui/projsetting.ui \
    ui/versionmanagewindow.ui

RESOURCES += \
    resources.qrc

# Include template files in the app bundle
macx:QMAKE_BUNDLE_DATA += templateFolder
templateFolder.files = $$files($$PWD/template/*)
templateFolder.path = Contents/Resources/template

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
