SUBDIRS = runprgs

bin_PROGRAMS = tnt tntc
tnt_SOURCES = ax25dump.c \
	      ax25k.c \
	      ax25k.h \
	      bcast.c \
	      bcast.h \
	      bcastadd.c \
	      bcastadd.h \
	      boxlist.c \
	      boxlist.h \
	      codconv.c \
	      comp.c \
	      comp.h \
	      connect.c \
	      connect.h \
	      cookie.c \
	      crc.c \
	      display.c \
	      file.c \
	      huffman.c \
	      iface.c \
	      iface.h \
	      ifacedef.h \
	      init.c \
	      init.h \
	      keyboard.c \
	      keys.h \
	      log.c \
	      macro.c \
	      macro.h \
	      main.c \
	      monbox.c \
	      monbox.h \
	      priv.c \
	      priv.h \
	      remote.c \
	      serial.c \
	      shell.c \
	      shell.h \
	      tnt.h \
	      window.c \
	      window.h \
	      xmon.c \
	      xmon.h \
	      yapp.c

tntc_sources = tntc.c tnt.h

LIBS = @INTLLIBS@ @LIBS@
tnt_LDADD = @INTLLIBS@
tntc_LDADD = @INTLLIBS@
localedir = $(datadir)/locale
INCLUDES = @CPPFLAGS@ -I../intl -DLOCALEDIR=\"$(localedir)\" -funsigned-char

