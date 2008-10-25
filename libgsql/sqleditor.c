/***************************************************************************
 *            sqleditor.c
 *
 *  Copyright  2008  Taras Halturin
 *  <halturin@gmail.com>
 ****************************************************************************/

/*
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


#include <gtksourceview/gtksourceview.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade-xml.h>
#include "sqleditor.h"
#include "editor.h"
#include "common.h"
#include "utils.h"
#include "stock.h"
#include "type_datetime.h"
#include "cvariable.h"
#include <string.h>


static GList *sqleditor_ui_addons = NULL;
static GList *sqleditor_action_addons = NULL;
static GList *sqleditor_f_action_addons = NULL;

static gchar sqleditor_ui[] =
"<ui>  "
"  <toolbar name=\"SQLEditorToolbarRun\">  "
"  	<toolitem name=\"SQLEditorRun\" action=\"ActionSQLEditorRun\"/>  "
"  	<toolitem name=\"SQLEditorRunStep\" action=\"ActionSQLEditorRunStep\"/>  "
"  	<toolitem name=\"SQLEditorStop\" action=\"ActionSQLEditorStop\"/>  "
"	   		<separator name=\"SeparatorA\" />	"
"  	<toolitem name=\"SQLEditorRunAtCursorToggle\" action=\"ActionSQLEditorRunAtCursorToggle\"/>  "
"  	<toolitem name=\"SQLEditorStopOnErrToggle\" action=\"ActionSQLEditorStopOnErrToggle\"/>  "
"	   		<separator name=\"SeparatorA1\" />	"
"  	<placeholder name=\"PHolderSQLEditorRun\" />  "
"  </toolbar>  "
  
"  <toolbar name=\"SQLEditorToolbarFetch\">  "
"  	<toolitem name=\"SQLEditorFetch\" action=\"ActionSQLEditorFetch\"/>  "
"  	<toolitem name=\"SQLEditorFetchAll\" action=\"ActionSQLEditorFetchAll\"/>  "
"  	<toolitem name=\"SQLEditorFetchStop\" action=\"ActionSQLEditorFetchStop\"/>  "
"	   		<separator name=\"SeparatorB\" />	"
"	<placeholder name=\"PHolderSQLEditorFetch\" />  "
//"  	<toolitem name=\"SQLEditorFetchExpand\" action=\"ActionSQLEditorFetchExpand\"/>  "
"  </toolbar>  "
"</ui>  ";


static void on_sql_run (GtkToolButton *button, gpointer data);
static void on_sql_run_step (GtkToolButton *button, gpointer data);
static void on_sql_stop (GtkToolButton *button, gpointer data);
static void on_set_runatcursor (GtkToggleToolButton *button, gpointer data);
static void on_set_stoponerror (GtkToggleToolButton *button, gpointer data);

static void on_custom_limit_checkbutton_toggled (GtkToggleButton *togglebutton,
										gpointer user_data);
static void on_sql_fetch (GtkToolButton *button, gpointer data);
static void on_sql_fetch_all (GtkToolButton *button, gpointer data);
static void on_sql_fetch_stop (GtkToolButton *button, gpointer data);

static void on_editor_set_parent (GtkWidget *widget, GtkObject *object,
								  gpointer user_data);

static void on_buffer_changed (GtkWidget *widget, gpointer user_data);

static void  gsql_editor_get_property	(GObject		*object,
							 guint			propid,
							 GValue	*value,
							 GParamSpec		*pspec);
static void gsql_editor_set_property	(GObject		*object,
							 guint			propid,
							 const GValue	*value,
							 GParamSpec		*pspec);
static void on_sqleditor_fetch_limit_set (gpointer data);


static GtkActionEntry sqleditor_acts[] = 
{

	{ "ActionSQLEditorRun", GSQL_STOCK_SQL_RUN, N_("Run"), "F9", N_("Run"), NULL},
	{ "ActionSQLEditorRunStep", GSQL_STOCK_SQL_RUN_STEP, N_("Run Step"), "F8", N_("Run Step"), NULL},
	{ "ActionSQLEditorStop", GTK_STOCK_STOP, N_("Stop Execution"), NULL, N_("Stop Execution"), NULL },
	
	{ "ActionSQLEditorFetch", GSQL_STOCK_SQL_FETCH_NEXT, N_("Fetch next"), NULL, N_("Fetch next"), NULL },
	{ "ActionSQLEditorFetchAll", GSQL_STOCK_SQL_FETCH_ALL, N_("Fetch all"), NULL, N_("Fetch all"), NULL },
	{ "ActionSQLEditorFetchStop", GTK_STOCK_STOP, N_("Stop fetching"), NULL, N_("Stop fetching"), NULL },
	
//	{ "ActionSQLEditorFetchExpand", GSQL_STOCK_SQL_SHOW_HIDE, N_("Expand Result"), NULL, N_("Expand Result"), NULL }
};

static GtkToggleActionEntry sqleditor_toggle_acts[] = 
{
	{ "ActionSQLEditorRunAtCursorToggle", GSQL_STOCK_SQL_RUN_AT_CURSOR, N_("Run at cursor"), NULL, N_("Run at cursor"), NULL, FALSE },
	{ "ActionSQLEditorStopOnErrToggle", GSQL_STOCK_SQL_STOP_ON_ERR, N_("Stop on error"), NULL, N_("Stop on error"), NULL, TRUE }
	
};

struct _GSQLEditorPrivate
{
	GSQLSession *session;
	
	GtkUIManager   *ui;
	
	GtkWidget *source;
	gulong change_handler_id;
	
	GtkTreeView *result_treeview;
	GtkVBox		*result_vbox;
	
	gboolean run_at_cursor;
	gboolean stop_on_error;
	gboolean stepping;
	
	GtkWidget	 *fetch_limit;
	GtkWidget    *checkb_limit;
	gboolean	 stop_fetch;
	guint		 fetch_max;
	guint		 fetch_step;
	
	gboolean	is_file;
	gchar	   *encoding;
};

static void gsql_editor_class_init (GSQLEditorClass *klass);
static void gsql_editor_init (GSQLEditor *obj);
static void gsql_editor_finalize (GObject *obj);
static void gsql_editor_destroy (GtkObject *obj);



static GtkVBoxClass *parent_class;

enum {
	SIG_TEST = 0,
	SIG_LAST
};

enum {
	PROP_0,
	PROP_IS_FILE,
	PROP_ENCODING
};

static guint editor_signals[SIG_LAST] = { 0 };


GType
gsql_editor_get_type ()
{
	static GType obj_type = 0;
	
	if (!obj_type)
	{
		static const GTypeInfo obj_info = 
		{
			sizeof (GSQLEditorClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gsql_editor_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GSQLEditor),
			0,
			(GInstanceInitFunc) gsql_editor_init,
			NULL
		};
		obj_type = g_type_register_static (GTK_TYPE_VPANED,
										   "GSQLEditor", &obj_info, 0);
		
	}
	
	return obj_type;	
}


GSQLEditor *
gsql_editor_new (GSQLSession *session, GtkWidget *source)
{
	GSQL_TRACE_FUNC;

	GSQLEditor *editor;
	GtkUIManager   *ui = NULL;
	GtkToolbar	   *tbar;
	GtkActionGroup *actions = NULL;
	GtkActionGroup *actions_add = NULL;
	GtkAccelGroup  *accel = NULL;
	GError			*error;
	guint n, ret = 0;
	GtkWidget *sql_vbox;
	GtkWidget *sql_result_vbox;
	GtkWidget *sql_toolbar;
	GtkWidget *sql_scroll;
	GtkWidget *result_toolbar;
	GtkWidget *result_scroll;
	GtkWidget *sql_result_treeview;
	GtkWidget *button;
	GtkWidget *toolitem;
	GtkWidget *custom_limit_spin;
	GtkWidget *custom_limit_checkbutton;
	GSQLEditorFActionCB f_action;
	guint limit_step, limit_max;
	
	if (!GSQL_IS_SESSION (session))
	{
		session = gsql_session_get_active ();
		
		if (!session)
		{
			GSQL_DEBUG ("Have no active session");
			return NULL;
		}
	}
	
	
	if (source == NULL)
		source = gsql_source_editor_new (NULL);
	
	editor = g_object_new (GSQL_EDITOR_TYPE, NULL);	
	
	ui = gtk_ui_manager_new ();
	editor->private->ui = ui;
	gtk_ui_manager_add_ui_from_string (ui, sqleditor_ui, -1, &error);
	
	n = g_list_length (sqleditor_ui_addons);
	
	for (; n>0; n--)
	{
		GSQL_DEBUG ("import ui addons: [%d]", n);
		ret = gtk_ui_manager_add_ui_from_string (ui, 
					(const gchar *)  g_list_nth_data (sqleditor_ui_addons, n-1), 
										   -1, &error);
	};
	
	actions = gtk_action_group_new ("sql_editor_actions");
		
	gtk_action_group_add_actions (actions, sqleditor_acts, 
								  G_N_ELEMENTS (sqleditor_acts), NULL);
	gtk_action_group_add_toggle_actions (actions, sqleditor_toggle_acts,
										 G_N_ELEMENTS (sqleditor_toggle_acts), NULL);
	
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	
	n = g_list_length (sqleditor_action_addons);
	
	for (; n>0; n--)
	{
		
		actions_add = (GtkActionGroup *) g_list_nth_data (sqleditor_action_addons, n-1);
		gtk_ui_manager_insert_action_group (ui, actions_add, 1);
	}
	
	n = g_list_length (sqleditor_f_action_addons);
	
	for (; n>0; n--)
	{
		f_action = g_list_nth_data (sqleditor_f_action_addons, n-1);
		actions_add = f_action();
		gtk_ui_manager_insert_action_group (ui, actions_add, 1);
	}
	
	gtk_ui_manager_ensure_update (ui);
	
	accel = gtk_ui_manager_get_accel_group (ui);
	gtk_window_add_accel_group (GTK_WINDOW(gsql_window), accel);
	
	sql_toolbar = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarRun");
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (sql_toolbar), 
							   GTK_ICON_SIZE_MENU);
	gtk_widget_show (sql_toolbar);
	
	sql_vbox = gtk_vbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (sql_vbox), sql_toolbar, FALSE, FALSE, 0);
	gtk_toolbar_set_style (GTK_TOOLBAR (sql_toolbar), GTK_TOOLBAR_ICONS);
	
	sql_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sql_scroll),
									GTK_POLICY_AUTOMATIC, 
									GTK_POLICY_AUTOMATIC);	
	gtk_widget_show (sql_scroll);
	gtk_box_pack_start (GTK_BOX (sql_vbox), sql_scroll, TRUE, TRUE, 0);
	gtk_paned_pack1 (GTK_PANED (editor), sql_vbox, TRUE, TRUE);
	
	gtk_container_add (GTK_CONTAINER (sql_scroll), 
					   source);
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarRun/SQLEditorRun");
	g_signal_connect (button, "clicked", G_CALLBACK (on_sql_run), editor);
	
	gtk_widget_add_accelerator (button, "clicked", accel,
								GDK_F9, (GdkModifierType) 0,
								GTK_ACCEL_VISIBLE);
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarRun/SQLEditorRunStep");
	
	gtk_widget_add_accelerator (button, "clicked", accel,
								GDK_F8, (GdkModifierType) 0,
								GTK_ACCEL_VISIBLE);
	g_signal_connect (button, "clicked", G_CALLBACK (on_sql_run_step), editor);
	
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarRun/SQLEditorStop");
	g_signal_connect (button, "clicked", G_CALLBACK (on_sql_stop), editor);
	gtk_widget_set_sensitive (button, FALSE);
	
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarRun/SQLEditorRunAtCursorToggle");
	g_signal_connect (button, "clicked", G_CALLBACK (on_set_runatcursor), editor);
	
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarRun/SQLEditorStopOnErrToggle");
	g_signal_connect (button, "clicked", G_CALLBACK (on_set_stoponerror), editor);
	editor->private->stop_on_error = TRUE;
		
	sql_result_vbox = gtk_vbox_new (FALSE, 0);
	result_toolbar = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarFetch");
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (result_toolbar), 
							   GTK_ICON_SIZE_MENU);
	gtk_box_pack_start (GTK_BOX (sql_result_vbox), result_toolbar, 
						FALSE, FALSE, 0);
	gtk_toolbar_set_style (GTK_TOOLBAR (result_toolbar), 
						   GTK_TOOLBAR_ICONS);
	
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarFetch/SQLEditorFetch");
	g_signal_connect (button, "clicked", G_CALLBACK (on_sql_fetch), editor);
	
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarFetch/SQLEditorFetchAll");
	g_signal_connect (button, "clicked", G_CALLBACK (on_sql_fetch_all), editor);
	
	button = gtk_ui_manager_get_widget (ui, "/SQLEditorToolbarFetch/SQLEditorFetchStop");
	g_signal_connect (button, "clicked", G_CALLBACK (on_sql_fetch_stop), editor);
	gtk_widget_set_sensitive (button, TRUE);
	
	toolitem = (GtkWidget *) gtk_tool_item_new ();
	gtk_widget_show (toolitem);
	gtk_toolbar_insert (GTK_TOOLBAR (result_toolbar), GTK_TOOL_ITEM (toolitem), 0);
	custom_limit_checkbutton = gtk_check_button_new ();
	gtk_widget_show (custom_limit_checkbutton);
	GTK_WIDGET_UNSET_FLAGS (custom_limit_checkbutton, GTK_CAN_FOCUS);
	gtk_container_add (GTK_CONTAINER (toolitem),
						custom_limit_checkbutton);
	gtk_widget_set_tooltip_text (custom_limit_checkbutton,
							 N_("Use custom fetch limit"));
	editor->private->checkb_limit = custom_limit_checkbutton;
	
	toolitem = (GtkWidget *) gtk_tool_item_new ();
	gtk_widget_show (toolitem);
	gtk_toolbar_insert (GTK_TOOLBAR (result_toolbar), GTK_TOOL_ITEM (toolitem), 1);
	custom_limit_spin = gtk_spin_button_new_with_range (SQL_EDITOR_CUSTOM_FETCH_LIMIT, 64000, 50);

	gtk_widget_show (custom_limit_spin);
	gtk_widget_set_sensitive (custom_limit_spin, FALSE);
	gtk_container_add (GTK_CONTAINER (toolitem),
						custom_limit_spin);
	editor->private->fetch_limit = custom_limit_spin;
	
	gtk_widget_set_tooltip_text (custom_limit_spin,
							 N_("Set custom limit value"));
	g_signal_connect ((gpointer) custom_limit_checkbutton, "toggled",
					G_CALLBACK (on_custom_limit_checkbutton_toggled),
					(gpointer) custom_limit_spin);
	
	limit_step = gsql_conf_value_get_int (GSQL_CONF_SQL_FETCH_STEP);
	
	if (!limit_step)
		limit_step = GSQL_EDITOR_FETCH_STEP_DEFAULT;
	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (custom_limit_spin),
								   limit_step);
	
	limit_max = gsql_conf_value_get_int (GSQL_CONF_SQL_FETCH_MAX);
	
	if (!limit_max)
		limit_max = GSQL_EDITOR_FETCH_MAX_DEFAULT;
	
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (custom_limit_spin),
							   GSQL_EDITOR_FETCH_STEP_DEFAULT,
							   GSQL_EDITOR_FETCH_MAX_DEFAULT);
	
	editor->private->fetch_max = limit_max;
	
	gsql_conf_nitify_add (GSQL_CONF_SQL_FETCH_STEP,
								on_sqleditor_fetch_limit_set,
								(gpointer) editor);
	
	gsql_conf_nitify_add (GSQL_CONF_SQL_FETCH_MAX,
								on_sqleditor_fetch_limit_set,
								(gpointer) editor);
	
	result_scroll = gtk_scrolled_window_new (NULL, NULL);        
	gtk_box_pack_start (GTK_BOX (sql_result_vbox), 
						result_scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (result_scroll), 
									GTK_POLICY_AUTOMATIC, 
									GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (result_scroll), 
											GTK_SHADOW_NONE);
        
	sql_result_treeview = gtk_tree_view_new ();        
	gtk_container_add (GTK_CONTAINER (result_scroll), 
					   sql_result_treeview);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (sql_result_treeview), TRUE);
	gtk_widget_set_no_show_all (sql_result_vbox, TRUE);
	gtk_widget_show_all (sql_result_vbox);
	
	gtk_paned_pack2 (GTK_PANED (editor), sql_result_vbox, TRUE, TRUE);
	
	editor->private->result_treeview = GTK_TREE_VIEW (sql_result_treeview);
	editor->private->result_vbox = GTK_VBOX (sql_result_vbox);
	editor->private->source = source;
	
	g_signal_connect (G_OBJECT (editor), "parent-set", 
					  G_CALLBACK (on_editor_set_parent),
					  NULL);
	editor->private->session = session;
	
	return editor;
}

void
gsql_editor_run_sql (GSQLEditor *editor)
{
	on_sql_run (NULL, editor);	
}

void
gsql_editor_merge_actions (gchar *ui_addons, GtkActionGroup *action_addons)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (ui_addons != NULL);
	g_return_if_fail (GTK_IS_ACTION_GROUP (action_addons));
	
	sqleditor_ui_addons = g_list_append (sqleditor_ui_addons, ui_addons);
	sqleditor_action_addons = g_list_append (sqleditor_action_addons, 
											 action_addons);
	
	
}

void
gsql_editor_merge_f_actions (gchar *ui_addons, GSQLEditorFActionCB f_action)
{
	GSQL_TRACE_FUNC;
	
	g_return_if_fail (ui_addons != NULL);
	g_return_if_fail (f_action != NULL);
	
	sqleditor_ui_addons = g_list_append (sqleditor_ui_addons, ui_addons);
	sqleditor_f_action_addons = g_list_append (sqleditor_f_action_addons, 
											 f_action);

}

/* 
 *  Static section:
 *
 *  gsql_editor_class_init
 *  gsql_editor_init
 *  gsql_editor_finalize
 *  gsql_editor_destroy
 *  change_handler
 *  on_run_sql
 *  on_sql_run_step
 *  do_sql_run
 *  on_sql_stop
 *  on_set_runatcursor
 *  on_set_stoponerror
 *  on_custom_limit_checkbutton_toggled
 *  on_editor_cb_close
 *  on_editor_cb_save
 *  on_editor_cb_revert
 *  on_editor_set_parent
 *  on_sqleditor_fetch_limit_set
 */


static void
gsql_editor_class_init (GSQLEditorClass *klass)
{
	GSQL_TRACE_FUNC;

	GObjectClass *obj_class = G_OBJECT_CLASS (klass);
	GtkObjectClass   *gtkobject_class = GTK_OBJECT_CLASS (klass);

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);
	gtkobject_class->destroy = gsql_editor_destroy;
	
	obj_class->finalize = gsql_editor_finalize;
	obj_class->set_property = gsql_editor_set_property;
	obj_class->get_property = gsql_editor_get_property;


	g_object_class_install_property (obj_class,
									 PROP_IS_FILE,
									 g_param_spec_boolean ("is-file",
														  "Set 'is file' flag",
														  "Set 'is file' flag. FIXME (description)",
														  FALSE,
														  G_PARAM_READWRITE));
	g_object_class_install_property (obj_class,
									 PROP_ENCODING,
									 g_param_spec_string ("encoding",
														  "Set 'encoding'",
														  "Set 'encoding'. FIXME (description)",
														  NULL,
														  G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS));

}

static void 
gsql_editor_init (GSQLEditor *obj)
{
	GSQL_TRACE_FUNC;

	g_return_if_fail (obj != NULL);
	obj->private = g_new0 (GSQLEditorPrivate, 1);

	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (obj), GTK_NO_WINDOW);
	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (obj), FALSE);
	
	obj->private->is_file = FALSE;
	obj->private->encoding = NULL;

}


static void
gsql_editor_finalize (GObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLEditor *editor = GSQL_EDITOR (obj);
	g_object_unref (editor->private->ui);
	g_free (editor->private);
	
	(* G_OBJECT_CLASS (parent_class)->finalize) (obj);	
	
}

/*FIXME: bug_DBLDESINV: double destroy invoke.
 
			trace: [0x8056408] on_file_close_activate [gsqlmenucb.c:231]
			trace: [0x8056408] gsql_workspace_get_current_content [workspace.c:378]
			trace: [0x8056408] gsql_session_get_active [session.c:348]
			trace: [0x8056408] gsql_session_get_workspace [session.c:315]
			** (lt-gsql:30443): DEBUG: Is GSQLContent page
			trace: [0x8056408] on_editor_cb_close [sqleditor.c:1317]
			trace: [0x8056408] gsql_content_get_changed [content.c:279]
			trace: [0x8056408] on_editor_focus_out [editor.c:379]
here===>	trace: [0x8056408] gsql_content_destroy [content.c:451]

			(lt-gsql:30443): Gtk-WARNING **: GtkContainerClass::remove not implemented for `GSQLContent'
			trace: [0x8056408] gsql_editor_destroy [sqleditor.c:496]
			trace: [0x8056408] gsql_cursor_close [cursor.c:463]
			trace: [0x8056408] gsql_variable_dispose [cvariable.c:99]
			trace: [0x8056408] gsql_variable_get_type [cvariable.c:47]
			trace: [0x8056408] gsql_variable_finalize [cvariable.c:111]
			trace: [0x8056408] gsql_variable_get_type [cvariable.c:47]
			trace: [0x8056408] on_cursor_close [oracle_cursor.c:44]
			trace: [0x8056408] gsql_cursor_get_type [cursor.c:69]
			trace: [0x8056408] gsql_cursor_dispose [cursor.c:610]
			trace: [0x8056408] gsql_cursor_get_type [cursor.c:69]
			trace: [0x8056408] gsql_cursor_finalize [cursor.c:621]
			trace: [0x8056408] gsql_cursor_get_type [cursor.c:69]
here===>	trace: [0x8056408] gsql_content_destroy [content.c:451]

			(lt-gsql:30443): Gtk-WARNING **: GtkContainerClass::remove not implemented for `GSQLContent'
			trace: [0x8056408] gsql_editor_destroy [sqleditor.c:496]
			trace: [0x8056408] gsql_content_finalize [content.c:429]
			trace: [0x8056408] gsql_session_release_title [session.c:407]
			trace: [0x8056408] untitled_hash_remove_key_notify [session.c:958]
			** (lt-gsql:30443): DEBUG: Untitled hash: entry found. removed.

 
 
 
 */

static void
gsql_editor_destroy (GtkObject *obj)
{
	GSQL_TRACE_FUNC;

	GSQLEditor *editor = GSQL_EDITOR (obj);
	
	if (editor->cursor)
	{
		gsql_cursor_close (editor->cursor);
		
		/* remove line below after fix bug_DBLDESINV */
		editor->cursor = NULL;
	}
	
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
	
}

static void 
gsql_editor_get_property	(GObject		*object,
							 guint			propid,
							 GValue	*value,
							 GParamSpec		*pspec)
{
	
	GSQLEditor *editor;
	
	g_return_if_fail (GSQL_IS_EDITOR (object));
	
	editor = GSQL_EDITOR (object);
	
	switch (propid)
	{
		case PROP_IS_FILE:
			g_value_set_boolean (value, editor->private->is_file);
			break;
			
		case PROP_ENCODING:
			g_value_set_string (value, editor->private->encoding);
			break;
	}
	
}

static void 
gsql_editor_set_property	(GObject		*object,
							 guint			propid,
							 const GValue	*value,
							 GParamSpec		*pspec)
{
	GSQL_TRACE_FUNC;
	
	GSQLEditor *editor;
	
	g_return_if_fail (GSQL_IS_EDITOR (object));
	
	editor = GSQL_EDITOR (object);
	gboolean state = editor->private->is_file;
	GtkTextBuffer *buffer;
	
	switch (propid)
	{
		case PROP_IS_FILE:
			GSQL_DEBUG ("PROP_IS_FILE");
			editor->private->is_file = g_value_get_boolean (value);
			
			GSQL_DEBUG ("state [%d] new[%d]", state, editor->private->is_file);
			
			if ((!state) && (editor->private->is_file))
			{
				GSQL_DEBUG ("set callback!");
				buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->private->source));
				
				g_signal_connect (G_OBJECT (buffer), "modified-changed",
								  G_CALLBACK (on_buffer_changed), NULL);
			}
			
			break;

		case PROP_ENCODING:
			if (editor->private->encoding)
				g_free (editor->private->encoding);
			editor->private->encoding = g_strdup (g_value_get_string (value));
			
			break;
	}
}


static void
change_handler (GtkTextBuffer *textbuffer, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLEditor *sqleditor = user_data;
	
	g_signal_handler_disconnect (textbuffer, sqleditor->private->change_handler_id);
	sqleditor->private->change_handler_id = 0;
	
	gsql_source_editor_markers_clear (GTK_SOURCE_BUFFER (textbuffer));
	
}

static void
do_sql_run (GSQLEditor *sqleditor)
{
	GSQL_TRACE_FUNC;
	
	GSQLCursor *cursor = NULL;
	GSQLSession *session;
	GSQLWorkspace *workspace = NULL;
	GtkSourceView *source;
	GtkVBox *result_vbox;
	GtkTextBuffer *buffer;
	GtkTextIter start_iter, end_iter, *search_iter, *s_iter, *e_iter;
	GtkWidget *run_b, *run_step_b, *stop_b, *runatcursor_b, *stoponerror_b,
				*fetch_b, *fetch_all_b, *toolbar_fetch;
	GList *list, *columns, *vlist;
	gint i, l;
	gchar *sql = NULL, *tmp, msg[128], tmp_value[1024];
	GTimer *timer;
	gulong  microsec;
	
	session = sqleditor->private->session;
	workspace = gsql_session_get_workspace (session);
	
	source = GTK_SOURCE_VIEW (sqleditor->private->source);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source));
	
	run_b = gtk_ui_manager_get_widget (sqleditor->private->ui,
									   "/SQLEditorToolbarRun/SQLEditorRun");
	run_step_b = gtk_ui_manager_get_widget (sqleditor->private->ui,
											"/SQLEditorToolbarRun/SQLEditorRunStep");
	stop_b = gtk_ui_manager_get_widget (sqleditor->private->ui,
										"/SQLEditorToolbarRun/SQLEditorStop");
	runatcursor_b = gtk_ui_manager_get_widget (sqleditor->private->ui,
											   "/SQLEditorToolbarRun/SQLEditorRunAtCursorToggle");
	stoponerror_b = gtk_ui_manager_get_widget (sqleditor->private->ui,
											   "/SQLEditorToolbarRun/SQLEditorStopOnErrToggle");
	result_vbox = sqleditor->private->result_vbox;
	
	fetch_b = gtk_ui_manager_get_widget (sqleditor->private->ui,
									   "/SQLEditorToolbarFetch/SQLEditorFetch");
	fetch_all_b = gtk_ui_manager_get_widget (sqleditor->private->ui,
										"/SQLEditorToolbarFetch/SQLEditorFetchAll");
	toolbar_fetch = gtk_ui_manager_get_widget (sqleditor->private->ui,
										"/SQLEditorToolbarFetch");
	
	GSQL_THREAD_ENTER;
	
	if (!session->engine->multi_statement)
	{		
		gtk_widget_set_sensitive (fetch_b, FALSE);
		gtk_widget_set_sensitive (fetch_all_b, FALSE);
		
	} else {
		
		gtk_widget_set_sensitive (fetch_b, TRUE);
		gtk_widget_set_sensitive (fetch_all_b, TRUE);
		
	}

	if (!sqleditor->private->run_at_cursor)
	{
		
		if (sqleditor->private->stepping)
		{
			sqleditor->private->run_at_cursor = TRUE;
			gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (runatcursor_b), TRUE);
			GSQL_DEBUG ("No! Run at cursor ");
			gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
										  &start_iter,
										  gtk_text_buffer_get_insert (GTK_TEXT_BUFFER (buffer)));
		} else {
			GSQL_DEBUG ("Run at start iter");
		gtk_text_buffer_get_start_iter  (GTK_TEXT_BUFFER (buffer), &start_iter);
		gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (buffer), &start_iter);
		gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (source),
										  &start_iter,
										  0, TRUE, 0,0);
		}
	} else {
		
		GSQL_DEBUG ("Run at cursor ");
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer),
										  &start_iter,
										  gtk_text_buffer_get_insert (GTK_TEXT_BUFFER (buffer)));
	}
	
	gtk_widget_set_sensitive (toolbar_fetch, FALSE);
	gtk_widget_set_sensitive (run_step_b, FALSE);
	gtk_widget_set_sensitive (run_b, FALSE);
	gtk_widget_set_sensitive (stop_b, TRUE);
	
	
	gtk_text_view_set_editable (GTK_TEXT_VIEW (source), FALSE);
	gsql_source_editor_markers_clear (buffer);
	GSQL_THREAD_LEAVE;
	
	search_iter = gtk_text_iter_copy (&start_iter);
	
	do
	{
		if (sql != NULL)
			g_free (sql);

		if (gtk_text_buffer_get_selection_bounds(buffer,
												 &start_iter,
												 &end_iter))
		{
			// run selected text
			tmp = gtk_text_iter_get_text (&start_iter, &end_iter);
			sql = g_strchug (tmp);
			sql = g_strchomp (sql);		
			s_iter = gtk_text_iter_copy (&start_iter);
			e_iter = gtk_text_iter_copy (&end_iter);
			
			GSQL_DEBUG ("Run selected SQL:[%s]", sql);

		} else {
		
			sql = gsql_source_buffer_get_delim_block (search_iter, &s_iter, &e_iter);
			GSQL_DEBUG ("SQL block running: %s", sql);
		};
		gtk_text_iter_free(search_iter);
		
		if (g_utf8_strlen (sql, -1) == 0)
		{
			sql = NULL;
			continue;
		}
		
		GSQL_THREAD_ENTER;
		
		if (sqleditor->private->stepping)
		{
			gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (buffer), e_iter);
			gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (source),
										  s_iter,
										  0, TRUE, 0,0);
		} else {
			gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (source),
										  s_iter,
										  0, TRUE, 0,0);
		}
		
		GSQL_THREAD_LEAVE;
		
		if (sqleditor->cursor)
		{
			gsql_cursor_close (sqleditor->cursor);
			sqleditor->cursor = NULL;
		}
		
		sqleditor->cursor = cursor = gsql_cursor_new (session, sql);
		
		if (cursor)
		{
			gsql_message_add (workspace, GSQL_MESSAGE_NORMAL, N_("Start SQL execution..."));
			gsql_cursor_notify_set (cursor, TRUE);
			timer = g_timer_new();
			gsql_cursor_open (cursor, FALSE);
			g_timer_stop (timer);
		}
		
		if ((gsql_cursor_get_state (cursor) == GSQL_CURSOR_STATE_OPEN) &&
			(cursor->stmt_type == GSQL_CURSOR_STMT_SELECT))
		{
			l = gtk_text_iter_get_line (s_iter);
			
			if (l == 0)	
				l++;
			tmp = g_strdup_printf (N_("Query execution finished at line %d [elapsed: %02f]"), 
								   l, g_timer_elapsed (timer, &microsec));
			
			gsql_message_add (workspace, GSQL_MESSAGE_NORMAL, tmp);
			
			GSQL_THREAD_ENTER;
			gsql_source_editor_marker_set (s_iter, GSQL_EDITOR_MARKER_COMPLETE);
			GSQL_THREAD_LEAVE;
			
			g_free (tmp); tmp = NULL;
			
			GSQL_DEBUG ("Cursor opened");
			/* i think it's bad idea to execute next query if the statement are select.
				set stepping flag
			*/
			sqleditor->private->stepping = TRUE;
			cursor->stmt_affected_rows = 0;
			
			//=========================================================
			on_sql_fetch (NULL, sqleditor);
			
			if (!session->engine->multi_statement)
			{
				gsql_cursor_close (cursor);
				sqleditor->cursor = NULL;
			}			
			
		} else {
			
			GSQL_THREAD_ENTER;
			gtk_widget_hide_all (GTK_WIDGET (result_vbox));
			gtk_widget_set_no_show_all (GTK_WIDGET (result_vbox), TRUE);
			
			if (gsql_cursor_get_state (cursor) == GSQL_CURSOR_STATE_ERROR)
			{ // mark as "error"
				GSQL_DEBUG ("Marking iters block as \"error\"");
				gsql_source_editor_marker_set (s_iter, GSQL_EDITOR_MARKER_FAILED);
				
				if (sqleditor->private->stop_on_error)
					sqleditor->private->stepping = TRUE;
				
				gsql_cursor_close (cursor);
				sqleditor->cursor = NULL;
				
				GSQL_THREAD_LEAVE;
				
			} else { 
				// mark as "complete"
				GSQL_DEBUG ("Marking iters block as \"complete\"");
				gsql_source_editor_marker_set (s_iter, GSQL_EDITOR_MARKER_COMPLETE);
				
				memset (msg, 0, 128);
				i = GSQL_MESSAGE_ERROR;
				
				l = gtk_text_iter_get_line (s_iter);
				if (l == 0)
					l++;
				
				switch (cursor->stmt_type)
				{
					case GSQL_CURSOR_STMT_DML:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s [%llu] %s %d [%s: %02f]",
									N_("Query execution finished [DML]. Affected rows"),
									cursor->stmt_affected_rows,
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
						
					case GSQL_CURSOR_STMT_INSERT:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s [%llu] %s %d [%s: %02f]",
									N_("Query execution finished. Inserted rows"),
									cursor->stmt_affected_rows,
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
						
					case GSQL_CURSOR_STMT_UPDATE:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s [%llu] %s %d [%s: %02f]",
									N_("Query execution finished. Updated rows"),
									cursor->stmt_affected_rows,
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
						
					case GSQL_CURSOR_STMT_DELETE:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s [%llu] %s %d [%s: %02f]",
									N_("Query execution finished. Deleted rows"),
									cursor->stmt_affected_rows,
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
						
					case GSQL_CURSOR_STMT_EXEC:
						i = GSQL_MESSAGE_NORMAL;
						g_snprintf (msg, 128, 
									"%s %s %d [%s: %02f]",
									N_("Query execution finished"),
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
					
					case GSQL_CURSOR_STMT_CREATE:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s %s %d [%s: %02f]",
									N_("Query execution finished. Object created"),
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
					
					case GSQL_CURSOR_STMT_DROP:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s %s %d [%s: %02f]",
									N_("Query execution finished. Object droped"),
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
						
					case GSQL_CURSOR_STMT_ALTER:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s %s %d [%s: %02f]",
									N_("Query execution finished. Object altered"),
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						
					case GSQL_CURSOR_STMT_DDL:
						i = GSQL_MESSAGE_NOTICE;
						g_snprintf (msg, 128, 
									"%s %s %d [%s: %02f]",
									N_("Query execution finished [DDL]"),
									N_("at line"), l+1, 
									N_("elapsed"),
									g_timer_elapsed (timer, &microsec));
						break;
						
					default:
						i = GSQL_MESSAGE_WARNING;
						g_snprintf (msg, 128,"%s",
									N_("Unknown statement type"));
				}
				
				GSQL_THREAD_LEAVE;	
				
				gsql_message_add (workspace, i, msg);

			}
			
		}
		g_timer_destroy (timer);
		
		search_iter = e_iter;
		gtk_text_iter_free(s_iter);
		
		GSQL_THREAD_ENTER
		gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (buffer), e_iter);
		gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (source),
									  e_iter,
									  0, TRUE, 0,0);
		GSQL_THREAD_LEAVE
				
		if (sqleditor->private->stepping)
			break;
		
		
		
	} while ((sql != NULL) && (!gtk_text_iter_is_end (e_iter)));
	
	if (sql != NULL)
		g_free (sql);
	
	if (!sqleditor->private->change_handler_id)
	{
		sqleditor->private->change_handler_id = g_signal_connect (G_OBJECT (buffer), 
																	  "changed", 
																	  G_CALLBACK (change_handler), 
																	  (gpointer) sqleditor);
	}
	
	GSQL_THREAD_ENTER;
	gtk_widget_set_sensitive (toolbar_fetch, TRUE);
	gtk_widget_set_sensitive (run_step_b, TRUE);
	gtk_widget_set_sensitive (run_b, TRUE);
	gtk_widget_set_sensitive (stop_b, FALSE);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (source), TRUE);
	GSQL_THREAD_LEAVE;
	
}

static void
on_sql_run (GtkToolButton *button, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GError *err;
	GThread *thread = NULL;
	GSQLEditor *sqleditor = data;
	
	g_return_if_fail (GSQL_IS_EDITOR (sqleditor));
	
	sqleditor->private->stepping = FALSE;
	thread = g_thread_create ((GThreadFunc) do_sql_run,
							  sqleditor, 
							  FALSE,
							  &err);
	if (!thread)
		g_warning ("Couldn't create thread");	
	
}

static void
on_sql_run_step (GtkToolButton *button, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GError *err;
	GThread *thread = NULL;
	GSQLEditor *sqleditor = data;
	
	g_return_if_fail (GSQL_IS_EDITOR (sqleditor));
	
	sqleditor->private->stepping = TRUE;
	thread = g_thread_create ((GThreadFunc) do_sql_run,
							  sqleditor, 
							  FALSE,
							  &err);
	if (!thread)
		g_warning ("Couldn't create thread");	

}


static void
on_sql_stop (GtkToolButton *button, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GSQLEditor *sqleditor = data;
	GSQLCursor *cursor;
	
	g_return_if_fail (GSQL_IS_EDITOR (sqleditor));
	
	gsql_cursor_stop (sqleditor->cursor);
	
}

static void 
on_set_runatcursor (GtkToggleToolButton *button, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	
	GSQLEditor *sqleditor = data;
	
	g_return_if_fail (GSQL_IS_EDITOR (sqleditor));
	
	sqleditor->private->run_at_cursor = gtk_toggle_tool_button_get_active (button);
	
}

static void on_set_stoponerror (GtkToggleToolButton *button, gpointer data)
{
	GSQL_TRACE_FUNC;
	
	GSQLEditor *sqleditor = data;
	
	g_return_if_fail (GSQL_IS_EDITOR (sqleditor));
	
	sqleditor->private->stop_on_error = gtk_toggle_tool_button_get_active (button);
	
}

static void
on_custom_limit_checkbutton_toggled (GtkToggleButton *togglebutton,
										gpointer user_data)
{
	GSQL_TRACE_FUNC;

	GtkWidget *spin = user_data;
	gboolean status;
	guint limit_step;
        
	status = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (spin, status);
	
	if (!status)
	{
		limit_step = gsql_conf_value_get_int (GSQL_CONF_SQL_FETCH_STEP);
	
		if (!limit_step)
			limit_step = GSQL_EDITOR_FETCH_STEP_DEFAULT;
		
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 
								   limit_step);
	}
}


static void
emit_inserted_stoper (GtkTreeModel *tree_model,
			GtkTreePath  *path,
			GtkTreeIter  *iter,
			gpointer      user_data)
{	
	g_signal_stop_emission_by_name(G_OBJECT(tree_model), "row-inserted");
}

static void
emit_changed_stoper (GtkTreeModel *tree_model,
			GtkTreePath  *path,
			GtkTreeIter  *iter,
			gpointer      user_data)
{	
	g_signal_stop_emission_by_name(G_OBJECT(tree_model), "row-changed");
}

static void
emit_reordered_stoper (GtkTreeModel *tree_model,
			GtkTreePath  *path,
			GtkTreeIter  *iter,
			gpointer      user_data)
{	
	g_signal_stop_emission_by_name(G_OBJECT(tree_model), "rows-reordered");
}


static void
do_sql_fetch (GSQLEditor *editor)
{
	GSQL_TRACE_FUNC;
	
    GSQLCursor *cursor = NULL;
	GSQLWorkspace *workspace = NULL;
	GtkVBox *result_vbox;
	GtkTreeView *result_treeview;
	GtkListStore *liststore, *liststore_new;
	static GtkListStore *ls_plug = NULL;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *column_header;
	GtkWidget *fetch_b, *fetch_all_b, *fetch_stop_b, *toolbar_run;
	GList *list, *columns, *vlist;
	gint i, l, *column_pos, var_count;
	GType *var_types;
	GValue *values;
	gchar msg[128], tmp_value[1024], *tmp;
	GSQLVariable *var;
	gboolean sorting;
	gboolean continue_fetch;
	guint rows_limit = 0, rows_fetched = 0, rows;
	static guint signal_inserted = 0;
	static guint signal_reordered = 0;
	static guint signal_changed = 0;
    
    cursor = editor->cursor;
    g_return_if_fail (gsql_cursor_get_state (cursor) == GSQL_CURSOR_STATE_OPEN);
    g_return_if_fail (cursor->stmt_type == GSQL_CURSOR_STMT_SELECT);
	
	workspace = gsql_session_get_workspace (cursor->session);
	
	if (cursor->stmt_affected_rows >= editor->private->fetch_max)
	{
		g_snprintf (msg, 128, 
					N_("The maximum limit of fetch is reached [%d]. You can extend this limit in the settings."),
					editor->private->fetch_max);

		gsql_message_add (workspace, GSQL_MESSAGE_WARNING,
						  msg);
		return;
	}
	
	fetch_b = gtk_ui_manager_get_widget (editor->private->ui,
									   "/SQLEditorToolbarFetch/SQLEditorFetch");
	fetch_all_b = gtk_ui_manager_get_widget (editor->private->ui,
										"/SQLEditorToolbarFetch/SQLEditorFetchAll");
	fetch_stop_b = gtk_ui_manager_get_widget (editor->private->ui,
										"/SQLEditorToolbarFetch/SQLEditorFetchStop");
	toolbar_run = gtk_ui_manager_get_widget (editor->private->ui,
									   "/SQLEditorToolbarRun");
	
	result_treeview = editor->private->result_treeview;
	result_vbox = editor->private->result_vbox;
	
	gtk_widget_set_sensitive (toolbar_run, FALSE);
	gtk_widget_set_sensitive (fetch_b, FALSE);
	gtk_widget_set_sensitive (fetch_all_b, FALSE);
	gtk_widget_set_sensitive (fetch_stop_b, TRUE);
	
	editor->private->stop_fetch = FALSE;
	
	liststore = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (result_treeview)));
	
	continue_fetch = (cursor->stmt_affected_rows == 0) ? FALSE : TRUE;
	var_count = g_list_length (cursor->var_list);
	
	GSQL_THREAD_ENTER;
    
	if (!continue_fetch)
	{   
		var_count = g_list_length (cursor->var_list);
		if (!var_count)
		{
			GSQL_DEBUG ("Variables list is zero length");
		
			gtk_widget_hide_all (GTK_WIDGET (result_vbox));
			gtk_widget_set_no_show_all (GTK_WIDGET (result_vbox), TRUE);
			gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (result_treeview), FALSE);
			GSQL_THREAD_LEAVE;

			return;
		}
		
		if (liststore)
		{
			gtk_list_store_clear (liststore);
			g_object_unref(liststore);
		}
		
		columns = gtk_tree_view_get_columns (GTK_TREE_VIEW (result_treeview));
		g_list_foreach (columns, (GFunc) gsql_tree_view_remove_column,
							result_treeview);					
		g_list_free(columns);
	}
	
	GSQL_DEBUG ("alloc mem for %d", var_count);
	column_pos = g_new0 (gint, var_count);
	var_types = g_new0 (GType, var_count);
	values = g_new0 (GValue, var_count);
    
	vlist = g_list_first (cursor->var_list);
	
	for (i=0; i<var_count; i++)
	{
		GSQL_DEBUG ("Parsing %d variable", i);
		column_pos[i] = i;
		var = vlist->data;
		sorting = TRUE;
		
		vlist = g_list_next (vlist);
		tmp = "text";
		
		switch (var->value_type)
		{
			case G_TYPE_INT64:
				GSQL_DEBUG ("G_TYPE_INT64");
				
				var_types[i] = G_TYPE_INT64;
				g_value_init (&values[i], G_TYPE_INT64);
				
				if (!continue_fetch)
					renderer = gtk_cell_renderer_text_new();
				
				break;
				
			case G_TYPE_INT:
				GSQL_DEBUG ("G_TYPE_INT");
				
				var_types[i] = G_TYPE_INT;
				g_value_init (&values[i], G_TYPE_INT);
				
				if (!continue_fetch)
					renderer = gtk_cell_renderer_text_new();
				
				break;
				
			case G_TYPE_DOUBLE:
				GSQL_DEBUG ("G_TYPE_DOUBLE");
				
				var_types[i] = G_TYPE_DOUBLE;
				g_value_init (&values[i], G_TYPE_DOUBLE);
				
				if (!continue_fetch)
					renderer = gtk_cell_renderer_text_new();
				
				break;
				
			default:
			
				// GSQL_TYPE_DATETIME - case label does not reduce to an integer constant
				if (var->value_type == GSQL_TYPE_DATETIME)
				{
					sorting = FALSE;
					GSQL_DEBUG ("GSQL_TYPE_DATETIME");
				
					var_types[i] = GSQL_TYPE_DATETIME;
					g_value_init (&values[i], GSQL_TYPE_DATETIME);
				
					if (!continue_fetch)
						renderer = gsql_cell_renderer_datetime_new();
					tmp = "datetime";
				
					break;
				
				}
				
				GSQL_DEBUG ("G_TYPE_STRING");
				
				var_types[i] = G_TYPE_STRING;
				g_value_init (&values[i], G_TYPE_STRING);
				
				if (!continue_fetch)
					renderer = gtk_cell_renderer_text_new();
				
		}
		
		if (!continue_fetch)
		{
			column_header = gtk_label_new (var->field_name);
			gtk_widget_show (column_header);
		
			column = gtk_tree_view_column_new ();
			gtk_tree_view_column_set_widget (column, column_header);
			gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		
			if (sorting)
				gtk_tree_view_column_set_sort_column_id (column, i);
		
			gtk_tree_view_append_column(GTK_TREE_VIEW(result_treeview), column);
			gtk_tree_view_column_set_resizable (column, TRUE);
			gtk_tree_view_column_pack_start (column, renderer, TRUE);
			gtk_tree_view_column_add_attribute (column, renderer, tmp, i);
		}
	}
	
	if (!continue_fetch)
	{
		liststore_new = gtk_list_store_newv (var_count, var_types);
		gtk_widget_set_no_show_all (GTK_WIDGET (result_vbox), FALSE);
		gtk_widget_show_all (GTK_WIDGET (result_vbox));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (result_treeview), TRUE);
	
	} else {
	
		liststore_new = liststore;
	}
	
	rows_limit = editor->private->fetch_step;
	
	GSQL_DEBUG ("rows_limit: %d", rows_limit);
	
	GSQL_THREAD_LEAVE;
	rows_fetched = 0;
	editor->private->stop_fetch = FALSE;
	
	GSQL_FIXME;
	/* 	perfomance problem.
		A treeview widget tries to redraw on every insert operation.
		Any body knows how to avoid this trouble?
		hack use.
	 */
	if (!continue_fetch)
	{
		g_signal_connect (liststore_new, "row-inserted", 
						  G_CALLBACK (emit_inserted_stoper), NULL);
		g_signal_connect (liststore_new, "row-changed", 
						  G_CALLBACK (emit_changed_stoper), NULL);
		g_signal_connect (liststore_new, "rows-reordered", 
						  G_CALLBACK (emit_reordered_stoper), NULL);
	}
	
	while ( (rows_fetched < rows_limit) &&
		   (rows = gsql_cursor_fetch (cursor, 1) > 0))
	{
		GSQL_THREAD_ENTER;
		
		if (rows <= 0)
			break;
		
		vlist = g_list_first (cursor->var_list);

		for (l = 0; l < var_count; l++)
		{
			var = vlist->data;
			vlist = g_list_next (vlist);
			
			if (var->value == NULL)
				continue;
			
			if (var->raw_to_value)
				var->raw_to_value (var);

			switch (var->value_type)
			{
				case G_TYPE_STRING:
					g_value_set_string (&values[l], (const gchar *) var->value);
					break;
					
				case G_TYPE_INT64:
					g_value_set_int64 (&values[l], *((gint64 *) var->value));
					break;
					
				case G_TYPE_INT:
					g_value_set_int (&values[l], *((gint *) var->value));
					break;
					
				case G_TYPE_DOUBLE:
					g_value_set_double (&values[l], *((gdouble *) var->value));
					break;
					
				default:

					if (var->value_type == GSQL_TYPE_DATETIME)
					{
						g_value_set_boxed (&values[l], var->value);
						break;
					}

					g_value_set_string (&values[l], (const gchar *) N_("&lt;value&gt;"));
			}			
		}
		gtk_list_store_insert_with_valuesv (liststore_new, NULL, -1,
											column_pos, values, var_count);
		
		GSQL_THREAD_LEAVE;
		
		rows_fetched++;
		
		if (rows_fetched >= rows_limit)
			break;
		
		if (editor->private->stop_fetch)
		{
			gsql_message_add (workspace, GSQL_MESSAGE_WARNING,
							  N_("The fetching has been canceled."));
			break;
			
		}

		if (cursor->stmt_affected_rows + rows_fetched >= editor->private->fetch_max)
		{
			g_snprintf (msg, 128, 
						N_("The maximum limit of fetch is reached [%d]. You can extend this limit in the settings."),
						editor->private->fetch_max);

			gsql_message_add (workspace, GSQL_MESSAGE_WARNING,
							  msg);
			break;
		}

	}
	
	cursor->stmt_affected_rows += rows_fetched;
	
	g_snprintf (msg, 128, N_("Rows fetched: %d [total: %llu]"), 
				rows_fetched, cursor->stmt_affected_rows);
	
	gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, msg);
	
	GSQL_THREAD_ENTER;
	
	/* FIXME:
		perfomance problem. just hack.
	 
		(lt-gsql:2714): Gtk-CRITICAL **: gtk_tree_view_unref_tree_helper: assertion `node != NULL' failed
	 */
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (result_treeview),
							 	 NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW (result_treeview),
							 	 GTK_TREE_MODEL(liststore_new));
	
	GSQL_DEBUG ("Cursor state = [%d]", gsql_cursor_get_state (cursor));
	
	if ((!cursor->session->engine->multi_statement) || 
		(gsql_cursor_get_state (cursor) == GSQL_CURSOR_STATE_FETCHED)
		)
	{		
		gtk_widget_set_sensitive (fetch_b, FALSE);
		gtk_widget_set_sensitive (fetch_all_b, FALSE);
		
	} else {
		
		gtk_widget_set_sensitive (fetch_b, TRUE);
		gtk_widget_set_sensitive (fetch_all_b, TRUE);
	}

	gtk_widget_set_sensitive (fetch_stop_b, FALSE);
	gtk_widget_set_sensitive (toolbar_run, TRUE);
	
	GSQL_THREAD_LEAVE;
	
	g_free (var_types);
	g_free (column_pos);
	g_free (values); 	
	
}



static void
on_sql_fetch (GtkToolButton *button, gpointer data)
{
	GError *err;
	GThread *thread = NULL;
	GSQLEditor *editor = data;
	guint limit;
	
	limit = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->private->fetch_limit));
	editor->private->fetch_step = limit;
	
	if (button == NULL) // isn't callback. run foreground.
	{	
		do_sql_fetch (editor);	
		return;
	} 
	
	thread = g_thread_create ((GThreadFunc) do_sql_fetch,
							  editor, 
							  FALSE,
							  &err);
	if (!thread)
		GSQL_DEBUG ("Couldn't create thread");	
	
}

static void
on_sql_fetch_all (GtkToolButton *button, gpointer data)
{
	GSQLEditor *editor = data;
	GError *err;
	GThread *thread = NULL;
	guint   limit;
	
	limit = gsql_conf_value_get_int (GSQL_CONF_SQL_FETCH_MAX);
	
	if (!limit)
		limit = GSQL_EDITOR_FETCH_MAX_DEFAULT;
	
	editor->private->fetch_step = limit;
	
	if (button == NULL) // isn't callback. run foreground.
	{	
		do_sql_fetch (editor);	
		return;
	} 
	
	thread = g_thread_create ((GThreadFunc) do_sql_fetch,
							  editor, 
							  FALSE,
							  &err);
	if (!thread)
		GSQL_DEBUG ("Couldn't create thread");	
	
}

static void
on_sql_fetch_stop (GtkToolButton *button, gpointer data)
{
	GSQLEditor *editor = data;
	
	editor->private->stop_fetch = TRUE;
}

static void
on_editor_cb_save (GSQLContent *content, gboolean save_as)
{
	GSQL_TRACE_FUNC;
	
	GSQLEditor *editor;
	GSQLWorkspace *workspace;
	GList	   *l_childs;
	GtkWidget  *chooser;
	static gchar *folder = NULL;
	gint ret;
	gchar *file, *old_file;
	gchar *disp_file, *old_disp_name;
	GError *err = NULL;
	GIOChannel *ioc;
	GIOStatus status;
	GtkTextBuffer *buffer;
	GtkTextIter start_i, end_i;
	gchar *data = NULL;
	gsize  data_written;
	gchar msg[GSQL_MESSAGE_LEN];
	GValue bvalue = { 0 };
	
	l_childs = gtk_container_get_children (GTK_CONTAINER (content));

	g_return_if_fail (g_list_length (l_childs) == 1);
	g_return_if_fail (GSQL_IS_EDITOR (l_childs->data));
	
	editor = GSQL_EDITOR (l_childs->data);
	workspace = gsql_session_get_workspace (NULL);
	
	if (!editor->private->is_file)
		save_as = TRUE;

	
	if (save_as)
	{
		GSQL_DEBUG ("save_as...");
		chooser = gtk_file_chooser_dialog_new (N_("Save as..."),
					       NULL,
					       GTK_FILE_CHOOSER_ACTION_SAVE,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					       NULL);
		if (folder)
			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), 
											 folder);
		if (!editor->private->is_file)
			file = g_strdup_printf ("%s.sql", 
									gsql_content_get_display_name (content));
		else
			file = gsql_content_get_display_name (content);

		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), file);
		
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (chooser), 
														TRUE);

		ret = gtk_dialog_run (GTK_DIALOG (chooser));

		g_free (file); file = NULL;

		if (ret == GTK_RESPONSE_ACCEPT)
		{
			file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

			if (folder) 
				g_free (folder);
			
			folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (chooser));
			GSQL_DEBUG ("Saving file as: [file=%s]", file);
			
			disp_file = g_filename_display_basename (file);
			
		}
		
		gtk_widget_destroy (chooser);
		
		if (!file) // !GTK_RESPONSE_ACCEPT
			return;
		
	} else {
		
		file = gsql_content_get_name (content);
		GSQL_DEBUG ("Saving file: [file=%s]", file);
		disp_file = gsql_content_get_display_name (content);
	}
	
	ioc = g_io_channel_new_file (file, "w+", &err);
	
	if (!ioc)
	{
		memset (msg, 0, GSQL_MESSAGE_LEN);
		g_snprintf (msg, GSQL_MESSAGE_LEN, N_("Failed to save file '%s'. %s"), 
					file, err->message);
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, msg);
		
		g_free (file);
		g_free (disp_file);
		
		return;
	}
	
	if (editor->private->encoding)
		g_io_channel_set_encoding (ioc, editor->private->encoding, &err);
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->private->source));
	gtk_text_buffer_get_bounds (buffer, &start_i, &end_i);
	data = gtk_text_buffer_get_text (buffer, &start_i, &end_i, FALSE);
	
	status = g_io_channel_write_chars (ioc, data, -1, &data_written, &err);
	
	memset (msg, 0, GSQL_MESSAGE_LEN);
	
	if (status != G_IO_STATUS_NORMAL)
	{
		
		g_snprintf (msg, GSQL_MESSAGE_LEN, N_("Failed to save file '%s'. %s"), 
					file, err->message);
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, msg);
		
		g_free (file);
		g_free (disp_file);
		
		if (data)
			g_free (data);
		
		g_io_channel_unref (ioc);
		
		return;
		
	} else {
		
		g_io_channel_flush (ioc, &err);
		
		g_snprintf (msg, GSQL_MESSAGE_LEN, N_("File saved: %s [%d bytes]"), 
					file, data_written);
		gsql_message_add (workspace, GSQL_MESSAGE_NOTICE, msg);
		
	}
	
	gsql_content_set_name_full (content, file, disp_file);
	gsql_content_set_changed (content, FALSE);
	gtk_text_buffer_set_modified (buffer, FALSE);
	
	if (!editor->private->is_file)
	{
		
		g_value_init(&bvalue, G_TYPE_BOOLEAN);
		
		g_value_set_boolean (&bvalue, TRUE);
	
		g_object_set_property (G_OBJECT (editor), "is-file",
						   &bvalue);
	}
	
	g_free (file);
	g_free (disp_file);
		
	if (data)
		g_free (data);
		
	g_io_channel_unref (ioc);
	
	gtk_window_set_focus (GTK_WINDOW (gsql_window), editor->private->source);

}

static void
on_editor_cb_close (GSQLContent *content, gboolean force)
{
	GSQL_TRACE_FUNC;
	
	GSQLEditor *editor;
	GSQLWorkspace *workspace;
	GList	   *l_childs;
	gboolean	changed = FALSE;
	GtkWidget  *dialog = NULL;
	GladeXML* gxml;
	
	gint ret;
	
	l_childs = gtk_container_get_children (GTK_CONTAINER (content));

	g_return_if_fail (g_list_length (l_childs) == 1);
	g_return_if_fail (GSQL_IS_EDITOR (l_childs->data));
	
	editor = l_childs->data;
	changed = gsql_content_get_changed (content);
	
	if ((!editor->private->is_file) || (!changed) || (force))
	{
		gtk_widget_destroy (GTK_WIDGET (content));
		
		return;
	}
	
	gxml = glade_xml_new (GSQL_GLADE_DIALOGS, "gsql_unsaved_file_dialog", NULL);
		
	g_return_if_fail (gxml);
		
	dialog = glade_xml_get_widget (gxml, "gsql_unsaved_file_dialog");	
	
	ret = gtk_dialog_run (GTK_DIALOG (dialog));
	
	gtk_widget_destroy ((GtkWidget *) dialog);
	
	g_object_unref(G_OBJECT(gxml));
		
	switch (ret)
	{
		case 1: // Save
			on_editor_cb_save (content, FALSE);
			gtk_widget_destroy (GTK_WIDGET (content));
			break;
			
		case 2: // Cancel
			return;
			
		case 3: // Discard
			gtk_widget_destroy (GTK_WIDGET (content));
			break;
			
	}
	
}


static void
on_editor_cb_revert (GSQLContent *content)
{
	GSQL_TRACE_FUNC;

	GIOChannel *ioc;
	GtkTextIter start_iter, end_iter;
	GList	   *l_childs;
	GSQLWorkspace *workspace;
	GSQLEditor *editor;
#define BUFFER_SIZE 4096
	gchar buffer[BUFFER_SIZE];
	GError *err = NULL;
	gchar msg[GSQL_MESSAGE_LEN];
	GtkTextBuffer *tbuffer;
	gboolean reading = TRUE;
	GladeXML* gxml;
	GtkWidget  *dialog = NULL;
	gchar *file;
	gint ret;
	
	l_childs = gtk_container_get_children (GTK_CONTAINER (content));

	g_return_if_fail (g_list_length (l_childs) == 1);
	g_return_if_fail (GSQL_IS_EDITOR (l_childs->data));
	
	editor = GSQL_EDITOR (l_childs->data);
	
	tbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->private->source));
	
	if (!editor->private->is_file)
	{
		gxml = glade_xml_new (GSQL_GLADE_DIALOGS, "gsql_isnotafile_clear_dialog", NULL);
		
		g_return_if_fail (gxml);
		
		dialog = glade_xml_get_widget (gxml, "gsql_isnotafile_clear_dialog");	
	
		ret = gtk_dialog_run (GTK_DIALOG (dialog));
	
		gtk_widget_destroy ((GtkWidget *) dialog);
	
		g_object_unref(G_OBJECT(gxml));
		
		switch (ret)
		{
			case 1: // Ok
				gtk_text_buffer_set_text (GTK_TEXT_BUFFER (tbuffer),
										  "", 0);
				break;
			
			case 2: // Cancel
				return;
			
		}
		
	}

	workspace = gsql_session_get_workspace (NULL);
	
	file = gsql_content_get_name (content);
	
	ioc = g_io_channel_new_file (file, "r+", &err);
	
	if (editor->private->encoding)
		g_io_channel_set_encoding (ioc, editor->private->encoding, &err);
	
	if (!ioc)
	{
		memset (msg, 0, GSQL_MESSAGE_LEN);
		g_snprintf (msg, GSQL_MESSAGE_LEN, N_("Failed to reload file '%s'. %s"), 
					file, err->message);
		gsql_message_add (workspace, GSQL_MESSAGE_ERROR, msg);
		
		return;
	}
	
	gtk_text_buffer_get_bounds (tbuffer, &start_iter, &end_iter);
	gtk_text_buffer_delete  (tbuffer, &start_iter, &end_iter);
	
	gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (tbuffer));
	
	memset (buffer, 0, BUFFER_SIZE);
	
	while (reading)
	{
		gsize bytes_read;
		GIOStatus status;
		
		status = g_io_channel_read_chars (ioc, buffer,
						  BUFFER_SIZE, &bytes_read,
						  &err);
		switch (status)
		{
			case G_IO_STATUS_EOF:
				GSQL_DEBUG ("Opening file: G_IO_STATUS_EOF");
				reading = FALSE;
				break;
			
			case G_IO_STATUS_NORMAL:
				GSQL_DEBUG ("Opening file: G_IO_STATUS_NORMAL");
				
				if (bytes_read == 0)
					continue;
				
				gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tbuffer), 
											  &end_iter);
				gtk_text_buffer_insert (GTK_TEXT_BUFFER (tbuffer),
							&end_iter, buffer, bytes_read);
				break;
				
			case G_IO_STATUS_AGAIN:
				GSQL_DEBUG ("Opening file: G_IO_STATUS_AGAIN");
				continue;

			case G_IO_STATUS_ERROR:
			default:
				GSQL_DEBUG ("Opening file: G_IO_STATUS_ERROR");
				
				memset (msg, 0, GSQL_MESSAGE_LEN);
				
				g_snprintf (msg, GSQL_MESSAGE_LEN, N_("Failed to load file '%s'. %s"), 
							file, err->message);
				
				gsql_message_add (workspace, GSQL_MESSAGE_ERROR, msg);
				
				gtk_text_buffer_set_text (GTK_TEXT_BUFFER (tbuffer), "", 0);
				
				g_io_channel_unref (ioc);
				
				return;
		}
	}
	
	gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (tbuffer));
	g_io_channel_unref (ioc);
	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (tbuffer), FALSE);
	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (tbuffer), &start_iter);
	gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (tbuffer), &start_iter);
	
	gtk_window_set_focus (GTK_WINDOW (gsql_window), editor->private->source);
	gsql_content_set_changed (content, FALSE);
}

static void 
on_editor_set_parent (GtkWidget *widget, GtkObject *object,
								  gpointer user_data)
{
	GSQL_TRACE_FUNC;
		
	GtkWidget *parent;
	
	parent = gtk_widget_get_parent (widget);
	
	g_return_if_fail (GSQL_IS_CONTENT (parent));
	
	g_signal_connect (G_OBJECT (parent),
					  "close",
					  G_CALLBACK (on_editor_cb_close),
					  NULL);
	g_signal_connect (G_OBJECT (parent),
					  "save",
					  G_CALLBACK (on_editor_cb_save),
					  NULL);
	g_signal_connect (G_OBJECT (parent),
					  "revert",
					  G_CALLBACK (on_editor_cb_revert),
					  NULL);	
}

static void 
on_buffer_changed (GtkWidget *widget, gpointer user_data)
{
	GSQL_TRACE_FUNC;
	
	GSQLContent *content;
	
	gsql_source_editor_markers_clear (GTK_SOURCE_BUFFER(widget));
	
	content = gsql_workspace_get_current_content (NULL);
	
	g_return_if_fail (GSQL_IS_CONTENT (content));
	
	gsql_content_set_changed (content, gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (widget)));
	
}

static void
on_sqleditor_fetch_limit_set (gpointer data)
{
	GSQLEditor *editor = data;
	guint limit_step;
	guint limit_max;
	GtkWidget *widget;
	
	g_return_if_fail (GSQL_IS_EDITOR (editor));
	
	limit_step = gsql_conf_value_get_int (GSQL_CONF_SQL_FETCH_STEP);
	
	if (!limit_step)
		limit_step = GSQL_EDITOR_FETCH_STEP_DEFAULT;
	
	limit_max = gsql_conf_value_get_int (GSQL_CONF_SQL_FETCH_MAX);
	
	if (!limit_max)
		limit_max = GSQL_EDITOR_FETCH_MAX_DEFAULT;
	
	widget = editor->private->checkb_limit;
	
	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{
		widget = editor->private->fetch_limit;
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget),
								   limit_step);
	}
	
	editor->private->fetch_max = limit_max;
	
}


