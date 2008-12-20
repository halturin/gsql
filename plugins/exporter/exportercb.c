/* 
 * GSQL - database development tool for GNOME
 *
 * Copyright (C) 2006-2008  Taras Halturin  halturin@gmail.com
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

 
#include <glib.h>
#include <gtk/gtk.h>
#include <libgsql/common.h>
#include <libgsql/content.h>
#include <glade/glade.h>
#include "plugin_exporter.h"

struct _export_types
{
	gint  id;
	gchar *name;
};

static struct _export_types export_types[] = 
{
	{0, "CSV"}
/*	{1, "HTML"},
	{2, "XML"},
	{3, "Plain text"} */
};

static void
on_choosebutton_activate (GtkButton *button, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GtkDialog *dialog;
	GtkWidget *target = user_data;
	static gchar *folder = NULL;
	GtkWidget *chooser;
	static gchar *uri = NULL;
	gchar *file = NULL;
	gint ret;
	
	chooser = gtk_file_chooser_dialog_new (N_("Target filename..."),
					       NULL,
					       GTK_FILE_CHOOSER_ACTION_SAVE,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					       NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(chooser), 
													TRUE);
	if (folder)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), 
											 folder);
	if (uri)
		g_free (uri); 

	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (chooser), FALSE);
	
	uri = g_strdup(gtk_entry_get_text (GTK_ENTRY (target)));
	
	if (uri)
	{
		GSQL_DEBUG ("Export: [uri=%s]", uri);
		file = g_filename_display_basename (uri);
		if (gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser), 
									   uri))
			gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), 
											   file);
		
		g_free (file);
		file = NULL;
	}
	
	gtk_window_set_transient_for (GTK_WINDOW (chooser), GTK_WINDOW (gsql_window));

	ret = gtk_dialog_run (GTK_DIALOG (chooser));
	if (ret == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		if (folder) 
			g_free (folder);

		folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (chooser));
		GSQL_DEBUG ("Export target: [file=%s]", file);
		
		if (file)
		{
			gtk_entry_set_text (GTK_ENTRY (target), file);
			g_free (file);
		}
	}
	
	gtk_widget_destroy (chooser);
	

}

static GtkWidget*
get_export_types_combo ()
{
	GtkWidget *combo = NULL;
	gint i = 0;
	GtkListStore *model;
	GtkCellRenderer *cell;
	GtkTreeIter iter;
	
	combo = gtk_combo_box_new ();
	cell = gtk_cell_renderer_text_new ();
    
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo),
                                    cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
                                        cell, "markup", 1, NULL);
	
	model = gtk_list_store_new (2, G_TYPE_INT, G_TYPE_STRING);
	gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (model));
	
	while (i < G_N_ELEMENTS (export_types))
	{
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set( GTK_LIST_STORE(model), &iter,
							0, export_types[i].id,
							1, export_types[i].name,
							-1);
		i++;
	}
	
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	
	gtk_widget_show (combo);
	return combo;
	
	
}

void
on_open_export_dialog_activate (GtkButton *button, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GladeXML* gxml;
	GtkDialog *dialog;
	GtkWidget *combo;
	GtkWidget *combo_exptype;
	GtkWidget *choosebutton;
	GtkAlignment *alig;
	GtkAlignment *alig_exptype;
	GtkRadioButton *rbutton;
	GtkWidget *target;
	GtkWidget *headers;
	GtkWidget *progressbar, *config_vbox, *save_button;
	GSList *rgroup;
	GSQLContent *content = NULL;
	GSQLSession *session = NULL;
	GSQLWorkspace *workspace = NULL;
	gchar *encoding = NULL;
	static gchar *filename = NULL;
	static gboolean include_headers = TRUE;
	static gboolean only_fetched = FALSE;
	static gint exptype = 0;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gint ret;
	
	gxml = glade_xml_new (GSQLP_EXPORTER_GLADE_DIALOG, "export_dialog", NULL);
	g_return_if_fail(gxml);
	
	dialog = (GtkDialog *) glade_xml_get_widget (gxml, "export_dialog");
	
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (gsql_window));
	
	alig = (GtkAlignment *) glade_xml_get_widget (gxml, "alignment_enc");
	alig_exptype = (GtkAlignment *) glade_xml_get_widget (gxml, "alignment_exptype");
	
	combo = (GtkWidget *) gsql_enconding_list ();
	combo_exptype = (GtkWidget *) get_export_types_combo ();
	
	target = (GtkWidget *) glade_xml_get_widget (gxml, "target");
	
	choosebutton = (GtkWidget *) glade_xml_get_widget (gxml, "choosebutton");
	
	headers = (GtkWidget *) glade_xml_get_widget (gxml, "headers");
	progressbar = (GtkWidget *) glade_xml_get_widget (gxml, "progressbar");
	config_vbox = (GtkWidget *) glade_xml_get_widget (gxml, "config_vbox");
	save_button = (GtkWidget *) glade_xml_get_widget (gxml, "save_button");
	
	g_signal_connect ((gpointer) choosebutton, "clicked",
						G_CALLBACK (on_choosebutton_activate),
						(gpointer) target);
	
	if (filename)
		gtk_entry_set_text (GTK_ENTRY (target), filename);
	
	if (exptype)
		gtk_combo_box_set_active (GTK_COMBO_BOX (combo_exptype), exptype);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (headers), include_headers);
	gtk_container_add (GTK_CONTAINER (alig), combo);
	gtk_container_add (GTK_CONTAINER (alig_exptype), combo_exptype);
	
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (gsql_window));
	
	// 2 - cancel, 3 - export complete (via gtk_dialog_response)
	while ((ret != 2) && (ret !=3))
	{
		ret = gtk_dialog_run (dialog);
		GSQL_DEBUG ("Export result dialog: [ret=%d]", ret);
	
		if (ret == 1) // Save action selected
		{
			GSQL_DEBUG ("Start exporting...");
			rbutton = (GtkRadioButton *) glade_xml_get_widget (gxml, "radiobutton1");
			rgroup = gtk_radio_button_get_group (rbutton);
		
			if (filename)
				g_free (filename);
		
			filename = g_strdup (gtk_entry_get_text (GTK_ENTRY (target)));

			GSQL_DEBUG ("GSList length: %d", g_slist_length (rgroup));
		
			if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo_exptype), &iter))
			{
				model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_exptype));
				gtk_tree_model_get (model, &iter,
							0, &exptype, -1);
			}
		
			session = gsql_session_get_active ();
			workspace = gsql_session_get_workspace (session);
			content = gsql_workspace_get_current_content (workspace);
		
			gtk_widget_show (progressbar);
			gtk_widget_hide (config_vbox);
			gtk_widget_hide (save_button);
		
			g_object_set_data (G_OBJECT (content), "dialog", dialog);
			g_object_set_data (G_OBJECT (content), "progress", progressbar);
		
			g_return_if_fail (GSQL_IS_CONTENT (content));
		
			if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
			{
				model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
				gtk_tree_model_get (model, &iter,
							1, &encoding, -1);
				GSQL_DEBUG ("Exporter: encoding selected. [%s]", encoding);
			}
		
			include_headers = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (headers));

			GSQL_DEBUG ("Exporter: [filename=%s]", filename);
			switch (exptype)
			{
				case 0: // CSV
					GSQL_DEBUG ("Export type: CSV");
					exporter_export_to_csv (content, filename, 
											encoding, include_headers);
					break;
				case 1: // HTML
					GSQL_DEBUG ("Export type: HTML");
					exporter_export_to_html (content, filename, 
											 encoding, include_headers);
					break;
				case 2: // XML
					GSQL_DEBUG ("Export type: XML");
					exporter_export_to_xml (content, filename, 
											 encoding, include_headers);
					break;
				case 3: // Plain text
					GSQL_DEBUG ("Export type: Plain text");
					exporter_export_to_plain_text (content, filename, 
											 encoding, include_headers);
					break;
			}
		}
	
	}
	
	

	if (encoding)
		g_free (encoding);
	
	gtk_widget_destroy ((GtkWidget *) dialog);
	g_object_unref(G_OBJECT(gxml));
	
}
