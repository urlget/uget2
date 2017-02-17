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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <string.h>
#include <stdlib.h>
#include "pwmd.h"

gpg_error_t ug_set_pwmd_proxy_options(struct pwmd_proxy_s *pwmd,
       UgetProxy *proxy)
{
       gpg_error_t rc;
       pwm_t *pwm = NULL;
       gchar *result;
       gchar *path = NULL;
       gint i;
       gchar **args = NULL;

       pwmd->port = 80;

       if (proxy->pwmd.element) {
               pwmd->path = path = g_strdup_printf("%s\t", proxy->pwmd.element);

               for (i = 0; path[i]; i++) {
                       if (path[i] == '^')
                               path[i] = '\t';
               }
       }

       pwmd_init();
       rc = pwmd_new("uget", &pwm);
       if (rc)
	   goto fail;

       rc = pwmd_setopt (pwm, PWMD_OPTION_SOCKET_TIMEOUT, 120);

       if (!rc && proxy->pwmd.socket_args && *proxy->pwmd.socket_args)
	 args = g_strsplit (proxy->pwmd.socket_args, ",", 0);

       if (!rc)
	 rc = pwmd_connect(pwm, proxy->pwmd.socket,
			   g_strv_length (args) > 0 ? args[0] : NULL,
			   g_strv_length (args) > 1 ? args[1] : NULL,
			   g_strv_length (args) > 2 ? args[2] : NULL,
			   g_strv_length (args) > 3 ? args[3] : NULL,
			   g_strv_length (args) > 4 ? args[4] : NULL,
			   g_strv_length (args) > 5 ? args[5] : NULL,
			   g_strv_length (args) > 6 ? args[6] : NULL,
			   g_strv_length (args) > 7 ? args[7] : NULL
			   );
       if (rc)
               goto fail;

       rc = pwmd_setopt(pwm, PWMD_OPTION_PINENTRY_DESC, NULL);
       if (!rc)
	 rc = pwmd_command(pwm, NULL, NULL, NULL, NULL,
			   "OPTION lock-timeout=100");
       if (rc)
               goto fail;

       rc = pwmd_open(pwm, proxy->pwmd.file, NULL, NULL);
       if (rc)
               goto fail;

       rc = pwmd_command(pwm, &result, NULL, NULL, NULL, "GET %stype",
	       path ? path : "");
       if (rc)
               goto fail;

       pwmd->type = result;
       rc = pwmd_command(pwm, &result, NULL, NULL, NULL, "GET %shostname",
	       path ? path : "");
       if (rc)
               goto fail;

       pwmd->hostname = result;
       rc = pwmd_command(pwm, &result, NULL, NULL, NULL, "GET %sport",
	       path ? path : "");
       if (rc && gpg_err_code (rc) != GPG_ERR_ELEMENT_NOT_FOUND)
               goto fail;

       pwmd->port = atoi(result);
       pwmd_free(result);
       rc = pwmd_command(pwm, &result, NULL, NULL, NULL, "GET %susername",
	       path ? path : "");
       if (rc && gpg_err_code (rc) != GPG_ERR_ELEMENT_NOT_FOUND
	   && gpg_err_code (rc) != GPG_ERR_NO_DATA)
               goto fail;

       if (!rc)
               pwmd->username = result;
       rc = pwmd_command(pwm, &result, NULL, NULL, NULL, "GET %spassword",
	       path ? path : "");

       if (rc && gpg_err_code (rc) != GPG_ERR_ELEMENT_NOT_FOUND
	   && gpg_err_code (rc) != GPG_ERR_NO_DATA)
               goto fail;
       if (!rc)
               pwmd->password = result;

       rc = 0;

fail:
       if (args)
	 g_strfreev (args);

       if (pwm)
	   pwmd_close(pwm);
       return rc;
}

void ug_close_pwmd(struct pwmd_proxy_s *pwmd)
{
  pwmd_free(pwmd->type);
  pwmd_free(pwmd->hostname);
  pwmd_free(pwmd->username);
  pwmd_free(pwmd->password);
  g_free(pwmd->path);
}
