/*
 *
 *   Copyright (C) 2005-2017 by C.H. Huang
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

#include <memory.h>
#include <UgtkScheduleForm.h>

#include <glib/gi18n.h>

#define	COLOR_DISABLE_R     0.5
#define	COLOR_DISABLE_G     0.5
#define	COLOR_DISABLE_B     0.5

static const gdouble  colors[UGTK_SCHEDULE_N_STATE][3] =
{
	{1.0,   1.0,   1.0},        // UGTK_SCHEDULE_TURN_OFF
	{1.0,   0.752, 0.752},      // UGTK_SCHEDULE_UPLOAD_ONLY - reserve
	{0.552, 0.807, 0.552},      // UGTK_SCHEDULE_LIMITED_SPEED
//	{0.0,   0.658, 0.0},        // UGTK_SCHEDULE_NORMAL
	{0.0,   0.758, 0.0},        // UGTK_SCHEDULE_NORMAL
};

static const gchar*  week_days[7] =
{
	N_("Mon"),
	N_("Tue"),
	N_("Wed"),
	N_("Thu"),
	N_("Fri"),
	N_("Sat"),
	N_("Sun"),
};

// UgtkGrid
static struct {
	int  width;
	int  height;
	int  width_and_line;
	int  height_and_line;
	int  width_all;
	int  height_all;
} UgtkGrid;

static void       ugtk_grid_global_init (int width, int height);
static GtkWidget* ugtk_grid_new (const gdouble* rgb_array);
static gboolean   ugtk_grid_draw (GtkWidget* widget, cairo_t* cr, const gdouble* rgb_array);

// signal handler
static void     on_enable_toggled (GtkToggleButton* togglebutton, struct UgtkScheduleForm* sform);
static gboolean on_draw_callback (GtkWidget* widget, cairo_t* cr, struct UgtkScheduleForm* sform);
static gboolean on_button_press_event (GtkWidget* widget, GdkEventMotion* event, struct UgtkScheduleForm* sform);
static gboolean on_motion_notify_event (GtkWidget* widget, GdkEventMotion* event, struct UgtkScheduleForm* sform);
static gboolean on_leave_notify_event (GtkWidget* menu, GdkEventCrossing* event, struct UgtkScheduleForm* sform);


void  ugtk_schedule_form_init (struct UgtkScheduleForm* sform)
{
	PangoContext*  context;
	PangoLayout*   layout;
	GtkWidget*  widget;
	GtkGrid*    caption;
	GtkBox*     hbox;
	GtkBox*     vbox;
	int         text_width, text_height;

	sform->self = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	vbox = (GtkBox*) sform->self;

	// Enable Scheduler
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*)hbox, FALSE, FALSE, 2);
	widget = gtk_check_button_new_with_mnemonic (_("_Enable Scheduler"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	gtk_box_pack_start (hbox, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), TRUE, TRUE, 2);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_enable_toggled), sform);
	sform->enable = widget;

	// initialize UgtkGrid
	context = gtk_widget_get_pango_context (widget);
	layout = pango_layout_new (context);
	pango_layout_set_text (layout, gettext (week_days[0]), -1);
	pango_layout_get_pixel_size (layout, &text_width, &text_height);
	g_object_unref (layout);
	ugtk_grid_global_init (text_height, text_height + 2);

	// drawing area
	widget = gtk_drawing_area_new ();
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 2);
//	gtk_widget_set_has_window (widget, FALSE);
	gtk_widget_set_size_request (widget,
			UgtkGrid.width_all + text_width + 32, UgtkGrid.height_all);
	gtk_widget_add_events (widget,
			GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect (widget, "draw",
			G_CALLBACK (on_draw_callback), sform);
	g_signal_connect (widget, "button-press-event",
			G_CALLBACK (on_button_press_event), sform);
	g_signal_connect (widget, "motion-notify-event",
			G_CALLBACK (on_motion_notify_event), sform);
	g_signal_connect (widget, "leave-notify-event",
			G_CALLBACK (on_leave_notify_event), sform);
	sform->drawing = widget;

	// grid for tips, SpinButton
	sform->caption = gtk_grid_new ();
	gtk_box_pack_start (vbox, sform->caption, FALSE, FALSE, 2);
//	gtk_container_set_border_width (GTK_CONTAINER (sform->caption), 10);
	caption = (GtkGrid*) sform->caption;
	// time tips
	widget = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (widget), (gfloat)0.4, (gfloat)0.5);	// left, center
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 0, 0, 5, 1);
	sform->time_tips = GTK_LABEL (widget);

	// Turn off
	widget = ugtk_grid_new (colors[UGTK_SCHEDULE_TURN_OFF]);
	g_object_set (widget, "margin", 3, NULL);
	gtk_grid_attach (caption, widget, 0, 1, 1, 1);
	// Turn off - label
	widget = gtk_label_new (_("Turn off"));
	gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);	// left, center
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 1, 1, 1, 1);
	// Turn off - help label
	widget = gtk_label_new (_("- stop all task"));
	gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);	// left, center
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 2, 1, 2, 1);

	// Normal
	widget = ugtk_grid_new (colors[UGTK_SCHEDULE_NORMAL]);
	g_object_set (widget, "margin", 3, NULL);
	gtk_grid_attach (caption, widget, 0, 2, 1, 1);
	// Normal - label
	widget = gtk_label_new (_("Normal"));
	gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);	// left, center
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 1, 2, 1, 1);
	// Normal - help label
	widget = gtk_label_new (_("- run task normally"));
	gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);	// left, center
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 2, 2, 2, 1);
/*
	// Speed limit
	widget = ugtk_grid_new (colors[UGTK_SCHEDULE_LIMITED_SPEED]);
	g_object_set (widget, "margin", 3, NULL);
	gtk_grid_attach (caption, widget, 0, 3, 1, 1);
	// Speed limit - label
	widget = gtk_label_new (_("Limited speed"));
	gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);	// left, center
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 1, 3, 1, 1);
	// Speed limit - SpinButton
	widget = gtk_spin_button_new_with_range (5, 99999999, 1);
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 2, 3, 1, 1);
	sform->spin_speed = (GtkSpinButton*) widget;
	// Speed limit - KiB/s label
	widget = gtk_label_new (_("KiB/s"));
	gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);	// left, center
	g_object_set (widget, "margin", 2, NULL);
	gtk_grid_attach (caption, widget, 3, 3, 1, 1);
 */
	// change sensitive state
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sform->enable), FALSE);
	gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (sform->enable));
	gtk_widget_show_all (sform->self);
}

void  ugtk_schedule_form_get (struct UgtkScheduleForm* sform, UgtkSetting* setting)
{
//	gint  value;

	memcpy (setting->scheduler.state.at, sform->state, sizeof (sform->state));
	setting->scheduler.enable = gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (sform->enable));

//	value = gtk_spin_button_get_value_as_int (sform->spin_speed);
//	setting->scheduler.speed_limit = value * 1024;
}

void  ugtk_schedule_form_set (struct UgtkScheduleForm* sform, UgtkSetting* setting)
{
//	gint  value;

	memcpy (sform->state, setting->scheduler.state.at, sizeof (sform->state));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sform->enable), setting->scheduler.enable);
	gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (sform->enable));

//	value = setting->scheduler.speed_limit / 1024;
//	gtk_spin_button_set_value (sform->spin_speed, value);
}


// ----------------------------------------------------------------------------
// signal handler
//
static void on_enable_toggled (GtkToggleButton* togglebutton, struct UgtkScheduleForm* sform)
{
	gboolean  active;

	active = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (sform->drawing, active);
	gtk_widget_set_sensitive (sform->caption, active);
}

static gboolean on_draw_callback (GtkWidget* widget, cairo_t* cr, struct UgtkScheduleForm* sform)
{
	gboolean  sensitive;
	gint      y, x;
	gdouble   cy, cx, ox;
	PangoContext*  context;
	PangoLayout*   layout;
	PangoFontDescription*  desc;

	cairo_set_line_width (cr, 1);
	sensitive = gtk_widget_get_sensitive (widget);

	// setup Pango
	context = gtk_widget_get_pango_context (widget);
	desc = pango_context_get_font_description (context);
	layout = pango_cairo_create_layout (cr);
	pango_layout_set_font_description (layout, desc);

	// week days
	// ox = x offset
	for (ox = 0, cy = 0.5, y = 0;  y < 7;  y++, cy+=UgtkGrid.height_and_line) {
		cairo_move_to (cr, 1, cy);
		pango_layout_set_text (layout, gettext (week_days[y]), -1);
		pango_cairo_update_layout (cr, layout);
		pango_cairo_show_layout (cr, layout);
//		pango_layout_get_size (layout, &x, NULL);
//		x /= PANGO_SCALE;
		pango_layout_get_pixel_size (layout, &x, NULL);
		if (x + 4 > ox)
			ox = x + 4;
	}
	g_object_unref (layout);

	if (sform->drawing_offset == 0)
		sform->drawing_offset = ox;
	// draw grid columns
	for (cx = 0.5;  cx <= UgtkGrid.width_all;  cx += UgtkGrid.width_and_line) {
		cairo_move_to (cr, ox + cx, 0 + 0.5);
		cairo_line_to (cr, ox + cx, UgtkGrid.height_all - 1.0 + 0.5);
	}
	// draw grid rows
	for (cy = 0.5;  cy <= UgtkGrid.height_all;  cy += UgtkGrid.height_and_line) {
		cairo_move_to (cr, ox + 0.5, cy);
		cairo_line_to (cr, ox + UgtkGrid.width_all - 1.0 + 0.5, cy);
	}
	cairo_stroke (cr);

	// fill grid
	if (sensitive == FALSE) {
		cairo_set_source_rgb (cr,
				COLOR_DISABLE_R,
				COLOR_DISABLE_G,
				COLOR_DISABLE_B);
	}
	for (cy = 1.5, y = 0;  y < 7;  y++, cy+=UgtkGrid.height_and_line) {
		for (cx = 1.5+ox, x = 0;  x < 24;  x++, cx+=UgtkGrid.width_and_line) {
			if (sensitive) {
				cairo_set_source_rgb (cr,
						colors [sform->state[y][x]][0],
						colors [sform->state[y][x]][1],
						colors [sform->state[y][x]][2]);
			}
			cairo_rectangle (cr,
					cx,
					cy,
					UgtkGrid.width  - 0.5,
					UgtkGrid.height - 0.5);
			cairo_fill (cr);
		}
	}

	return FALSE;
}

static gboolean on_button_press_event (GtkWidget *widget, GdkEventMotion *event, struct UgtkScheduleForm* sform)
{
	gint      x, y;
	cairo_t*  cr;
	UgtkScheduleState  state;

	x  = (event->x - sform->drawing_offset) / UgtkGrid.width_and_line;
	y  =  event->y / UgtkGrid.height_and_line;
	if (x < 0 || y < 0 || x >= 24 || y >= 7)
		return FALSE;

	state = (sform->state[y][x] == UGTK_SCHEDULE_TURN_OFF) ? UGTK_SCHEDULE_NORMAL : UGTK_SCHEDULE_TURN_OFF;
//	state = sform->state[y][x] + 1;
//	if (state == UGTK_SCHEDULE_UPLOAD_ONLY)
//		state++;
//	if (state  > UGTK_SCHEDULE_NORMAL)
//		state  = UGTK_SCHEDULE_TURN_OFF;
	sform->state[y][x] = state;
	sform->last_state = state;
	// cairo
	cr = gdk_cairo_create (gtk_widget_get_window (widget));
	cairo_set_source_rgb (cr,
			colors [state][0],
			colors [state][1],
			colors [state][2]);
	cairo_rectangle (cr,
			(gdouble)x * UgtkGrid.width_and_line  + 1.0 + 0.5 + sform->drawing_offset,
			(gdouble)y * UgtkGrid.height_and_line + 1.0 + 0.5,
			UgtkGrid.width  - 0.5,
			UgtkGrid.height - 0.5);
	cairo_fill (cr);
	cairo_destroy (cr);

	return TRUE;
}

static gboolean on_motion_notify_event (GtkWidget *widget, GdkEventMotion *event, struct UgtkScheduleForm* sform)
{
	gint        x, y;
	gchar*      string;
	cairo_t*    cr;
	GdkWindow*  gdkwin;
	GdkModifierType  mod;
	UgtkScheduleState  state;

	gdkwin = gtk_widget_get_window (widget);
	gdk_window_get_device_position (gdkwin, event->device, &x, &y, &mod);
	x -= sform->drawing_offset;
	x /= UgtkGrid.width_and_line;
	y /= UgtkGrid.height_and_line;
	if (x < 0 || y < 0 || x >= 24 || y >= 7) {
		// clear time_tips
		gtk_label_set_text (sform->time_tips, "");
		return FALSE;
	}
	// update time_tips
	string = g_strdup_printf ("%s, %.2d:00 - %.2d:59",
			gettext (week_days[y]), x, x);
	gtk_label_set_text (sform->time_tips, string);
	g_free (string);
	// if no button press
	if ((mod & GDK_BUTTON1_MASK) == 0)
		return FALSE;

	state = sform->last_state;
	sform->state[y][x] = state;
	// cairo
	cr = gdk_cairo_create (gdkwin);
	cairo_rectangle (cr,
			sform->drawing_offset, 0,
			UgtkGrid.width_all, UgtkGrid.height_all);
	cairo_clip (cr);
	cairo_set_source_rgb (cr,
			colors [state][0],
			colors [state][1],
			colors [state][2]);
	cairo_rectangle (cr,
			(gdouble)x * UgtkGrid.width_and_line  + 1.0 + 0.5 + sform->drawing_offset,
			(gdouble)y * UgtkGrid.height_and_line + 1.0 + 0.5,
			UgtkGrid.width  - 0.5,
			UgtkGrid.height - 0.5);
	cairo_fill (cr);
	cairo_destroy (cr);

	return TRUE;
}

static gboolean on_leave_notify_event (GtkWidget* menu, GdkEventCrossing* event, struct UgtkScheduleForm* sform)
{
	gtk_label_set_text (sform->time_tips, "");
	return TRUE;
}


// ----------------------------------------------------------------------------
// UgtkGrid
//
static void  ugtk_grid_global_init (int width, int height)
{
	UgtkGrid.width  = width;
	UgtkGrid.height = height;
	UgtkGrid.width_and_line  = UgtkGrid.width  + 1;
	UgtkGrid.height_and_line = UgtkGrid.height + 1;
	UgtkGrid.width_all  = UgtkGrid.width_and_line * 24 + 1;
	UgtkGrid.height_all = UgtkGrid.height_and_line * 7 + 1;
}

static GtkWidget*  ugtk_grid_new (const gdouble* rgb_array)
{
	GtkWidget*  widget;

	widget = gtk_drawing_area_new ();
	gtk_widget_set_size_request (widget, UgtkGrid.width + 2, UgtkGrid.height + 2);
	gtk_widget_add_events (widget, GDK_POINTER_MOTION_MASK);

	g_signal_connect (widget, "draw",
			G_CALLBACK (ugtk_grid_draw), (gpointer) rgb_array);

	return widget;
}

static gboolean ugtk_grid_draw (GtkWidget* widget, cairo_t* cr, const gdouble* rgb_array)
{
	GtkAllocation  allocation;
	gdouble        x, y, width, height;

	gtk_widget_get_allocation (widget, &allocation);
	x = 0.5;
	y = 0.5;
	width  = (gdouble) (allocation.width - 1);
	height = (gdouble) (allocation.height - 1);
	cairo_set_line_width (cr, 1);
	cairo_rectangle (cr, x, y, width, height);
	cairo_stroke (cr);
	if (gtk_widget_get_sensitive (widget)) {
		cairo_set_source_rgb (cr,
				rgb_array [0],
				rgb_array [1],
				rgb_array [2]);
	}
	else {
		cairo_set_source_rgb (cr,
				COLOR_DISABLE_R,
				COLOR_DISABLE_G,
				COLOR_DISABLE_B);
	}
	cairo_rectangle (cr, x + 1.0, y + 1.0, width - 2.0, height - 2.0);
	cairo_fill (cr);

	return FALSE;
}

