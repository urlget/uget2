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

#ifndef UG_SETTING_H
#define UG_SETTING_H

#ifdef	HAVE_CONFIG_H
#include <config.h>
#endif
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION					"1.10.5"
#endif

#include "UgData1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UG_APP_GTK_NAME					"uGet"
#define UG_APP_GTK_VERSION				PACKAGE_VERSION " (stable branch)"
// default setting
#define UG_APP_GTK_CLIPBOARD_PATTERN	"BIN|ZIP|GZ|7Z|Z|TAR|TGZ|BZ2|LZH|A[0-9]?|RAR|R[0-9][0-9]|3GP|AAC|FLAC|M4A|M4P|MP3|OGG|WAV|WMA|MP4|OGV|MKV|AVI|MOV|WMV|FLV|F4V|MPG|MPEG|RMVB|RPM|DEB|EXE"
#define UG_APP_GTK_LAUNCH_APP_TYPES		"torrent"

typedef struct	UgSetting				UgSetting;
typedef enum	UgScheduleState			UgScheduleState;

enum UgScheduleState
{
	UG_SCHEDULE_TURN_OFF,
	UG_SCHEDULE_UPLOAD_ONLY,	// reserve
	UG_SCHEDULE_LIMITED_SPEED,
	UG_SCHEDULE_NORMAL,

	UG_SCHEDULE_N_STATE,
};

// <UgSetting>
struct UgSetting
{
	const UgData1Interface*	iface;			// for UgMarkup parse/write

	// "DownloadColumnSetting"
	struct UgDownloadColumnSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		guint		changed_count;	// sync with UgDownloadWidget.changed_count

		gboolean	completed;
		gboolean	total;
		gboolean	percent;
		gboolean	elapsed;		// consuming time
		gboolean	left;			// remaining time
		gboolean	speed;
		gboolean	upload_speed;
		gboolean	uploaded;
		gboolean	ratio;
		gboolean	retry;
		gboolean	category;
		gboolean	url;
		gboolean	added_on;
		gboolean	completed_on;

		struct
		{
			gint		nth;
			gint		order;		// sort order
		} sort;
	} download_column;

	// "SummarySetting"
	struct UgSummarySetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		gboolean	name;
		gboolean	folder;
		gboolean	category;
		gboolean	url;
		gboolean	message;
	} summary;

	// "WindowSetting"
	struct UgWindowSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		// visible
		gboolean	toolbar;
		gboolean	statusbar;
		gboolean	category;
		gboolean	summary;
		gboolean	banner;

		gint		x;		// window position
		gint		y;
		gint		width;
		gint		height;
		gboolean	maximized;
	} window;

	struct UgUserInterfaceSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		// close_action == 0, Let user decide.
		// close_action == 1, Minimize to tray.
		// close_action == 2, Exit Uget.
		// close_confirmation == FALSE, Remember this action.
		// close_confirmation == TRUE,  Always confirm.
		gboolean	close_confirmation;
		gint		close_action;
		gboolean	delete_confirmation;
		gboolean	show_trayicon;
		gboolean	start_in_tray;
		gboolean	start_in_offline_mode;
		gboolean	start_notification;
		gboolean	sound_notification;
		gboolean	apply_recently;
#ifdef HAVE_APP_INDICATOR
		gboolean	app_indicator;
#endif
	} ui;

	// "ClipboardSetting"
	struct UgClipboardSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		gchar*		pattern;
		gboolean	monitor;
		gboolean	quiet;
		gint		nth_category;
	} clipboard;

	// "SpeedLimitSetting" - global speed limits
	struct UgSpeedLimitSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		struct {
			guint	upload;					// KiB / second
			guint	download;				// KiB / second
		} normal;

		struct {
			guint	upload;					// KiB / second
			guint	download;				// KiB / second
		} scheduler;
	} speed_limit;

	// "SchedulerSetting"
	struct UgSchedulerSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		gboolean	enable;
		guint		state[7][24];	// 1 week, 7 days, 24 hours
	} scheduler;

	// "CommandlineSetting"
	struct UgCommandlineSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		gboolean	quiet;			// --quiet
		gint		category_index;	// --category-index
	} commandline;

	// "PluginSetting"
	struct UgPluginSetting
	{
		const UgData1Interface*	iface;		// for UgMarkup parse/write

		struct
		{
			gboolean	enable;
			gboolean	launch;
			gboolean	shutdown;
			gchar*		path;
			gchar*		args;
			gchar*		uri;
		} aria2;
	} plugin;

	gboolean		offline_mode;
	guint           when_complete;    // when downloads complete

	// "FolderList"
	GList*			folder_list;
	// Others
	struct UgLaunchSetting
	{
		gboolean		active;
		gchar*			types;
	} launch;

	struct UgAutoSave
	{
		gboolean		active;
		guint			interval;
	} auto_save;
};

void	ug_setting_init  (UgSetting* setting);

gboolean	ug_setting_save  (UgSetting* setting, const gchar* file);
gboolean	ug_setting_load  (UgSetting* setting, const gchar* file);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_SETTING_H

