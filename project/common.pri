
CONFIG     += debug_and_release silent

SOLUTION_DIR=$${PWD}/

INCLUDEPATH += $${SOLUTION_DIR}../src
INCLUDEPATH += $${SOLUTION_DIR}../src/jsoncpp

#-------------------------------------------------
# Output

win32:{
    INCLUDEPATH += $${SOLUTION_DIR}../src/curl/include
    INCLUDEPATH += $${SOLUTION_DIR}../src/libidn/libidn
    INCLUDEPATH += $${SOLUTION_DIR}../src/libidn/windows/include

    equals(TEMPLATE,vcapp)|equals(TEMPLATE,vclib):{
        DESTDIR     = $${SOLUTION_DIR}../lib/$(PlatformName)/$(Configuration)
        MOC_DIR     = $${SOLUTION_DIR}../obj/$${TARGET}
        OBJECTS_DIR = $${SOLUTION_DIR}../obj/$${TARGET}
        RCC_DIR     = $${SOLUTION_DIR}../out/$${TARGET}/resources
        UI_DIR      = $${SOLUTION_DIR}../out/$${TARGET}/ui/
    }
    else:{
        !contains(QMAKE_HOST.arch, x86_64):{
            CONFIG(debug, debug|release):{
                DESTDIR = $${SOLUTION_DIR}../lib/Win32/Debug
            }
            else:{
                DESTDIR = $${SOLUTION_DIR}../lib/Win32/Debug
            }
        }
        else:{
            CONFIG(debug, debug|release):{
                DESTDIR = $${SOLUTION_DIR}../lib/x64/Debug
            }
            else:{
                DESTDIR = $${SOLUTION_DIR}../lib/x64/Debug
            }
        }
    }
}


#-------------------------------------------------
# Options

QMAKE_CFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
QMAKE_CXXFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration

win32:{
    DEFINES     += "_UNICODE" "NOMINMAX"
    CharacterSet = 1
    QMAKE_CXXFLAGS += /bigobj /Zm480 /wd4512 /wd4100
}


unix:{
    mac:{
        MAC_OS_VERSION = $$system(sw_vers -productVersion)
        MAC_OS_VERSION ~= s/\([0-9]*.[0-9]*\).*/\1/

        QMAKE_MACOSX_DEPLOYMENT_TARGET = $${MAC_OS_VERSION}


        #
        # note: copied from unix section, since this stuff WAS happening on mac anyway.
        ##
        CONFIG += link_pkgconfig
        QMAKE_CXXFLAGS += -fPIC     ## put only here, sub-libs pick it up from elsewhere?
        # end: copied from unix section.


        QT_CONFIG -= no-pkg-config

        #we do it this way, since we don't want any more tokens.
        PKG_CONFIG_LIBDIR = "/usr/local/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
        PKG_CONFIG_LIBDIR = "/usr/local/opt/openssl/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
        PKG_CONFIG_LIBDIR = "$${PKG_CONFIG_LIBDIR}:" #end with a colon.


        contains(MAC_OS_VERSION, 10.9):{
            # NOTE: below line copied from unix section:
            QMAKE_CXXFLAGS += -std=c++11 -DCXX_11

            CONFIG += c++11
        }
        else:{
            # NOTE: below line copied from unix section:
            QMAKE_CXXFLAGS += -std=c++03 -Dnullptr=NULL -DOT_USE_TR1

            MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
            QMAKE_MAC_SDK=macosx10.8
            !exists($$MAC_SDK): error("The selected Mac OSX SDK does not exist at $${MAC_SDK}!")
        }
    }
    
    # LINUX:
    else:{
        GCC_VERSION = $$system(g++ -dumpversion | cut -d. -f1-2)

        CONFIG += link_pkgconfig
        QMAKE_CXXFLAGS += -fPIC ## put only here, sub-libs pick it up from elsewhere?

        lessThan(GCC_VERSION, 4.7):{
            QMAKE_CXXFLAGS += -std=c++03 -Dnullptr=NULL -DOT_USE_TR1
        }
        else:{
            QMAKE_CXXFLAGS += -std=c++11 -DCXX_11
        }
    }
}



