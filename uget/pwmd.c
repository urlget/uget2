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

static gpg_error_t alloc_result(const char* s, size_t len, char** result)
{
        char *b;

        if (!len) {
                *result = NULL;
                return 0;
        }

        b = pwmd_malloc(len+1);
        if (!b)
          return GPG_ERR_ENOMEM;

        memcpy(b, s, len);
        b[len] = 0;
        *result = b;
        return 0;
}

static int failure(const char* root, const char* id, gpg_error_t rc,
                   int required)
{
        if (!rc)
                return 0;

        if (gpg_err_code (rc) == GPG_ERR_ELEMENT_NOT_FOUND
            || gpg_err_code (rc) == GPG_ERR_NO_DATA) {
                if (!required)
                        return 0;

                fprintf(stderr, "pwmd: ERR(%u): %s%s%s: %s\n", rc,
                        root ? root : "", root ? "^": "", id,
                        gpg_strerror (rc));
        }

        return 1;
}

gpg_error_t ug_set_pwmd_proxy_options(struct pwmd_proxy_s* pwmd,
                                      UgetProxy* proxy)
{
	char *bulk = NULL;
        const gchar *bresult = NULL;
        gchar *result = NULL;
        size_t brlen, len;
        size_t offset = 0;
        char *str;
        gpg_error_t rcs[4] = { 0 };
        gpg_error_t rc, brc;
        pwm_t *pwm = NULL;
        gchar *path = NULL;
        const gchar *root = proxy->pwmd.element;
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

        rc = pwmd_setopt(pwm, PWMD_OPTION_SOCKET_TIMEOUT, 120);

        if (!rc && proxy->pwmd.socket_args && *proxy->pwmd.socket_args)
          args = g_strsplit(proxy->pwmd.socket_args, ",", 0);

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
                rc = pwmd_setopt(pwm, PWMD_OPTION_LOCK_TIMEOUT, 100);
        if (rc)
                goto fail;

        rc = pwmd_open(pwm, proxy->pwmd.file, NULL, NULL);
        if (rc)
                goto fail;

        rc = pwmd_bulk_append(&bulk, "NOP", 3, "NOP", NULL, 0, &offset);
        if (rc)
                goto fail;

        rcs[0] = 0;
        rcs[1] = GPG_ERR_MISSING_ERRNO;
        str = pwmd_strdup_printf("%stype", path);
        rc = pwmd_bulk_append_rc(&bulk, rcs, "TYPE", 4, "GET", str,
                                 strlen (str), &offset);
        pwmd_free(str);
        if (rc)
                goto fail;

        rcs[0] = 0;
        rcs[1] = GPG_ERR_MISSING_ERRNO;
        str = pwmd_strdup_printf("%shostname", path);
        rc = pwmd_bulk_append_rc(&bulk, rcs, "HOST", 4, "GET", str,
                                 strlen (str), &offset);
        pwmd_free(str);
        if (rc)
                goto fail;

        rcs[0] = 0;
        rcs[1] = gpg_err_make (GPG_ERR_SOURCE_USER_1, GPG_ERR_ELEMENT_NOT_FOUND);
        rcs[2] = GPG_ERR_MISSING_ERRNO;
        str = pwmd_strdup_printf("%sport", path);
        rc = pwmd_bulk_append_rc(&bulk, rcs, "PORT", 4, "GET", str,
                                 strlen (str), &offset);
        pwmd_free(str);
        if (rc)
                goto fail;

        rcs[0] = 0;
        rcs[1] = gpg_err_make (GPG_ERR_SOURCE_USER_1, GPG_ERR_ELEMENT_NOT_FOUND);
        rcs[2] = gpg_err_make (GPG_ERR_SOURCE_USER_1, GPG_ERR_NO_DATA);
        rcs[3] = GPG_ERR_MISSING_ERRNO;
        str = pwmd_strdup_printf("%susername", path);
        rc = pwmd_bulk_append_rc(&bulk, rcs, "USER", 4, "GET", str,
                                 strlen (str), &offset);
        pwmd_free(str);
        if (rc)
                goto fail;

        rcs[0] = 0;
        rcs[1] = gpg_err_make (GPG_ERR_SOURCE_USER_1, GPG_ERR_ELEMENT_NOT_FOUND);
        rcs[2] = gpg_err_make (GPG_ERR_SOURCE_USER_1, GPG_ERR_NO_DATA);
        rcs[3] = GPG_ERR_MISSING_ERRNO;
        str = pwmd_strdup_printf("%spassword", path);
        rc = pwmd_bulk_append_rc(&bulk, rcs, "PASS", 4, "GET", str,
                                 strlen (str), &offset);
        pwmd_free(str);
        if (rc)
                goto fail;

        rc = pwmd_bulk_finalize(&bulk);
        if (!rc)
                rc = pwmd_bulk(pwm, &result, &len, NULL, NULL, bulk, strlen (bulk));
        if (rc)
                goto fail;

        offset = 0;
        rc = pwmd_bulk_result(result, len, "TYPE", 4, &offset, &bresult,
                              &brlen, &brc);
        if (failure(root, "type", rc ? rc : brc, 1))
                goto fail;
        else if(brlen)
                alloc_result (bresult, brlen, &pwmd->type);

        rc = pwmd_bulk_result(result, len, "HOST", 4, &offset, &bresult,
                              &brlen, &brc);
        if (failure(root, "hostname", rc ? rc : brc, 1))
                goto fail;
        else if (brlen)
                alloc_result(bresult, brlen, &pwmd->hostname);

        rc = pwmd_bulk_result(result, len, "PORT", 4, &offset, &bresult,
                              &brlen, &brc);
        if (failure(root, "port", rc ? rc : brc, 0))
                goto fail;
        else if (brlen) {
                char *p;

                alloc_result(bresult, brlen, &p);
                pwmd->port = atoi(p);
                pwmd_free(p);
          }

        rc = pwmd_bulk_result(result, len, "USER", 4, &offset, &bresult,
                              &brlen, &brc);
        if (failure(root, "username", rc ? rc : brc, 0))
                goto fail;
        else if (brlen)
                alloc_result(bresult, brlen, &pwmd->username);

        rc = pwmd_bulk_result(result, len, "PASS", 4, &offset, &bresult,
                              &brlen, &brc);
        if (failure(root, "password", rc ? rc : brc, 0))
                goto fail;
        else if (brlen)
                alloc_result(bresult, brlen, &pwmd->password);

        brc = 0;

fail:
        if (args)
                g_strfreev(args);

        pwmd_free(result);
        pwmd_free(bulk);
        pwmd_close(pwm);
        return rc ? rc : brc;
}

void ug_close_pwmd(struct pwmd_proxy_s* pwmd)
{
  pwmd_free(pwmd->type);
  pwmd_free(pwmd->hostname);
  pwmd_free(pwmd->username);
  pwmd_free(pwmd->password);
  g_free(pwmd->path);
}
