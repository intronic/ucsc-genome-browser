CC=gcc
COPT=-O -ggdb
CFLAGS=
HG_DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DMACHTYPE_$(MACHTYPE)
HG_WARN=-Wformat -Wimplicit -Wuninitialized -Wreturn-type
HG_INC=-I../inc -I../../inc -I../../../inc
CGI_BIN=/usr/local/apache/cgi-bin

.c.o:
	${CC} ${COPT} ${CFLAGS} ${HG_DEFS} ${HG_WARN} ${HG_INC} ${XINC} -c $*.c

