CC=gcc
COPT=-O
CFLAGS=
HG_DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DMACHTYPE_${MACHTYPE}
HG_WARN=-Wformat -Wimplicit -Wuninitialized -Wreturn-type
HG_INC=-I../inc -I../../inc -I../../../inc -I../../../../inc

# add the follow to makefiles to enable stronger warning checks
# HG_WARN = ${HG_WARN_ERR}
ifeq (${OSTYPE},darwin)
    HG_WARN_ERR = -DJK_WARN -Wall -Werror -Wno-unused-variable
else
    HG_WARN_ERR = -DJK_WARN -Wall -Werror
endif

SCRIPTS=/cluster/bin/scripts
CGI_BIN=/usr/local/apache/cgi-bin
BINDIR = ${HOME}/bin/${MACHTYPE}
MKDIR=mkdir -p
STRIP=strip
CVS=cvs

# portable naming of compiled executables: add ".exe" if compiled on 
# Windows (with cygwin).
ifeq (${OS}, Windows_NT)
  AOUT=a.exe
  EXE=.exe
else
  AOUT=a.out
  EXE=
endif

.c.o:
	${CC} ${COPT} ${CFLAGS} ${HG_DEFS} ${HG_WARN} ${HG_INC} ${XINC} -c $*.c


