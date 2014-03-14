

INCLUDEPATH += $${PWD}

OTHER_FILES += \
    $${PWD}/idn.map

HEADERS += \
    $${PWD}/gunibreak.h \
    $${PWD}/gunicomp.h \
    $${PWD}/gunidecomp.h \
    $${PWD}/idna.h \
    $${PWD}/idn-free.h \
    $${PWD}/pr29.h \
    $${PWD}/punycode.h \
    $${PWD}/stringprep.h \
    $${PWD}/tld.h

SOURCES += \
    $${PWD}/idna.c \
    $${PWD}/idn-free.c \
    $${PWD}/nfkc.c \
    $${PWD}/pr29.c \
    $${PWD}/profiles.c \
    $${PWD}/punycode.c \
    $${PWD}/rfc3454.c \
    $${PWD}/strerror-idna.c \
    $${PWD}/strerror-pr29.c \
    $${PWD}/strerror-punycode.c \
    $${PWD}/strerror-stringprep.c \
    $${PWD}/strerror-tld.c \
    $${PWD}/stringprep.c \
    $${PWD}/tld.c \
    $${PWD}/tlds.c \
    $${PWD}/toutf8.c \
    $${PWD}/version.c
