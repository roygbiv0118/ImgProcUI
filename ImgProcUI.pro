######################################################################
# Automatically generated by qmake (3.0) ?? ?? 9 14:52:15 2017
######################################################################

TEMPLATE = app
TARGET = ImgProcUI
INCLUDEPATH += .

# Input
HEADERS += build/IPUI/ui_ipui.h \
           ImageProcess/include/ImageProcess.h \
           ImageProcess/include/stdafx.h \
           ImageProcess/include/targetver.h \
           IPUI/include/ipui.h \
           IPUI/include/rgblabel.h \
           RgbProcess/include/RgbProcess.h \
           RgbProcess/include/stdafx.h \
           RgbProcess/include/targetver.h \
           build/IPUI/moc_ipui.cpp \
           build/IPUI/moc_rgblabel.cpp
FORMS += IPUI/form/ipui.ui
SOURCES += build/CMakeFiles/feature_tests.cxx \
           build/IPUI/IPUI_automoc.cpp \
           ImageProcess/src/dllmain.cpp \
           ImageProcess/src/ImageProcess.cpp \
           ImageProcess/src/stdafx.cpp \
           IPUI/src/ipui.cpp \
           IPUI/src/main.cpp \
           IPUI/src/rgblabel.cpp \
           RgbProcess/src/dllmain.cpp \
           RgbProcess/src/RgbProcess.cpp \
           RgbProcess/src/stdafx.cpp \
           build/CMakeFiles/3.5.0-rc3/CompilerIdC/CMakeCCompilerId.c \
           build/CMakeFiles/3.5.0-rc3/CompilerIdCXX/CMakeCXXCompilerId.cpp
RESOURCES += IPUI/form/ipui.qrc
TRANSLATIONS += Internationalize/mx_zh.ts
