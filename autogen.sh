if [ ! -f AUTHORS ]; then
	touch AUTHORS
fi

if [ ! -f NEWS ]; then
	touch NEWS
fi

if [ ! -f README ]; then
	touch README
fi

if [ ! -f ChangeLog ]; then
	touch ChangeLog
fi

# autoscan

## Generate `aclocal.m4' by scanning `configure.ac' or `configure.in'
aclocal

## --- AC_CONFIG_HEADERS
autoheader

## --- AC_PROG_INTLTOOL
intltoolize

## --- AC_PROG_LIBTOOL
# libtoolize --copy

## --- AM_INIT_AUTOMAKE
automake --add-missing

autoconf


