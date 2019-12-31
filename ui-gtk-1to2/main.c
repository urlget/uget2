/*
 *
 *   Copyright (C) 2013-2020 by C.H. Huang
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

#include "UgCategory.h"
#include "UgData-download.h"
#include "Ugtk1to2.h"

static void register_iface (void)
{
	// data
	ug_data1_interface_register (&ug_common_iface);
	ug_data1_interface_register (&ug_proxy_iface);
	ug_data1_interface_register (&ug_progress_iface);
	ug_data1_interface_register (&ug_http_iface);
	ug_data1_interface_register (&ug_ftp_iface);
	ug_data1_interface_register (&ug_log_iface);
	// category
	ug_data1_interface_register (&ug_category_iface);
	ug_data1_interface_register (&ug_relation_iface);
}

int  main (int argc, char** argv)
{
	Ugtk1to2*  u1t2;
	char*      path;
	int        n;

	path = g_build_filename (g_get_user_config_dir (), "uGet", NULL);
	puts ("\n"
	      "Convert uGet for GTK+ data file from 1.10.x to 2.x" "\n");
	puts ("Usage:");
	printf ("  %s [config directory]" "\n\n", argv[0]);
	printf ("e.g." "\n"
	        "  %s \"%s\"" "\n\n",
	        argv[0], path);
	g_free (path);
	if (argc == 1)
		return EXIT_SUCCESS;

	// starting convert
	register_iface ();
	u1t2 = ugtk_1to2_new (argv[1]);

	// setting
	n = ugtk_1to2_load_setting (u1t2);
	if (n == TRUE) {
		puts ("Load setting OK.");
		n = ugtk_1to2_save_setting (u1t2);
		if (n == TRUE)
			puts ("Save setting OK.");
		else
			puts ("Failed to save setting.");
	}
	// category and download
	n = ugtk_1to2_load_category (u1t2);
	printf ("Load %d category\n", n);
	if (n == 0) {
		puts ("No category exists");
		return EXIT_FAILURE;
	}
	n = ugtk_1to2_save_category (u1t2);
	printf ("Save %d category\n", n);

	ugtk_1to2_free (u1t2);

	return EXIT_SUCCESS;
}
