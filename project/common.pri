
CONFIG     += debug_and_release

SOLUTION_DIR=$${PWD}/


win32:{
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

mac:MAC_OS_VERSION = $$system(uname -r)

