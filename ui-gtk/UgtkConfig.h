/*
 *
 *   Copyright (C) 2012-2017 by C.H. Huang
 *   plushuang.tw@gmail.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  ---
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU Lesser General Public License in all respects
 *  for all of the code used other than OpenSSL.  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so.  If you
 *  do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 *
 */

#ifndef UGTK_CONFIG_H
#define UGTK_CONFIG_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#elif defined _WIN32 || defined _WIN64
//#  define HAVE_GLIB        1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GETTEXT_PACKAGE
#define GETTEXT_PACKAGE		"uget"
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION		"2.1.4"
#endif

// ----------------------------------------------------------------------------
// path

const char* ugtk_get_data_dir (void);
const char* ugtk_get_config_dir (void);
const char* ugtk_get_locale_dir (void);

#if defined _WIN32 || defined _WIN64

const char* ugtk_get_install_dir (void);
gboolean    ugtk_is_portable (void);

// Please undef UG_DATADIR and LOCALEDIR if you doesn't want to use fixed path
// to load data and you use autoconf and automake to build uGet in MSYS2.
#undef UG_DATADIR
#undef LOCALEDIR

#ifndef UG_DATADIR
#define UG_DATADIR    ugtk_get_data_dir()
#endif

#ifndef LOCALEDIR
#define LOCALEDIR     ugtk_get_locale_dir()
#endif

#else

#ifndef UG_DATADIR
#define UG_DATADIR    "/usr/share"
#endif

#ifndef LOCALEDIR
#define LOCALEDIR     UG_DATADIR "/locale"
#endif

#endif // _WIN32 || _WIN64


#ifdef __cplusplus
}
#endif

#endif // UGTK_CONFIG_H
