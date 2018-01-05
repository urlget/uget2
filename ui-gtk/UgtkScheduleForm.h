/*
 *
 *   Copyright (C) 2005-2018 by C.H. Huang
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

#ifndef UGTK_SCHEDULE_FORM_H
#define UGTK_SCHEDULE_FORM_H

#include <UgtkSetting.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif


struct UgtkScheduleForm
{
	GtkWidget*      self;

	GtkWidget*      enable;

	GtkWidget*      drawing;
	guint           drawing_offset;

	GtkWidget*      caption;
	GtkLabel*       time_tips;
	GtkSpinButton*  spin_speed;

	int             state[7][24];	// 1 week, 7 days, 24 hours
	UgtkScheduleState  last_state;
};

void  ugtk_schedule_form_init (struct UgtkScheduleForm* sform);
void  ugtk_schedule_form_get  (struct UgtkScheduleForm* sform, UgtkSetting* setting);
void  ugtk_schedule_form_set  (struct UgtkScheduleForm* sform, UgtkSetting* setting);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_SCHEDULE_FORM_H

