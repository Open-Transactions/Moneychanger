

#-------------------------------------------------
# Objects

MOC_DIR        = $${SOLUTION_DIR}../obj/$${TARGET}
OBJECTS_DIR    = $${SOLUTION_DIR}../obj/$${TARGET}


#-------------------------------------------------
# Output

RCC_DIR        = $${SOLUTION_DIR}../out/$${TARGET}/resources
UI_DIR         = $${SOLUTION_DIR}../out/$${TARGET}/ui/


#-------------------------------------------------
# Target

win32:{
DESTDIR = $${SOLUTION_DIR}../lib/$(PlatformName)/$(Configuration)
}

