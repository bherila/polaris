# Makefile form Microsoft C++ nmake.exe
# $Id: Makefile,v 1.5 2006/05/17 15:53:11 edink Exp $

CC=cl.exe
LD=link.exe

BUILD=Release

LIBGDCFLAGS= \
   /D PATHSEPARATOR="\";\"" \
   /D DEFAULT_FONTPATH="\"C:\\WINDOWS\\FONTS;C:\\WINNT\\FONTS\"" \
   /D HAVE_FT2BUILD_H \
   /D HAVE_LIBZ \
   /D HAVE_GD_BUNDLED=1  \
   /D HAVE_GD_GIF_READ=1  \
   /D HAVE_GD_GIF_CREATE=1  \
   /D HAVE_GD_IMAGESETBRUSH=1  \
   /D HAVE_GD_IMAGESETTILE=1 \
   /D HAVE_GD_JPG  \
   /D HAVE_GD_PNG  \
   /D HAVE_GD_STRINGFTEX=1  \
   /D HAVE_GD_STRINGTTF=1  \
   /D HAVE_GD_XBM  \
   /D HAVE_LIBFREETYPE=1  \
   /D HAVE_LIBJPEG  \
   /D HAVE_LIBPNG  \
   /D USE_GD_IMGSTRTTF

CFLAGS=-I. -I.. -nologo -DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32 $(LIBGDCFLAGS)

LIBS=libjpeg.lib freetype2.lib libpng.lib zlib.lib kernel32.lib user32.lib advapi32.lib

!if "$(BUILD)" == "Debug"
CFLAGS=$(CFLAGS) /Od /Zi /MDd /D_DEBUG /LDd
LDFLAGS=/DEBUG $(LDFLAGS) /nodefaultlib:msvcrt.lib
LIBS=$(LIBS) msvcrtd.lib
!else
CFLAGS=$(CFLAGS) /Og /Oi /O2 /Oy /GF /MD /DNDEBUG /LD
LDFLAGS=/nodefaultlib:msvcrtd.lib
LIBS=$(LIBS) msvcrt.lib
!endif
CPPFLAGS=$(CFLAGS)

!if "$(DLLNAME)" == ""
DLLNAME=bgd.dll
!endif

DYNAMICLIB=$(DLLNAME:.dll=.lib)

!if "$(STATICLIB)" == ""
STATICLIB=bgd_a.lib
!endif

OBJS= \
  gd.obj \
	gdcache.obj \
  gdfontg.obj \
  gdfontl.obj \
  gdfontmb.obj \
  gdfonts.obj \
  gdfontt.obj \
  gdft.obj \
  gdfx.obj \
  gd_gd2.obj \
  gd_gd.obj \
  gd_gif_in.obj \
  gd_gif_out.obj \
  gdhelpers.obj \
  gd_io.obj \
  gd_io_dp.obj \
  gd_io_file.obj \
  gd_io_ss.obj \
  gd_jpeg.obj \
  gd_security.obj \
  gdkanji.obj \
  gd_png.obj \
  gd_ss.obj \
  gdtables.obj \
  gd_topal.obj \
  gd_wbmp.obj \
  gdxpm.obj \
  wbmp.obj

EXE_OBJS= \
  annotate.obj \
  circletexttest.obj \
  fontconfigtest.obj \
  fontsizetest.obj \
  fontwheeltest.obj \
	gd2copypal.obj \
	gd2togif.obj \
	gd2topng.obj \
	gdcmpgif.obj \
	gdparttopng.obj \
	gdtopng.obj \
	giftogd2.obj \
  gdtest.obj \
  gdtestft.obj \
  gifanimtest.obj \
  pngtogd.obj \
  pngtogd2.obj \
  testac.obj \
  testtr.obj \
  webpng.obj
  
  
all: $(DLLNAME) $(STATICLIB) tests

$(EXE_OBJS:.obj=.exe): $*.obj $(DLLNAME)
	link /out:$*.exe $*.obj $(DYNAMICLIB)

tests: $(EXE_OBJS:.obj=.exe)

libgd.dll.res: libgd.rc
	$(RC) /fo libgd.dll.res libgd.rc

{..}.c.obj:
	$(CC) $(CFLAGS) -c $<

$(STATICLIB): $(OBJS)
	$(LD) /lib /out:$(STATICLIB) $(OBJS)
	
$(DLLNAME): $(OBJS) libgd.dll.res
	$(LD) /dll /out:$(DLLNAME) \
		$(LDFLAGS) \
		$(OBJS) \
		libgd.dll.res \
		$(LIBS)
		
dist: all
	-rmdir /s /q distro
	mkdir distro
	mkdir distro\bin
	copy *.exe distro\bin
	copy *.dll distro\bin
	mkdir distro\include
	copy ..\*.h distro\include
	mkdir distro\lib
	copy *.lib distro\lib
	copy ..\COPYING distro\COPYING.TXT

clean:
	-del /q *.obj *.dll *.lib *.ilk *.pdb *.exp *.exe *.png
	-rmdir /s /q distro
