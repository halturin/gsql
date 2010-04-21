/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2010  Taras Halturin  halturin@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */


#include <libgsql/conf.h>
#include "color-hinter.h"


struct _GSQLColorHinterPrivate
{
	GHashTable *storedch;

	GdkColor	bg;
	GdkColor	fg;
	GtkLabel	*label;
	GtkWidget 	*ebox;

};


G_DEFINE_TYPE (GSQLColorHinter, gsql_colorhinter, GTK_TYPE_TOOL_ITEM);

static void
gsql_colorhinter_dispose (GObject *obj)
{
//	GSQLColorHinter *ch = GSQL_COLORHINTER (obj);
		
	G_OBJECT_CLASS (gsql_colorhinter_parent_class)->dispose (obj);
}


static void 
gsql_colorhinter_finalize (GObject *obj)
{
	GSQLColorHinter *ch = GSQL_COLORHINTER (obj);

	g_free (ch->private);
	
	G_OBJECT_CLASS (gsql_colorhinter_parent_class)->finalize (obj);
}

static void
gsql_colorhinter_class_init (GSQLColorHinterClass *class)
{
	GSQL_TRACE_FUNC

	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->dispose = gsql_colorhinter_dispose;
	object_class->finalize = gsql_colorhinter_finalize;
}

static void
gsql_colorhinter_init (GSQLColorHinter *ch)
{
	GSQL_TRACE_FUNC

	ch->private = g_new0 (GSQLColorHinterPrivate, 1);

}

static void
gsql_colorhinter_update_color (GSQLColorHinter *ch)
{
	GSQL_TRACE_FUNC
	
	ch->private->fg.red = ~ch->private->bg.red;
	ch->private->fg.green = ~ch->private->bg.green;
	ch->private->fg.blue = ~ch->private->bg.blue;

	gtk_widget_modify_bg (GTK_WIDGET (ch->private->ebox), 
		    					GTK_STATE_NORMAL, &ch->private->bg);

	gtk_widget_modify_fg (GTK_WIDGET (ch->private->label), 
		    					GTK_STATE_NORMAL, &ch->private->fg);
}

static void
on_button_pressed (GtkWidget *widget, GdkEventButton *event,
    				gpointer user_data)
{
	GtkWidget *dialog = NULL;
	GtkWidget *selection;
	GSQLColorHinter *ch = user_data;
	GtkWidget *button;
	guint res;

#define GSQL_CH_RESPONSE_CLEAR 10

	dialog = gtk_color_selection_dialog_new (N_("Set the color hint for the current session"));
	
	button = gtk_dialog_add_button (GTK_DIALOG (dialog), N_("Clear"), 
	    								GSQL_CH_RESPONSE_CLEAR);
	gtk_box_reorder_child (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (dialog))), 
	    					button, 0);
	
	selection = gtk_color_selection_dialog_get_color_selection (GTK_COLOR_SELECTION_DIALOG (dialog));
	
	gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (selection), 
	    									&ch->private->bg);
	gtk_color_selection_set_has_palette (GTK_COLOR_SELECTION (selection), 
	    									TRUE);

	res = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (res)
	{
		case GTK_RESPONSE_OK:

			gtk_color_selection_get_current_color (GTK_COLOR_SELECTION (selection), 
	    									&ch->private->bg);

			gsql_colorhinter_update_color (ch);
		
			g_debug ("res = OK");

			break;

		case GSQL_CH_RESPONSE_CLEAR:
			
			gsql_colorhinter_clear_color (ch);

			break;		
	}

	gtk_widget_destroy (dialog);

}

void
gsql_colorhinter_clear_color (GSQLColorHinter *ch)
{
	GSQL_TRACE_FUNC

	gtk_widget_modify_bg (GTK_WIDGET (ch->private->ebox), 
		    					GTK_STATE_NORMAL, NULL);

	gtk_widget_modify_fg (GTK_WIDGET (ch->private->label), 
		    					GTK_STATE_NORMAL, NULL);

}


void 
gsql_colorhinter_save_color (GSQLColorHinter *ch, const gchar *name)
{
	GSQL_TRACE_FUNC
	
	g_return_if_fail (GSQL_IS_COLORHINTER (ch));
	g_return_if_fail (name != NULL);

	//	save to /apps/gsql/ui/colorhinter

}


void
gsql_colorhinter_load_color (GSQLColorHinter *ch, const gchar *name)
{
	GSQL_TRACE_FUNC

	gchar *colorname;
	
	g_return_if_fail (GSQL_IS_COLORHINTER (ch));

	colorname = (gchar *) g_hash_table_lookup (ch->private->storedch, name);

	if (!colorname)
	{
		gtk_widget_modify_bg (GTK_WIDGET (ch->private->ebox), 
		    					GTK_STATE_NORMAL, NULL);

		gtk_widget_modify_fg (GTK_WIDGET (ch->private->label), 
		    					GTK_STATE_NORMAL, NULL);

		gtk_label_set_text (GTK_LABEL (ch->private->label),
		    				N_("<small>Color\nHint</small>"));

		return;
	}

	gsql_colorhinter_update_color (ch);
	
}

GSQLColorHinter *
gsql_colorhinter_new ()
{
	GSQL_TRACE_FUNC

	GSQLColorHinter *ch;

	ch = g_object_new (GSQL_COLORHINTER_TYPE, NULL);

	ch->private->ebox = gtk_event_box_new ();
	
	ch->private->label = GTK_LABEL (gtk_label_new (N_("<small>Color\nHint</small>")));

	g_signal_connect (ch->private->ebox, "button-press-event",
	    				G_CALLBACK (on_button_pressed), ch);
	
	gtk_widget_set_tooltip_text (GTK_WIDGET (ch->private->label), 
	    						 N_("Click here to set the color"));

	gtk_container_add (GTK_CONTAINER (ch->private->ebox), 
	    								GTK_WIDGET (ch->private->label));
	
	gtk_label_set_use_markup (GTK_LABEL (ch->private->label), TRUE);
	gtk_label_set_justify (GTK_LABEL (ch->private->label), GTK_JUSTIFY_CENTER);
	gtk_misc_set_padding (GTK_MISC (ch->private->label), 16, 0);
	
	gtk_container_add (GTK_CONTAINER (ch), ch->private->ebox);

	return ch;
}
