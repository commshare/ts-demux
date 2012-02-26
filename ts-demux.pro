HEADERS += \
    SRC/TSParse.h \
    SRC/TSDemux.h \
    SRC/BitBuffer.h

SOURCES += \
    SRC/TSParse.c \
    SRC/TSDemux.c \
    SRC/BitBuffer.c


#If support 192, 204, 208 size packet, add following defination
DEFINES += TS_SUPPORT_192_PKT
DEFINES += TS_SUPPORT_204_PKT
DEFINES += TS_SUPPORT_208_PKT







