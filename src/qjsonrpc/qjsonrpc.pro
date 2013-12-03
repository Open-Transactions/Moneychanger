TEMPLATE = subdirs

SUBDIRS += src \
           tests
CONFIG += ordered

mac:{

        OS_VERSION = $$system(uname -r)
        QT_CONFIG -= no-pkg-config

    # -------------------------------------------
    # NOTE: This is necessary on Mac OSX Mavericks (10.9)
    #   Because libc++ is now chosen by default over libstdc++
    #   And your dependencies will have to be rebuilt with similar options.
    #
        contains(OS_VERSION, 13.0.0):QT_CONFIG += -spec macx-clang-libc++
        contains(OS_VERSION, 13.0.0):LIBS += -stdlib=libc++ -mmacosx-version-min=10.7
        contains(OS_VERSION, 13.0.0):CONFIG += c++11
        contains(OS_VERSION, 13.0.0):QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++ -std=c++11 -static
        contains(OS_VERSION, 13.0.0):QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

       contains(OS_VERSION, 13.0.0): MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
       contains(OS_VERSION, 13.0.0):if( !exists( $$MAC_SDK) ) {
          error("The selected Mac OSX SDK does not exist at $$MAC_SDK!")
        }
        contains(OS_VERSION, 13.0.0):QMAKE_MAC_SDK=macosx10.8
        contains(OS_VERSION, 13.0.0):INCLUDEPATH += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
        contains(OS_VERSION, 13.0.0):DEPENDPATH  += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
        contains(OS_VERSION, 13.0.0):LIBS += -framework CoreFoundation
    #
    # -------------------------------------------
}
