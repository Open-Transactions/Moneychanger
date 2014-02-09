HEADERS += \
    $${PWD}/axtls.h \
    $${PWD}/curl_darwinssl.h \
    $${PWD}/curl_schannel.h \
    $${PWD}/cyassl.h \
    $${PWD}/gskit.h \
    $${PWD}/gtls.h \
    $${PWD}/nssg.h \
    $${PWD}/openssl.h \
    $${PWD}/polarssl.h \
    $${PWD}/polarssl_threadlock.h \
    $${PWD}/qssl.h \
    $${PWD}/vtls.h

SOURCES += \
    $${PWD}/axtls.c \
    $${PWD}/curl_darwinssl.c \
    $${PWD}/curl_schannel.c \
    $${PWD}/cyassl.c \
    $${PWD}/gskit.c \
    $${PWD}/gtls.c \
    $${PWD}/nss.c \
    $${PWD}/openssl.c \
    $${PWD}/polarssl.c \
    $${PWD}/polarssl_threadlock.c \
    $${PWD}/qssl.c \
    $${PWD}/vtls.c


INCLUDEPATH += $${PWD}
