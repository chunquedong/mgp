QT += core gui widgets opengl openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG += c++11 console
CONFIG(debug, debug|release): DEFINES += _DEBUG

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mgpview.cpp

HEADERS += \
    mainwindow.h \
    mgpview.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += src \
  $$(FMAKE_REPO)/lib/cpp/glew-2.2.0-debug/include \
  $$(FMAKE_REPO)/lib/cpp/openal-1.22.2-debug/include \
  $$(FMAKE_REPO)/lib/cpp/bullet-3.24-debug/include \
  $$(FMAKE_REPO)/lib/cpp/freetype-2.4.12-debug/include \
  $$(FMAKE_REPO)/lib/cpp/jsonc-2.0-debug/include \
  $$(FMAKE_REPO)/lib/cpp/ljs-1.0-debug/include \

CONFIG(release, debug|release): INCLUDEPATH += \
  $$(FMAKE_REPO)/lib/cpp/mgpCore-1.0-release/include \
  $$(FMAKE_REPO)/lib/cpp/mgpModules-1.0-release/include \
  $$(FMAKE_REPO)/lib/cpp/mgpPro-1.0-release/include \

CONFIG(debug, debug|release): INCLUDEPATH += \
  $$(FMAKE_REPO)/lib/cpp/mgpCore-1.0-debug/include \
  $$(FMAKE_REPO)/lib/cpp/mgpModules-1.0-debug/include \
  $$(FMAKE_REPO)/lib/cpp/mgpPro-1.0-debug/include \

CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/glew-2.2.0-release/lib
CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/openal-1.22.2-release/lib
CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/bullet-3.24-release/lib
CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/freetype-2.4.12-release/lib
CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/jsonc-2.0-release/lib
CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/ljs-1.0-release/lib
CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/mgpCore-1.0-release/lib
CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/mgpModules-1.0-release/lib
# CONFIG(release, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/mgpPro-1.0-release/lib

CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/glew-2.2.0-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/openal-1.22.2-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/bullet-3.24-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/freetype-2.4.12-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/jsonc-2.0-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/ljs-1.0-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/mgpCore-1.0-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/mgpModules-1.0-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/mgpPro-1.0-debug/lib
CONFIG(debug, debug|release): LIBS += -L$$(FMAKE_REPO)/lib/cpp/glfw-3.3.8-debug/lib

# LIBS += -lmgpPro
LIBS += -lmgpModules
LIBS += -lmgpCore
LIBS += -lglew
LIBS += -lopenal
LIBS += -lbullet
LIBS += -lfreetype
LIBS += -ljsonc
LIBS += -lljs
LIBS += -lglfw

win32 {
    DEFINES += _WINDOWS _WIN32
    DEFINES += VK_USE_PLATFORM_WIN32_KHR
    INCLUDEPATH += $$(VULKAN_SDK)/Include
    LIBS += -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -limm32 -limagehlp -lversion -lwinmm -lxinput
#    LIBS += -L$$(VULKAN_SDK)/Lib -lvulkan-1
    LIBS += -lOpenGL32 -lGLU32
    QMAKE_CXXFLAGS_WARN_ON -= -w34100
    QMAKE_CXXFLAGS_WARN_ON -= -w34189
    QMAKE_CXXFLAGS_WARN_ON -= -w4302
    QMAKE_CXXFLAGS_WARN_ON -= -w4311
    QMAKE_CXXFLAGS_WARN_ON -= -w4244
}

linux {
    DEFINES += SDL_VIDEO_DRIVER_X11
    DEFINES += VK_USE_PLATFORM_XLIB_KHR
    QMAKE_CXXFLAGS += -lstdc++ -pthread -w
    INCLUDEPATH += /usr/include/gtk-2.0
    INCLUDEPATH += /usr/lib/x86_64-linux-gnu/gtk-2.0/include
    INCLUDEPATH += /usr/include/atk-1.0
    INCLUDEPATH += /usr/include/cairo
    INCLUDEPATH += /usr/include/gdk-pixbuf-2.0
    INCLUDEPATH += /usr/include/pango-1.0
    INCLUDEPATH += /usr/include/gio-unix-2.0
    INCLUDEPATH += /usr/include/freetype2
    INCLUDEPATH += /usr/include/glib-2.0
    INCLUDEPATH += /usr/lib/x86_64-linux-gnu/glib-2.0/include
    INCLUDEPATH += /usr/include/pixman-1
    INCLUDEPATH += /usr/include/libpng12
    INCLUDEPATH += /usr/include/harfbuzz
    INCLUDEPATH += $$(VULKAN_SDK)/include
    LIBS += -lrt -ldl -lX11 -lpthread  -lxcb
    #LIBS += -lgtk-x11-2.0 -lglib-2.0 -lgobject-2.0 -lsndio
    #LIBS += -L$$(VULKAN_SDK)/lib/ -lvulkan
}

macx {
    DEFINES += VK_USE_PLATFORM_MACOS_MVK
    INCLUDEPATH += $$(HOME)/vulkansdk-macos-1.0.69.0/macOS/include
    LIBS += -L/usr/lib -liconv
    LIBS += -F$$(HOME)/vulkansdk-macos-1.0.69.0/MoltenVK/macOS -framework MoltenVK
    LIBS += -F/System/Library/Frameworks -framework Metal
    LIBS += -F/System/Library/Frameworks -framework MetalKit
    LIBS += -F/System/Library/Frameworks -framework GameKit
    LIBS += -F/System/Library/Frameworks -framework IOKit
    LIBS += -F/System/Library/Frameworks -framework IOSurface
    LIBS += -F/System/Library/Frameworks -framework ForceFeedback
    LIBS += -F/System/Library/Frameworks -framework OpenAL
    LIBS += -F/System/Library/Frameworks -framework CoreAudio
    LIBS += -F/System/Library/Frameworks -framework AudioToolbox
    LIBS += -F/System/Library/Frameworks -framework QuartzCore
    LIBS += -F/System/Library/Frameworks -framework Carbon
    LIBS += -F/System/Library/Frameworks -framework Cocoa
    LIBS += -F/System/Library/Frameworks -framework Foundation
    QMAKE_MACOSX_DEPLOYMENT_TARGET=10.13
    QMAKE_CXXFLAGS += -x c++ -x objective-c++ -stdlib=libc++ -w -arch x86_64
    res.files = res
    res.path = Contents/Resources
    QMAKE_BUNDLE_DATA += res
}
