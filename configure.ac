AC_INIT(uget, 1.93)

## Use automake (add automake to autogen.sh)
AM_INIT_AUTOMAKE

## --- Use config.h (autogen.sh add autoheader)
AC_CONFIG_HEADERS([config.h])

## --- Determine a C compiler to use.
AC_PROG_CC

## --- Check C compiler -c -o options.
AM_PROG_CC_C_O

## --- Determine a C++ compiler to use.
# AC_PROG_CXX

## Use library (static library)
AC_PROG_RANLIB

## ----------------------------------------------
## L10N  (add intltoolize to autogen.sh)
AC_PROG_INTLTOOL
## replace ALL_LINGUAS with po/LINGUAS
# ALL_LINGUAS=""
GETTEXT_PACKAGE="$PACKAGE"
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(LOCALEDIR)
## Use AM_GLIB_DEFINE_LOCALEDIR with AC_CONFIG_HEADERS

## ----------------------------------------------
## GTK+
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.4)

## ----------------
## glib (add HAVE_GLIB definition to config.h)
PKG_CHECK_EXISTS([glib-2.0],
                 [have_glib="yes"],
                 [have_glib="no"])
if test x$have_glib = xyes ; then
	PKG_CHECK_MODULES(GLIB, glib-2.0)
	AC_SUBST(GLIB_CFLAGS)
	AC_SUBST(GLIB_LIBS)
	AC_DEFINE(HAVE_GLIB, 1, [Define to 1 if glib support is required.])
fi

## ----------------
## pthread
AC_CHECK_LIB(pthread, pthread_create,,
	[AC_MSG_ERROR([required library pthread missing])])
PTHREAD_CFLAGS="-pthread"
PTHREAD_LIBS="-pthread"
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

## ----------------
## LFS
AC_PATH_PROG(GETCONF, getconf)
if test "x$GETCONF" != "x" ; then
	LFS_CFLAGS=`$GETCONF LFS_CFLAGS`
	LFS_LDFLAGS=`$GETCONF LFS_LDFLAGS`
fi
AC_SUBST(LFS_CFLAGS)
AC_SUBST(LFS_LDFLAGS)

## ----------------
## cURL
AC_PATH_PROG(CURL_CONFIG, curl-config)
if test "x$CURL_CONFIG" = "x" ; then
	AC_MSG_ERROR(Unable to find curl-config, please install libcurl)
fi
CURL_CFLAGS=`$CURL_CONFIG --cflags`
CURL_LIBS=`$CURL_CONFIG --libs`
let CURL_VERNUM=0x0`$CURL_CONFIG --vernum`
let CURL_VERMIN=0x071301		# 7.19.1
if test $CURL_VERNUM -lt $CURL_VERMIN; then
	AC_MSG_ERROR(Requires libcurl version >= 7.19.1)
fi
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

## ----------------
## GnuTLS
AC_ARG_WITH(
	[gnutls],
	AC_HELP_STRING([--with-gnutls[=@<:@no/auto/yes@:>@]], [Enable GnuTLS support.]),
	[with_gnutls="$withval"],
	[with_gnutls="auto"]
)
if test "x$with_gnutls" != "xno"; then
#	AC_CHECK_HEADER(gcrypt.h, [USE_GNUTLS_GCRYPT=1], [USE_GNUTLS_GCRYPT=0])
#	if test "$USE_GNUTLS_GCRYPT" = "1"; then
#		LIBGCRYPT_CFLAGS=""
#		AC_SUBST(LIBGCRYPT_CFLAGS)
#	fi
#	AC_CHECK_HEADER(gcrypt/gcrypt.h, [USE_GNUTLS_GCRYPT=1], [USE_GNUTLS_GCRYPT=0])
#	if test "$USE_GNUTLS_GCRYPT" = "1"; then
#		LIBGCRYPT_CFLAGS=""
#		AC_SUBST(LIBGCRYPT_CFLAGS, [""])
#	fi
#	AC_CHECK_LIB(gcrypt, gcry_control, [USE_GNUTLS_GCRYPT=1], [USE_GNUTLS_GCRYPT=0])
#	if test "$USE_GNUTLS_GCRYPT" = "1"; then
#		LIBGCRYPT_LIBS="-lgcrypt"
#		AC_SUBST(LIBGCRYPT_LIBS, ["-lgcrypt"])
#	fi
	AC_PATH_PROG(LIBGCRYPT_CONFIG, libgcrypt-config)
	if test "x$LIBGCRYPT_CONFIG" = "x" ; then
		if test "x$with_gnutls" = "xyes"; then
			AC_MSG_ERROR(Unable to find libgcrypt-config, please install libgcrypt)
		fi
	else
		LIBGCRYPT_CFLAGS=`$LIBGCRYPT_CONFIG --cflags`
		LIBGCRYPT_LIBS=`$LIBGCRYPT_CONFIG --libs`
		AC_SUBST(LIBGCRYPT_CFLAGS)
		AC_SUBST(LIBGCRYPT_LIBS)
		AC_DEFINE(USE_GNUTLS, 1, [Define to 1 if gnutls support is required.])
	fi
fi

## ----------------
## OpenSSL
AC_ARG_WITH(
	[openssl],
	AC_HELP_STRING([--with-openssl], [Enable OpenSSL support.]),
	[enable_openssl="yes"],
	[enable_openssl="no"]
)
if test "x$enable_openssl" = "xyes"; then
	PKG_CHECK_MODULES(LIBCRYPTO, libcrypto)
	AC_SUBST(LIBCRYPTO_CFLAGS)
	AC_SUBST(LIBCRYPTO_LIBS)
	AC_DEFINE(USE_OPENSSL, 1, [Define to 1 if openssl support is required.])
fi

## ----------------
## libnotify
AC_ARG_ENABLE(
	[notify],
	AC_HELP_STRING([--disable-notify], [Disable libnotify support.]),
	[enable_notify="$enableval"],
	[enable_notify="yes"]
)
if test "x$enable_notify" = "xyes"; then
  PKG_CHECK_MODULES(LIBNOTIFY, libnotify)
  AC_DEFINE(HAVE_LIBNOTIFY, 1, [Define to 1 if libnotify support is required.])
  # for ArchLinux
  AC_SUBST(LIBNOTIFY_CFLAGS)
  AC_SUBST(LIBNOTIFY_LIBS)
fi

## ----------------
## appindicator
AC_ARG_ENABLE(
	[appindicator],
	AS_HELP_STRING([--enable-appindicator[=@<:@no/auto/yes@:>@]],[Build support for application indicators.]),
	[enable_appindicator=$enableval],
	[enable_appindicator="auto"]
)
if test x$enable_appindicator = xauto ; then
	PKG_CHECK_EXISTS([appindicator3-0.1],
	                 [enable_appindicator="yes"],
	                 [enable_appindicator="no"])
fi
if test x$enable_appindicator = xyes ; then
	PKG_CHECK_MODULES(APP_INDICATOR, appindicator3-0.1)
	AC_SUBST(APP_INDICATOR_CFLAGS)
	AC_SUBST(APP_INDICATOR_LIBS)
	AC_DEFINE(HAVE_APP_INDICATOR, 1, [Have AppIndicator])
fi

## ----------------
## gstreamer
AC_ARG_ENABLE(
	[gstreamer],
	AC_HELP_STRING([--disable-gstreamer], [Disable GStreamer audio support.]),
	[enable_gstreamer="$enableval"],
	[enable_gstreamer="yes"]
)
if test "x$enable_gstreamer" = "xyes"; then
  PKG_CHECK_EXISTS([gstreamer-1.0],
                   [enable_gstreamer1="yes"],
                   [enable_gstreamer1="no"])
fi
if test "x$enable_gstreamer1" = "xyes"; then
  PKG_CHECK_MODULES(GSTREAMER, gstreamer-1.0)
  AC_DEFINE(HAVE_GSTREAMER, 1, [Define to 1 if gstreamer support is required.])
  # for ArchLinux
  AC_SUBST(GSTREAMER_CFLAGS)
  AC_SUBST(GSTREAMER_LIBS)
elif test "x$enable_gstreamer" = "xyes"; then
  PKG_CHECK_MODULES(GSTREAMER, gstreamer-0.10)
  AC_DEFINE(HAVE_GSTREAMER, 1, [Define to 1 if gstreamer support is required.])
  # for ArchLinux
  AC_SUBST(GSTREAMER_CFLAGS)
  AC_SUBST(GSTREAMER_LIBS)
fi

## -----------------
## libpwmd
AC_ARG_ENABLE(
	[pwmd],
	AC_HELP_STRING([--enable-pwmd], [Enable pwmd support.]),
	[enable_pwmd="$enableval"],
	[enable_pwmd="no"]
)
if test "x$enable_pwmd" = "xyes"; then
  PKG_CHECK_MODULES(LIBPWMD, [libpwmd-7.0 >= 7.0.0])
  AC_DEFINE(HAVE_LIBPWMD, 1, [Define to 1 if libpwmd support is required.])
fi

## ----------------------------------------------
## output
AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  tests/Makefile
  uget/Makefile
  uglib/Makefile
  ui-gtk/Makefile
  ui-gtk-1to2/Makefile
  pixmaps/Makefile
  sounds/Makefile
  po/Makefile.in
  Windows/Makefile
])

AC_OUTPUT

