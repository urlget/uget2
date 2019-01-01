/*
    Copyright (C) 2011-2016 Ben Kibbey <bjk@luxsci.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02110-1301  USA
*/
#ifndef PWMD_H
#define PWMD_H

#include <libpwmd.h>
#include "UgetData.h"

struct pwmd_proxy_s {
       gchar* hostname;
       gchar* username;
       gchar* password;
       gchar* type;
       gchar* path;
       gint port;
};

gpg_error_t ug_set_pwmd_proxy_options(struct pwmd_proxy_s*, UgetProxy*);
void ug_close_pwmd(struct pwmd_proxy_s*);

#endif
