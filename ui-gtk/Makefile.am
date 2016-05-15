bin_PROGRAMS = uget-gtk

uget_gtk_CPPFLAGS = \
	-DUG_DATADIR='"$(datadir)"'  \
	-I$(top_srcdir)/ui-gtk    \
	-I$(top_srcdir)/uget      \
	-I$(top_srcdir)/uglib

uget_gtk_CFLAGS = \
	@PTHREAD_CFLAGS@        \
	@LFS_CFLAGS@            \
	@CURL_CFLAGS@           \
	@LIBGCRYPT_CFLAGS@      \
	@LIBCRYPTO_CFLAGS@      \
	@GTK_CFLAGS@            \
	@LIBNOTIFY_CFLAGS@      \
	@APP_INDICATOR_CFLAGS@  \
	@GSTREAMER_CFLAGS@      \
	@LIBPWMD_CFLAGS@

uget_gtk_LDFLAGS  = @LFS_LDFLAGS@

uget_gtk_LDADD = \
	$(top_builddir)/uget/libuget.a    \
	$(top_builddir)/uglib/libuglib.a  \
	@PTHREAD_LIBS@        \
	@CURL_LIBS@           \
	@LIBGCRYPT_LIBS@      \
	@LIBCRYPTO_LIBS@      \
	@GTK_LIBS@            \
	@LIBNOTIFY_LIBS@      \
	@APP_INDICATOR_LIBS@  \
	@GSTREAMER_LIBS@      \
	@LIBPWMD_LIBS@

uget_gtk_SOURCES = \
	UgtkUtil.c  UgtkConfig.c  UgtkSetting.c  \
	UgtkNodeList.c  UgtkNodeTree.c  UgtkNodeView.c  \
	UgtkTraveler.c  UgtkSummary.c  \
	UgtkTrayIcon.c  UgtkBanner.c  \
	UgtkSequence.c  UgtkSelector.c  \
	UgtkProxyForm.c  UgtkDownloadForm.c  UgtkCategoryForm.c  \
	UgtkNodeDialog.c  UgtkBatchDialog.c  \
	UgtkScheduleForm.c  UgtkSettingForm.c  UgtkSettingDialog.c  \
	UgtkConfirmDialog.c  UgtkAboutDialog.c  \
	UgtkMenubar.c  UgtkMenubar-ui.c  \
	UgtkApp.c  UgtkApp-ui.c  UgtkApp-callback.c  \
	UgtkApp-timeout.c  UgtkApp-main.c

noinst_HEADERS = \
	UgtkUtil.h  UgtkConfig.h  UgtkSetting.h  \
	UgtkNodeList.h  UgtkNodeTree.h  UgtkNodeView.h  \
	UgtkTraveler.h  UgtkSummary.h  \
	UgtkTrayIcon.h  UgtkBanner.h  \
	UgtkSequence.h  UgtkSelector.h  \
	UgtkProxyForm.h  UgtkDownloadForm.h  UgtkCategoryForm.h  \
	UgtkNodeDialog.h  UgtkBatchDialog.h  \
	UgtkScheduleForm.h  UgtkSettingForm.h  UgtkSettingDialog.h  \
	UgtkConfirmDialog.h  UgtkAboutDialog.h  \
	UgtkMenubar.h  \
	UgtkApp.h

