/*
 *
 *   Copyright (C) 2005-2015 by C.H. Huang
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

#include <string.h>
#include <stdlib.h>

#include "UgData-download.h"
#include "UgSetting.h"

static void	ug_string_list_in_markup (GList** string_list, GMarkupParseContext* context);
static void	ug_string_list_to_markup (GList** string_list, UgMarkup* markup);
static void	ug_schedule_state_in_markup (guint (*state)[7][24], GMarkupParseContext* context);
static void	ug_schedule_state_to_markup (guint (*state)[7][24], UgMarkup* markup);


// ----------------------------------------------------------------------------
// UgDownloadColumnSetting

static const UgDataEntry	ug_download_column_setting_entry[] =
{
	{"completed",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, completed),	UG_TYPE_INT,	NULL,	NULL},
	{"total",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, total),		UG_TYPE_INT,	NULL,	NULL},
	{"percent",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, percent),		UG_TYPE_INT,	NULL,	NULL},
	{"elapsed",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, elapsed),		UG_TYPE_INT,	NULL,	NULL},
	{"left",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, left),			UG_TYPE_INT,	NULL,	NULL},
	{"speed",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, speed),		UG_TYPE_INT,	NULL,	NULL},
	{"UploadSpeed",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, upload_speed),	UG_TYPE_INT,	NULL,	NULL},
	{"uploaded",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, uploaded),		UG_TYPE_INT,	NULL,	NULL},
	{"ratio",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, ratio),		UG_TYPE_INT,	NULL,	NULL},
	{"retry",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, retry),		UG_TYPE_INT,	NULL,	NULL},
	{"category",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, category),		UG_TYPE_INT,	NULL,	NULL},
	{"URL",			G_STRUCT_OFFSET (struct UgDownloadColumnSetting, url),			UG_TYPE_INT,	NULL,	NULL},
	{"AddedOn",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, added_on),		UG_TYPE_INT,	NULL,	NULL},
	{"CompletedOn",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, completed_on),	UG_TYPE_INT,	NULL,	NULL},

	{"sort-nth",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, sort.nth),		UG_TYPE_INT,	NULL,	NULL},
	{"sort-order",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, sort.order),	UG_TYPE_INT,	NULL,	NULL},
	{NULL}			// null-terminated
};

static const UgData1Interface	ug_download_column_setting_iface =
{
	sizeof (struct UgDownloadColumnSetting),
	"DownloadColumnSetting",
	ug_download_column_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// UgSummarySetting
static const UgDataEntry	ug_summary_setting_entry[] =
{
	{"name",		G_STRUCT_OFFSET (struct UgSummarySetting, name),			UG_TYPE_INT,	NULL,	NULL},
	{"folder",		G_STRUCT_OFFSET (struct UgSummarySetting, folder),			UG_TYPE_INT,	NULL,	NULL},
	{"category",	G_STRUCT_OFFSET (struct UgSummarySetting, category),		UG_TYPE_INT,	NULL,	NULL},
	{"URL",			G_STRUCT_OFFSET (struct UgSummarySetting, url),				UG_TYPE_INT,	NULL,	NULL},
	{"message",		G_STRUCT_OFFSET (struct UgSummarySetting, message),			UG_TYPE_INT,	NULL,	NULL},
	{NULL}			// null-terminated
};

static const UgData1Interface	ug_summary_setting_iface =
{
	sizeof (struct UgSummarySetting),
	"SummarySetting",
	ug_summary_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// UgWindowSetting
static const UgDataEntry	ug_window_setting_entry[] =
{
	{"Toolbar",		G_STRUCT_OFFSET (struct UgWindowSetting, toolbar),		UG_TYPE_INT,	NULL,	NULL},
	{"Statusbar",	G_STRUCT_OFFSET (struct UgWindowSetting, statusbar),	UG_TYPE_INT,	NULL,	NULL},
	{"Category",	G_STRUCT_OFFSET (struct UgWindowSetting, category),		UG_TYPE_INT,	NULL,	NULL},
	{"Summary",		G_STRUCT_OFFSET (struct UgWindowSetting, summary),		UG_TYPE_INT,	NULL,	NULL},
	{"Banner",		G_STRUCT_OFFSET (struct UgWindowSetting, banner),		UG_TYPE_INT,	NULL,	NULL},
	{"x",			G_STRUCT_OFFSET (struct UgWindowSetting, x)	,			UG_TYPE_INT,	NULL,	NULL},
	{"y",			G_STRUCT_OFFSET (struct UgWindowSetting, y),			UG_TYPE_INT,	NULL,	NULL},
	{"width",		G_STRUCT_OFFSET (struct UgWindowSetting, width),		UG_TYPE_INT,	NULL,	NULL},
	{"height",		G_STRUCT_OFFSET (struct UgWindowSetting, height),		UG_TYPE_INT,	NULL,	NULL},
	{"maximized",	G_STRUCT_OFFSET (struct UgWindowSetting, maximized),	UG_TYPE_INT,	NULL,	NULL},
	{NULL},			// null-terminated
};

static const UgData1Interface	ug_window_setting_iface =
{
	sizeof (struct UgWindowSetting),
	"WindowSetting",
	ug_window_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// UgUserInterfaceSetting
static const UgDataEntry	ug_user_interface_setting_entry[] =
{
	{"CloseConfirmation",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting, close_confirmation),	UG_TYPE_INT,	NULL,	NULL},
	{"CloseAction",			G_STRUCT_OFFSET (struct UgUserInterfaceSetting, close_action),			UG_TYPE_INT,	NULL,	NULL},
	{"DeleteConfirmation",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	delete_confirmation),	UG_TYPE_INT,	NULL,	NULL},
	{"ShowTrayIcon",		G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	show_trayicon),			UG_TYPE_INT,	NULL,	NULL},
	{"StartInTray",			G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	start_in_tray),			UG_TYPE_INT,	NULL,	NULL},
	{"StartInOfflineMode",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	start_in_offline_mode),	UG_TYPE_INT,	NULL,	NULL},
	{"StartNotification",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	start_notification),	UG_TYPE_INT,	NULL,	NULL},
	{"SoundNotification",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	sound_notification),	UG_TYPE_INT,	NULL,	NULL},
	{"ApplyRecently",		G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	apply_recently),		UG_TYPE_INT,	NULL,	NULL},
#ifdef HAVE_APP_INDICATOR
	{"AppIndicator",		G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	app_indicator),			UG_TYPE_INT,	NULL,	NULL},
#endif
	{NULL},			// null-terminated
};

static const UgData1Interface	ug_user_interface_setting_iface =
{
	sizeof (struct UgUserInterfaceSetting),
	"UserInterfaceSetting",
	ug_user_interface_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// UgClipboardSetting
static const UgDataEntry	ug_clipboard_setting_entry[] =
{
	{"pattern",		G_STRUCT_OFFSET (struct UgClipboardSetting, pattern),	UG_TYPE_STRING,		NULL,	NULL},
	{"monitor",		G_STRUCT_OFFSET (struct UgClipboardSetting, monitor),	UG_TYPE_INT,		NULL,	NULL},
	{"quiet",		G_STRUCT_OFFSET (struct UgClipboardSetting, quiet),		UG_TYPE_INT,		NULL,	NULL},
	{"NthCategory",	G_STRUCT_OFFSET (struct UgClipboardSetting, nth_category),	UG_TYPE_INT,	NULL,	NULL},
	{NULL},			// null-terminated
};

static const UgData1Interface	ug_clipboard_setting_iface =
{
	sizeof (struct UgClipboardSetting),
	"ClipboardSetting",
	ug_clipboard_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// UgSpeedLimitSetting
static const UgDataEntry	ug_speed_limit_setting_entry[] =
{
	{"NormalUpload",	G_STRUCT_OFFSET (struct UgSpeedLimitSetting, normal.upload),	UG_TYPE_UINT,	NULL,	NULL},
	{"NormalDownload",	G_STRUCT_OFFSET (struct UgSpeedLimitSetting, normal.download),	UG_TYPE_UINT,	NULL,	NULL},
	{"SchedulerUpload",		G_STRUCT_OFFSET (struct UgSpeedLimitSetting, scheduler.upload),		UG_TYPE_UINT,	NULL,	NULL},
	{"SchedulerDownload",	G_STRUCT_OFFSET (struct UgSpeedLimitSetting, scheduler.download),	UG_TYPE_UINT,	NULL,	NULL},
	{NULL},			// null-terminated
};

static const UgData1Interface	ug_speed_limit_setting_iface =
{
	sizeof (struct UgSpeedLimitSetting),
	"SpeedLimitSetting",
	ug_speed_limit_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// SchedulerSetting
static const UgDataEntry	ug_scheduler_setting_entry[] =
{
	{"enable",		G_STRUCT_OFFSET (struct UgSchedulerSetting, enable),	UG_TYPE_INT,		NULL,	NULL},
	{"state",		G_STRUCT_OFFSET (struct UgSchedulerSetting, state),		UG_TYPE_CUSTOM,		ug_schedule_state_in_markup,	ug_schedule_state_to_markup},
	{NULL},			// null-terminated
};

static const UgData1Interface	ug_scheduler_setting_iface =
{
	sizeof (struct UgSchedulerSetting),
	"SchedulerSetting",
	ug_scheduler_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// PluginSetting
static const UgDataEntry	ug_plugin_setting_entry[] =
{
	{"aria2-enable",	G_STRUCT_OFFSET (struct UgPluginSetting, aria2.enable),		UG_TYPE_INT,	NULL,	NULL},
	{"aria2-launch",	G_STRUCT_OFFSET (struct UgPluginSetting, aria2.launch),		UG_TYPE_INT,	NULL,	NULL},
	{"aria2-shutdown",	G_STRUCT_OFFSET (struct UgPluginSetting, aria2.shutdown),	UG_TYPE_INT,	NULL,	NULL},
	{"aria2-path",		G_STRUCT_OFFSET (struct UgPluginSetting, aria2.path),		UG_TYPE_STRING,	NULL,	NULL},
	{"aria2-args",		G_STRUCT_OFFSET (struct UgPluginSetting, aria2.args),		UG_TYPE_STRING,	NULL,	NULL},
	{"aria2-uri",		G_STRUCT_OFFSET (struct UgPluginSetting, aria2.uri),		UG_TYPE_STRING,	NULL,	NULL},
	{NULL},			// null-terminated
};

static const UgData1Interface	ug_plugin_setting_iface =
{
	sizeof (struct UgPluginSetting),
	"PluginSetting",
	ug_plugin_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// CommandlineSetting
static const UgDataEntry	ug_commandline_setting_entry[] =
{
	{"quiet",			G_STRUCT_OFFSET (struct UgCommandlineSetting, quiet),			UG_TYPE_INT,	NULL,	NULL},
	{"CategoryIndex",	G_STRUCT_OFFSET (struct UgCommandlineSetting, category_index),	UG_TYPE_INT,	NULL,	NULL},
	{NULL},			// null-terminated
};

static const UgData1Interface	ug_commandline_setting_iface =
{
	sizeof (struct UgCommandlineSetting),
	"CommandlineSetting",
	ug_commandline_setting_entry,
	NULL, NULL, NULL,
};

// ----------------------------------------------------------------------------
// UgSetting
static const UgDataEntry	ug_setting_data_entry[] =
{
	{"LaunchApp",		G_STRUCT_OFFSET (UgSetting, launch.active),		UG_TYPE_INT,		NULL,	NULL},
	{"LaunchAppTypes",	G_STRUCT_OFFSET (UgSetting, launch.types),		UG_TYPE_STRING,		NULL,	NULL},
	{"AutoSave",		G_STRUCT_OFFSET (UgSetting, auto_save.active),	UG_TYPE_INT,		NULL,	NULL},
	{"AutoSaveInterval",G_STRUCT_OFFSET (UgSetting, auto_save.interval),UG_TYPE_INT,		NULL,	NULL},
	{"DownloadColumn",	G_STRUCT_OFFSET (UgSetting, download_column),	UG_TYPE_STATIC,		NULL,	NULL},
	{"Summary",			G_STRUCT_OFFSET (UgSetting, summary),			UG_TYPE_STATIC,		NULL,	NULL},
	{"Window",			G_STRUCT_OFFSET (UgSetting, window),			UG_TYPE_STATIC,		NULL,	NULL},
	{"UserInterface",	G_STRUCT_OFFSET (UgSetting, ui),				UG_TYPE_STATIC,		NULL,	NULL},
	{"Clipboard",		G_STRUCT_OFFSET (UgSetting, clipboard),			UG_TYPE_STATIC,		NULL,	NULL},
	{"SpeedLimit",		G_STRUCT_OFFSET (UgSetting, speed_limit),		UG_TYPE_STATIC,		NULL,	NULL},
	{"Scheduler",		G_STRUCT_OFFSET (UgSetting, scheduler),			UG_TYPE_STATIC,		NULL,	NULL},
	{"Commandline",		G_STRUCT_OFFSET (UgSetting, commandline),		UG_TYPE_STATIC,		NULL,	NULL},
	{"Plug-in",			G_STRUCT_OFFSET (UgSetting, plugin),			UG_TYPE_STATIC,		NULL,	NULL},
//	{"OfflineMode",		G_STRUCT_OFFSET (UgSetting, offline_mode),		UG_TYPE_INT,		NULL,	NULL},
//	{"Shutdown",		G_STRUCT_OFFSET (UgSetting, shutdown),			UG_TYPE_INT,		NULL,	NULL},
	{"FolderList",		G_STRUCT_OFFSET (UgSetting, folder_list),		UG_TYPE_CUSTOM,		ug_string_list_in_markup,	ug_string_list_to_markup},
	{NULL},				// null-terminated
};

static const UgData1Interface	ug_setting_iface =
{
	sizeof (UgSetting),			// instance_size
	"UgSetting",				// name
	ug_setting_data_entry,	// entry
	NULL, NULL, NULL,
};


// ----------------------------------------------------------------------------
// "FolderList" UgMarkup functions
//
static void ug_string_list_start_element (GMarkupParseContext*	context,
                                          const gchar*		element_name,
                                          const gchar**		attr_names,
                                          const gchar**		attr_values,
                                          GList**			string_list,
                                          GError**			error)
{
	guint	index;

	for (index=0; attr_names[index]; index++) {
		if (strcmp (attr_names[index], "value") == 0)
			*string_list = g_list_prepend (*string_list, g_strdup (attr_values[index]));
	}

	// skip end_element() one times.
	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
}

// GList**  user_data
static GMarkupParser	ug_string_list_parser =
{
	(gpointer) ug_string_list_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL, NULL, NULL
};

static void	ug_string_list_in_markup (GList** string_list, GMarkupParseContext* context)
{
	g_markup_parse_context_push (context, &ug_string_list_parser, string_list);
}

static void	ug_string_list_to_markup (GList** string_list, UgMarkup* markup)
{
	GList*	link;

	for (link = g_list_last (*string_list);  link;  link = link->prev) {
		ug_markup_write_element_start	(markup, "string value='%s'", link->data);
		ug_markup_write_element_end	(markup, "string");
	}
}

// ----------------------------------------------------------------------------
// "UgSchedulerSetting" UgMarkup functions
//
void ug_schedule_state_text (GMarkupParseContext *context,
                             const gchar         *text,
                             gsize                text_len,
                             guint              (*state)[7][24],
                             GError             **error)
{
	guint		weekdays, dayhours;

	for (weekdays = 0;  weekdays < 7;  weekdays++) {
		for (dayhours = 0;  dayhours < 24;  dayhours++) {
			(*state)[weekdays][dayhours] = atoi (text);
			text = strchr (text, ',');
			if (text)
				text++;
		}
	}
}

// guint (*state)[7][24]
static GMarkupParser	ug_schedule_state_parser =
{
	(gpointer) NULL,
	(gpointer) g_markup_parse_context_pop,
	(gpointer) ug_schedule_state_text,
	NULL, NULL
};

static void	ug_schedule_state_in_markup (guint (*state)[7][24], GMarkupParseContext* context)
{
	g_markup_parse_context_push (context, &ug_schedule_state_parser, state);
}

static void	ug_schedule_state_to_markup (guint (*state)[7][24], UgMarkup* markup)
{
	guint		weekdays, dayhours;
	GString*	gstr;

	gstr = g_string_sized_new (2 * 7 * 24 + 1);
	for (weekdays = 0;  weekdays < 7;  weekdays++) {
		for (dayhours = 0;  dayhours < 24;  dayhours++) {
			g_string_append_printf (gstr, "%u,",
					(*state)[weekdays][dayhours]);
		}
	}

	ug_markup_write_text (markup, gstr->str, gstr->len);
	g_string_free (gstr, TRUE);
}

// ----------------------------------------------------------------------------
// "UgSetting" UgMarkup functions
//
static void ug_setting_start_element (GMarkupParseContext*	context,
                                      const gchar*			element_name,
                                      const gchar**			attr_names,
                                      const gchar**			attr_values,
                                      UgSetting*			setting,
                                      GError**				error)
{
	guint	index;

//	if (strcmp (element_name, "UgSetting") != 0) {
//		g_set_error (error, G_MARKUP_ERROR,
//				G_MARKUP_ERROR_UNKNOWN_ELEMENT, "Unknown element");
//		return;
//	}

	for (index=0; attr_names[index]; index++) {
		if (strcmp (attr_names[index], "version") != 0)
			continue;
		if (strcmp (attr_values[index], "1") == 0) {
			g_markup_parse_context_push (context,
					&ug_data1_parser, setting);
			return;
		}
	}

	g_set_error (error, G_MARKUP_ERROR,
			G_MARKUP_ERROR_UNKNOWN_ELEMENT, "Unknown element");
}

// UgSetting*  user_data
static GMarkupParser	ug_setting_parser =
{
	(gpointer) ug_setting_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL, NULL, NULL
};

// ----------------------------------------------------------------------------
// "UgSetting" functions
//
void	ug_setting_init (UgSetting* setting)
{
	guint			weekdays, dayhours;

	setting->iface = &ug_setting_iface;

	// "SummarySetting"
	setting->summary.iface = &ug_summary_setting_iface;
	setting->summary.name     = TRUE;
	setting->summary.folder   = TRUE;
	setting->summary.category = FALSE;
	setting->summary.url      = FALSE;
	setting->summary.message  = TRUE;

	// "DownloadColumnSetting"
	setting->download_column.iface = &ug_download_column_setting_iface;
	setting->download_column.changed_count = 1;
	setting->download_column.completed    = TRUE;
	setting->download_column.total        = TRUE;
	setting->download_column.percent      = TRUE;
	setting->download_column.elapsed      = TRUE;
	setting->download_column.left         = TRUE;
	setting->download_column.speed        = TRUE;
	setting->download_column.upload_speed = TRUE;
	setting->download_column.uploaded     = FALSE;
	setting->download_column.ratio        = TRUE;
	setting->download_column.retry        = TRUE;
	setting->download_column.category     = FALSE;
	setting->download_column.url          = FALSE;
	setting->download_column.added_on     = TRUE;
	setting->download_column.completed_on = FALSE;
	// default sorted column
	setting->download_column.sort.nth     = 0;
	setting->download_column.sort.order   = 0;

	// "WindowSetting"
	setting->window.iface = &ug_window_setting_iface;
	setting->window.toolbar   = TRUE;
	setting->window.statusbar = TRUE;
	setting->window.category  = TRUE;
	setting->window.summary   = TRUE;
	setting->window.banner    = TRUE;
	setting->window.x = 0;
	setting->window.y = 0;
	setting->window.width = 0;
	setting->window.height = 0;
	setting->window.maximized = FALSE;

	// "UserInterfaceSetting"
	setting->ui.iface = &ug_user_interface_setting_iface;
	setting->ui.close_confirmation = TRUE;
	setting->ui.close_action = 0;
	setting->ui.delete_confirmation = TRUE;
	setting->ui.show_trayicon = TRUE;
	setting->ui.start_in_tray = FALSE;
	setting->ui.start_in_offline_mode = FALSE;
	setting->ui.start_notification = TRUE;
	setting->ui.sound_notification = TRUE;
	setting->ui.apply_recently = TRUE;
#ifdef HAVE_APP_INDICATOR
	setting->ui.app_indicator = TRUE;
#endif

	// "ClipboardSetting"
	setting->clipboard.iface = &ug_clipboard_setting_iface;
	g_free (setting->clipboard.pattern);
	setting->clipboard.pattern = g_strdup (UG_APP_GTK_CLIPBOARD_PATTERN);
	setting->clipboard.monitor = TRUE;
	setting->clipboard.quiet = FALSE;
	setting->clipboard.nth_category = 0;

	// "SpeedLimitSetting"
	setting->speed_limit.iface = &ug_speed_limit_setting_iface;
	setting->speed_limit.normal.upload = 0;
	setting->speed_limit.normal.download = 0;
	setting->speed_limit.scheduler.upload = 0;
	setting->speed_limit.scheduler.download = 0;

	// "SchedulerSetting"
	setting->scheduler.iface = &ug_scheduler_setting_iface;
	setting->scheduler.enable = FALSE;
	for (weekdays = 0;  weekdays < 7;  weekdays++) {
		for (dayhours = 0;  dayhours < 24;  dayhours++)
			setting->scheduler.state[weekdays][dayhours] = UG_SCHEDULE_NORMAL;
	}

	// "CommandlineSetting"
	setting->commandline.iface = &ug_commandline_setting_iface;
	setting->commandline.quiet = FALSE;
	setting->commandline.category_index = -1;

	// "PluginSetting"
	setting->plugin.iface = &ug_plugin_setting_iface;
	setting->plugin.aria2.enable = FALSE;
	setting->plugin.aria2.launch = TRUE;
	setting->plugin.aria2.shutdown = TRUE;
	setting->plugin.aria2.path = g_strdup ("aria2c");
	setting->plugin.aria2.args = g_strdup ("--enable-rpc=true -D --check-certificate=false");
	setting->plugin.aria2.uri  = g_strdup ("http://localhost:6800/rpc");

	// "FolderList"
	setting->folder_list = NULL;

	// Others
	setting->offline_mode = FALSE;
	setting->when_complete = 0;
	setting->launch.active = TRUE;
	setting->launch.types = g_strdup (UG_APP_GTK_LAUNCH_APP_TYPES);
	setting->auto_save.active = TRUE;
	setting->auto_save.interval = 3;
}

gboolean	ug_setting_save (UgSetting* setting, const gchar* file)
{
	UgMarkup*	markup;

	markup = ug_markup_new ();
	if (ug_markup_write_start (markup, file, TRUE)) {
		ug_markup_write_element_start	(markup, "UgSetting version='1'");
		ug_data1_write_markup ((UgData1*) setting, markup);
		ug_markup_write_element_end	(markup, "UgSetting");
		ug_markup_write_end (markup);
		return TRUE;
	}
	return FALSE;
}

gboolean	ug_setting_load (UgSetting* setting, const gchar* file)
{
	return ug_markup_parse (file, &ug_setting_parser, setting);
}

