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


#include <gtk/gtk.h>
#include <glib.h>
#include <libgsql/common.h>

typedef struct _GSQLEncoding GSQLEncoding;
struct _GSQLEncoding
{
	gint   index;
	gchar *charset;
	gchar *name;
};

/* 
 * The original versions of the following tables are taken from GEDIT 
 */
typedef enum
{
	GSQL_ENCODING_ISO_8859_1,
	GSQL_ENCODING_ISO_8859_2,
	GSQL_ENCODING_ISO_8859_3,
	GSQL_ENCODING_ISO_8859_4,
	GSQL_ENCODING_ISO_8859_5,
	GSQL_ENCODING_ISO_8859_6,
	GSQL_ENCODING_ISO_8859_7,
	GSQL_ENCODING_ISO_8859_8,
	GSQL_ENCODING_ISO_8859_8_I,
	GSQL_ENCODING_ISO_8859_9,
	GSQL_ENCODING_ISO_8859_10,
	GSQL_ENCODING_ISO_8859_13,
	GSQL_ENCODING_ISO_8859_14,
	GSQL_ENCODING_ISO_8859_15,
	GSQL_ENCODING_ISO_8859_16,

	GSQL_ENCODING_UTF_7,
	GSQL_ENCODING_UTF_8,
	GSQL_ENCODING_UTF_16,
	GSQL_ENCODING_UTF_16_BE,
	GSQL_ENCODING_UTF_16_LE,
	GSQL_ENCODING_UTF_32,  
	GSQL_ENCODING_UCS_2,
	GSQL_ENCODING_UCS_4,

	GSQL_ENCODING_ARMSCII_8,
	GSQL_ENCODING_BIG5,
	GSQL_ENCODING_BIG5_HKSCS,
	GSQL_ENCODING_CP_866,

	GSQL_ENCODING_EUC_JP,
	GSQL_ENCODING_EUC_JP_MS,
	GSQL_ENCODING_CP932,
	GSQL_ENCODING_EUC_KR,
	GSQL_ENCODING_EUC_TW,

	GSQL_ENCODING_GB18030,
	GSQL_ENCODING_GB2312,
	GSQL_ENCODING_GBK,
	GSQL_ENCODING_GEOSTD8,
	GSQL_ENCODING_HZ,

	GSQL_ENCODING_IBM_850,
	GSQL_ENCODING_IBM_852,
	GSQL_ENCODING_IBM_855,
	GSQL_ENCODING_IBM_857,
	GSQL_ENCODING_IBM_862,
	GSQL_ENCODING_IBM_864,

	GSQL_ENCODING_ISO_2022_JP,
	GSQL_ENCODING_ISO_2022_KR,
	GSQL_ENCODING_ISO_IR_111,
	GSQL_ENCODING_JOHAB,
	GSQL_ENCODING_KOI8_R,
	GSQL_ENCODING_KOI8__R,
	GSQL_ENCODING_KOI8_U,
  
	GSQL_ENCODING_SHIFT_JIS,
	GSQL_ENCODING_TCVN,
	GSQL_ENCODING_TIS_620,
	GSQL_ENCODING_UHC,
	GSQL_ENCODING_VISCII,

	GSQL_ENCODING_WINDOWS_1250,
	GSQL_ENCODING_WINDOWS_1251,
	GSQL_ENCODING_WINDOWS_1252,
	GSQL_ENCODING_WINDOWS_1253,
	GSQL_ENCODING_WINDOWS_1254,
	GSQL_ENCODING_WINDOWS_1255,
	GSQL_ENCODING_WINDOWS_1256,
	GSQL_ENCODING_WINDOWS_1257,
	GSQL_ENCODING_WINDOWS_1258,

	GSQL_ENCODING_LAST

} GSQLEncodingIndex;

static GSQLEncoding encodings [] = {

	{ GSQL_ENCODING_ISO_8859_1,
		"ISO-8859-1", N_("Western") },
	{ GSQL_ENCODING_ISO_8859_2,
		"ISO-8859-2", N_("Central European") },
	{ GSQL_ENCODING_ISO_8859_3,
		"ISO-8859-3", N_("South European") },
	{ GSQL_ENCODING_ISO_8859_4,
		"ISO-8859-4", N_("Baltic") },
	{ GSQL_ENCODING_ISO_8859_5,
		"ISO-8859-5", N_("Cyrillic") },
	{ GSQL_ENCODING_ISO_8859_6,
		"ISO-8859-6", N_("Arabic") },
	{ GSQL_ENCODING_ISO_8859_7,
		"ISO-8859-7", N_("Greek") },
	{ GSQL_ENCODING_ISO_8859_8,
		"ISO-8859-8", N_("Hebrew Visual") },
	{ GSQL_ENCODING_ISO_8859_8_I,
		"ISO-8859-8-I", N_("Hebrew") },
	{ GSQL_ENCODING_ISO_8859_9,
		"ISO-8859-9", N_("Turkish") },
	{ GSQL_ENCODING_ISO_8859_10,
		"ISO-8859-10", N_("Nordic") },
	{ GSQL_ENCODING_ISO_8859_13,
		"ISO-8859-13", N_("Baltic") },
	{ GSQL_ENCODING_ISO_8859_14,
		"ISO-8859-14", N_("Celtic") },
	{ GSQL_ENCODING_ISO_8859_15,
		"ISO-8859-15", N_("Western") },
	{ GSQL_ENCODING_ISO_8859_16,
		"ISO-8859-16", N_("Romanian") },

	{ GSQL_ENCODING_UTF_7,
		"UTF-7", N_("Unicode") },
	{ GSQL_ENCODING_UTF_8,
		"UTF-8", N_("Unicode") },
	{ GSQL_ENCODING_UTF_16,
		"UTF-16", N_("Unicode") },
	{ GSQL_ENCODING_UTF_16_BE,
		"UTF-16BE", N_("Unicode") },
	{ GSQL_ENCODING_UTF_16_LE,
		"UTF-16LE", N_("Unicode") },
	{ GSQL_ENCODING_UTF_32,
		"UTF-32", N_("Unicode") },
	{ GSQL_ENCODING_UCS_2,
		"UCS-2", N_("Unicode") },
	{ GSQL_ENCODING_UCS_4,
		"UCS-4", N_("Unicode") },

	{ GSQL_ENCODING_ARMSCII_8,
		"ARMSCII-8", N_("Armenian") },
	{ GSQL_ENCODING_BIG5,
		"BIG5", N_("Chinese Traditional") },
	{ GSQL_ENCODING_BIG5_HKSCS,
		"BIG5-HKSCS", N_("Chinese Traditional") },
	{ GSQL_ENCODING_CP_866,
		"CP866", N_("Cyrillic/Russian") },

	{ GSQL_ENCODING_EUC_JP,
		"EUC-JP", N_("Japanese") },
	{ GSQL_ENCODING_EUC_JP_MS,
		"EUC-JP-MS", N_("Japanese") },
	{ GSQL_ENCODING_CP932,
		"CP932", N_("Japanese") },

	{ GSQL_ENCODING_EUC_KR,
		"EUC-KR", N_("Korean") },
	{ GSQL_ENCODING_EUC_TW,
		"EUC-TW", N_("Chinese Traditional") },

	{ GSQL_ENCODING_GB18030,
		"GB18030", N_("Chinese Simplified") },
	{ GSQL_ENCODING_GB2312,
		"GB2312", N_("Chinese Simplified") },
	{ GSQL_ENCODING_GBK,
		"GBK", N_("Chinese Simplified") },
	{ GSQL_ENCODING_GEOSTD8,
		"GEORGIAN-ACADEMY", N_("Georgian") }, /* FIXME GEOSTD8 ? */
	{ GSQL_ENCODING_HZ,
		"HZ", N_("Chinese Simplified") },

	{ GSQL_ENCODING_IBM_850,
		"IBM850", N_("Western") },
	{ GSQL_ENCODING_IBM_852,
		"IBM852", N_("Central European") },
	{ GSQL_ENCODING_IBM_855,
		"IBM855", N_("Cyrillic") },
	{ GSQL_ENCODING_IBM_857,
		"IBM857", N_("Turkish") },
	{ GSQL_ENCODING_IBM_862,
		"IBM862", N_("Hebrew") },
	{ GSQL_ENCODING_IBM_864,
		"IBM864", N_("Arabic") },

	{ GSQL_ENCODING_ISO_2022_JP,
		"ISO-2022-JP", N_("Japanese") },
	{ GSQL_ENCODING_ISO_2022_KR,
		"ISO-2022-KR", N_("Korean") },
	{ GSQL_ENCODING_ISO_IR_111,
		"ISO-IR-111", N_("Cyrillic") },
	{ GSQL_ENCODING_JOHAB,
		"JOHAB", N_("Korean") },
	{ GSQL_ENCODING_KOI8_R,
		"KOI8R", N_("Cyrillic") },
	{ GSQL_ENCODING_KOI8__R,
		"KOI8-R", N_("Cyrillic") },
	{ GSQL_ENCODING_KOI8_U,
		"KOI8U", N_("Cyrillic/Ukrainian") },
  
	{ GSQL_ENCODING_SHIFT_JIS,
		"SHIFT_JIS", N_("Japanese") },
	{ GSQL_ENCODING_TCVN,
		"TCVN", N_("Vietnamese") },
	{ GSQL_ENCODING_TIS_620,
		"TIS-620", N_("Thai") },
	{ GSQL_ENCODING_UHC,
		"UHC", N_("Korean") },
	{ GSQL_ENCODING_VISCII,
		"VISCII", N_("Vietnamese") },

	{ GSQL_ENCODING_WINDOWS_1250,
		"WINDOWS-1250", N_("Central European") },
	{ GSQL_ENCODING_WINDOWS_1251,
		"WINDOWS-1251", N_("Cyrillic") },
	{ GSQL_ENCODING_WINDOWS_1252,
		"WINDOWS-1252", N_("Western") },
	{ GSQL_ENCODING_WINDOWS_1253,
		"WINDOWS-1253", N_("Greek") },
	{ GSQL_ENCODING_WINDOWS_1254,
		"WINDOWS-1254", N_("Turkish") },
	{ GSQL_ENCODING_WINDOWS_1255,
		"WINDOWS-1255", N_("Hebrew") },
	{ GSQL_ENCODING_WINDOWS_1256,
		"WINDOWS-1256", N_("Arabic") },
	{ GSQL_ENCODING_WINDOWS_1257,
		"WINDOWS-1257", N_("Baltic") },
	{ GSQL_ENCODING_WINDOWS_1258,
		"WINDOWS-1258", N_("Vietnamese") }
};


GtkWidget*
gsql_utils_header_new (GtkWidget * icon, gchar *text, gchar *tooltip,
						 gboolean close_button, gint direction)
{
	GSQL_TRACE_FUNC;

	GtkWidget *box;
	GtkWidget *image;
	GtkWidget *label;
	GtkWidget *button;
	int h, w;
	GtkRcStyle *rcstyle;
	
	label = gtk_label_new (text);
	
	gtk_widget_set_tooltip_markup (GTK_WIDGET (label), tooltip);
	
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);	 


	if (!direction)
	{
		box = gtk_hbox_new(FALSE,0);
		gtk_misc_set_padding (GTK_MISC (label), 2, 0);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	}
	else
	{
		box = gtk_vbox_new(FALSE, 0);
		gtk_label_set_angle (GTK_LABEL (label), 270);
		gtk_misc_set_padding (GTK_MISC (label), 0, 2);
		gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0);
	}

	if (icon)
	{
		gtk_box_pack_start (GTK_BOX (box), icon, FALSE, FALSE,0);	
		gtk_misc_set_alignment (GTK_MISC (icon), 0.5, 0.5);
	}

	gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE,0);
		
	if (close_button)
	{
		gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);

		button = gtk_button_new();
		gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);		
		GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);	
		
		rcstyle = gtk_rc_style_new ();
		rcstyle->xthickness = rcstyle->ythickness = 0;
		gtk_widget_modify_style (button, rcstyle);
		gtk_rc_style_unref (rcstyle);
		
		gtk_widget_set_size_request (button, w+2, h+2);
		
		image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
		gtk_container_add(GTK_CONTAINER (button), image);
		
		gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE,0);

		g_object_set_data_full (G_OBJECT (box), "close_button", 
		    					gtk_widget_ref (button),
		    					(GDestroyNotify) gtk_widget_unref);
		    		
	}

	g_object_set_data_full (G_OBJECT (box), "label", 
		    					gtk_widget_ref (label),
		    					(GDestroyNotify) gtk_widget_unref);

	gtk_widget_show_all (box);
	
	return box; 
}

void
gsql_tree_view_remove_column (gpointer column, gpointer treeview)
{
	gtk_tree_view_remove_column (GTK_TREE_VIEW (treeview), 
								 GTK_TREE_VIEW_COLUMN (column));
}


gchar *
gsql_source_buffer_get_delim_block (GtkTextIter * iter, 
									GtkTextIter ** block_start,
									GtkTextIter ** block_end)
{
	GSQL_TRACE_FUNC;

	gchar * str, *tmp;
	gunichar uch;
	gchar * sql;
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	GtkTextIter *search_iter;
	GtkTextBuffer * buffer;
	gboolean found = FALSE, found_real_start = FALSE;
	GtkTextIter *sql_iter_start;
	GtkTextIter *sql_iter_end;
	
	//gtk_text_iter_free()

	if (iter == NULL)
		return NULL;
		
	buffer = gtk_text_iter_get_buffer (iter);		
	search_iter = gtk_text_iter_copy (iter);
	
    // the end delimiter searching
	do
	{
		if (gtk_text_iter_get_char (search_iter) == '/')
		{
			GSQL_DEBUG ("searching the END: at / cursor placed. Move backward.");
			gtk_text_iter_backward_char (search_iter);
		}
		
		if (!gtk_text_iter_forward_search (search_iter,"\n/", 
											0,
											&start_iter, &end_iter,
											NULL))
		{
			GSQL_DEBUG ("Searching the end: the end block is the end of buffer");
			
			gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &end_iter);
			sql_iter_end = gtk_text_iter_copy (&end_iter);
			
			*block_end = gtk_text_iter_copy (&end_iter);
			found = TRUE;
		}
		else 
		{
			GSQL_DEBUG ("Searching the end: found '\\n/' sequence");
			
			sql_iter_end = gtk_text_iter_copy (&start_iter);
			*block_end = gtk_text_iter_copy (&end_iter);
			
			gtk_text_iter_forward_char (*block_end);
			found = TRUE;
		}
	}
	while (!found); // have found the end delimiter

	found = FALSE;
	
	gtk_text_iter_free(search_iter);
	search_iter = gtk_text_iter_copy (iter);
	
	// the start delimiter searching
	do
	{
		if (gtk_text_iter_get_char (search_iter) == '\n')
		{
			GSQL_DEBUG ("Searching the start: at \\n cursor placed. Move forward.");
			gtk_text_iter_forward_char (search_iter);
		}
		
		if (!gtk_text_iter_backward_search (search_iter,"/\n", 
											0,
											&start_iter, &end_iter,
											NULL))
		{
			GSQL_DEBUG ("Searching the start: the start block is the start of buffer");
			
			gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &start_iter);
			sql_iter_start = gtk_text_iter_copy (&start_iter);
			
			*block_start = gtk_text_iter_copy (&start_iter);
			found = TRUE;
		}
		else
		{
			GSQL_DEBUG ("Searching the start: found '/\\n' sequence");
			gtk_text_iter_free(search_iter);

			// we need skeep spaces and '\n'. other words...
			// get real start iter.
			while (!found_real_start)
			{
				uch = gtk_text_iter_get_char (&end_iter);
				GSQL_DEBUG ("Searching the start: the real start searching. uch = %c", uch);
				
				switch (uch)
				{
					case 0x0a: // \n
					case 0x20: // space
						gtk_text_iter_forward_char (&end_iter);
						GSQL_DEBUG ("Searching the start: the real start searching. move forward");
						break;
					
					default:
						GSQL_DEBUG ("Searching the start: the real start searching. HAVE FOUND");
						found_real_start = TRUE;
				}
			}
			
			sql_iter_start = gtk_text_iter_copy (&end_iter);
			
			*block_start = gtk_text_iter_copy (&end_iter);
			found = TRUE;
		}

	} while (!found); // have found the start delimiter	
	
	str = gtk_text_iter_get_text (sql_iter_start, sql_iter_end);
	tmp = g_strchug (str);
	tmp = g_strchomp (tmp);
	
	if (tmp == NULL)
		return NULL;
		
	sql = g_strdup (tmp);
	g_free (str);
	
	gtk_text_iter_free(sql_iter_start);
	gtk_text_iter_free(sql_iter_end);
	
	return sql;
}


GtkWidget *
gsql_enconding_list (void)
{

	GtkWidget *combo = NULL;
	gint i = 0;
	GtkListStore *model;
	GtkCellRenderer *cell;
	GtkTreeIter encoding_iter;
	
	combo = gtk_combo_box_new ();
	cell = gtk_cell_renderer_text_new ();
    
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo),
                                    cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
                                        cell, "markup", 0, NULL);
	
	model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (model));
	
	while (i < GSQL_ENCODING_LAST)
	{
		gtk_list_store_append(GTK_LIST_STORE(model), &encoding_iter);
		
		gtk_list_store_set( GTK_LIST_STORE(model), &encoding_iter,
							0, g_strdup_printf("%s (%s)",encodings[i].name,encodings[i].charset),
							1, encodings[i].charset,
							-1);
		i++;
	}
	
	gtk_widget_show (combo);
	
	return combo;
	
}


gchar *
gsql_utils_escape_string (gchar *message)
{
	GString *str;
	gchar *ch;

	str = g_string_new ("");
	ch = message;
	
	while (*ch != '\0')
	{
		switch (*ch)
		{
			case '<':
				str = g_string_append (str, "&lt;");
				break;
				
			case '>':
				str = g_string_append (str, "&gt;");
				break;
				
			case '&':
				str = g_string_append (str, "&amp;");
				break;
				
			default:
				str = g_string_append_c (str, *ch);
		}
		
		ch++;
	}
	return g_string_free (str, FALSE);
}

GType
gsql_type_unsupported_get_type (void)
{
	static GType unsupported_type = 0;
	
	if (!unsupported_type)
	{
		unsupported_type = g_type_register_static_simple (G_TYPE_STRING, 
												   "GSQLTypeUnsupported",
												   0, NULL, 0, NULL, 0);
		
	}
		
	return unsupported_type;
	
}
