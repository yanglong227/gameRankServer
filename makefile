
#-----------------------------------------------------------------------

APP       := xxx
TARGET    := rankServer
MFLAGS    :=
DFLAGS    :=
CONFIG    := 
STRIP_FLAG:= N
J2CPP_FLAG:= 

INCLUDE   += -I/usr/local/mqq/wbl/include 
LIB       += -L/usr/local/mqq/wbl/lib -lwbl

#-----------------------------------------------------------------------

include /usr/local/taf/makefile.taf

#-----------------------------------------------------------------------


export LC_ALL   =   zh_CN.UTF-8
export LC_LANG  =   zh_CN.UTF-8
