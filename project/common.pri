
CONFIG     += debug_and_release

SOLUTION_DIR=$${PWD}/


win32:{


equals(TEMPLATE,vcapp)|equals(TEMPLATE,vclib):{

#-------------------------------------------------
# Target
DESTDIR = $${SOLUTION_DIR}../lib/$(PlatformName)/$(Configuration)

#-------------------------------------------------
# Objects

MOC_DIR        = $${SOLUTION_DIR}../obj/$${TARGET}
OBJECTS_DIR    = $${SOLUTION_DIR}../obj/$${TARGET}


#-------------------------------------------------
# Output

RCC_DIR        = $${SOLUTION_DIR}../out/$${TARGET}/resources
UI_DIR         = $${SOLUTION_DIR}../out/$${TARGET}/ui/

}
else:{
#-------------------------------------------------
# Target (no Visual Studio)
    CONFIG(debug, debug|release):{
        DESTDIR = $${SOLUTION_DIR}../lib/Win32/Debug
    }
    else:{
        DESTDIR = $${SOLUTION_DIR}../lib/Win32/Debug
    }
}

LIBPATH += $${DESTDIR}

}

