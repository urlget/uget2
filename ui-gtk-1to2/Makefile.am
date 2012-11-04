bin_PROGRAMS = uget-gtk-1to2

uget_gtk_1to2_CPPFLAGS = \
	-DDATADIR='"$(datadir)"'  \
	-I$(top_srcdir)/ui-gtk    \
	-I$(top_srcdir)/uget      \
	-I$(top_srcdir)/uglib

uget_gtk_1to2_CFLAGS = \
	@GTK_CFLAGS@

uget_gtk_1to2_LDADD = \
	$(top_builddir)/uget/libuget.a    \
	$(top_builddir)/uglib/libuglib.a  \
	@GTK_LIBS@

uget_gtk_1to2_SOURCES = \
	UgCategory.c  \
	UgData1.c     \
	UgData-download.c   \
	UgDataset.c   \
	UgMarkup.c    \
	UgRegistry1.c \
	UgSetting.c   \
	Ugtk1to2.c    \
	../ui-gtk/UgtkSetting.c \
	main.c

noinst_HEADERS = \
	UgCategory.h  \
	UgData1.h     \
	UgData-download.h   \
	UgDataset.h   \
	UgMarkup.h    \
	UgRegistry1.h \
	UgSetting.h   \
	Ugtk1to2.h    \
	../ui-gtk/UgtkSetting.h	
