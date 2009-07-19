#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import gobject
import gtk




GSQL_STOCK_ALL_SCHEMAS			= "gsql-all-schemas"
GSQL_STOCK_ARGUMENTS			= "gsql-arguments"
GSQL_STOCK_CLOSE				= "gsql-close"
GSQL_STOCK_COLUMNS				= "gsql-columns"
GSQL_STOCK_CONSTRAINT			= "gsql-constraints"
GSQL_STOCK_CONSTRAINT_P			= "gsql-constraints-p"
GSQL_STOCK_CONSTRAINT_F			= "gsql-constraints-f"
GSQL_STOCK_CONSTRAINT_U			= "gsql-constraints-u"
GSQL_STOCK_CONSTRAINT_C			= "gsql-constraints-c"
GSQL_STOCK_FIND					= "gsql-find"
GSQL_STOCK_FUNCTIONS			= "gsql-functions"
GSQL_STOCK_GSQL					= "gsql-gsql"
GSQL_STOCK_INDEXES				= "gsql-indexes"
GSQL_STOCK_LOGO					= "gsql-logo"
GSQL_STOCK_MOUTPUT				= "gsql-msg-output"
GSQL_STOCK_MERROR				= "gsql-msg-error"
GSQL_STOCK_MNORMAL				= "gsql-msg-normal"
GSQL_STOCK_MNOTICE				= "gsql-msg-notice"
GSQL_STOCK_MWARNING				= "gsql-msg-warning"
GSQL_STOCK_MY_SCHEMA			= "gsql-my-schema"
GSQL_STOCK_OBJ_CLONE			= "gsql-object-clone"
GSQL_STOCK_OBJ_CODE				= "gsql-object-code"
GSQL_STOCK_OBJ_NEW				= "gsql-object-new"
GSQL_STOCK_OBJ_REVERT			= "gsql-object-revert"
GSQL_STOCK_OBJ_SAVE				= "gsql-object-save"
GSQL_STOCK_OBJ_SQL				= "gsql-object-sql"
GSQL_STOCK_PROCEDURES			= "gsql-procedures"
GSQL_STOCK_PRIVILEGES			= "gsql-privileges"
GSQL_STOCK_SEQUENCES			= "gsql-sequences"
GSQL_STOCK_SESSION_CLOSE		= "gsql-session-close"
GSQL_STOCK_SESSION_COMMIT		= "gsql-session-commit"
GSQL_STOCK_SESSION_NEW			= "gsql-session-new"
GSQL_STOCK_SESSION_ROLLBACK		= "gsql-session-rollback"
GSQL_STOCK_SQL_FETCH_ALL		= "gsql-sql-fetch-all"
GSQL_STOCK_SQL_FETCH_NEXT		= "gsql-sql-fetch-next"
GSQL_STOCK_SQL_RUN_AT_CURSOR	= "gsql-sql-run-at-cursor"
GSQL_STOCK_SQL_RUN				= "gsql-sql-run"
GSQL_STOCK_SQL_RUN_STEP			= "gsql-sql-run-step"
GSQL_STOCK_SQL_SHOW_HIDE		= "gsql-sql-show-hide"
GSQL_STOCK_SQL_STOP_ON_ERR		= "gsql-sql-stop-on-err"
GSQL_STOCK_SQL_STOP				= "gsql-sql-stop"
GSQL_STOCK_SQL_UPDATEABLE		= "gsql-sql-updateable"
GSQL_STOCK_TABLES				= "gsql-tables"
GSQL_STOCK_TRIGGERS				= "gsql-triggers"
GSQL_STOCK_UNKNOWN				= "gsql-unknown"
GSQL_STOCK_USERS				= "gsql-users"
GSQL_STOCK_VIEWS				= "gsql-views"

stock_dict =  { GSQL_STOCK_ALL_SCHEMAS 			 :'./pixmaps/all.schemas.png' ,
				GSQL_STOCK_ARGUMENTS 			 :'./pixmaps/arguments.png' ,
				GSQL_STOCK_CLOSE 				 :'./pixmaps/close.png' ,
				GSQL_STOCK_COLUMNS 				 :'./pixmaps/columns.png' ,
				GSQL_STOCK_CONSTRAINT 			 :'./pixmaps/constraints.png' ,
				GSQL_STOCK_CONSTRAINT_P 		 :'./pixmaps/constraints.primary.png' ,
				GSQL_STOCK_CONSTRAINT_F 		 :'./pixmaps/constraints.foreign.png' ,
				GSQL_STOCK_CONSTRAINT_U 		 :'./pixmaps/constraints.unique.png' ,
				GSQL_STOCK_CONSTRAINT_C 		 :'./pixmaps/constraints.check.png' ,
				GSQL_STOCK_FIND 				 :'./pixmaps/find.png' ,
				GSQL_STOCK_FUNCTIONS 			 :'./pixmaps/functions.png' ,
				GSQL_STOCK_GSQL 				 :'./pixmaps/gsql.png' ,
				GSQL_STOCK_INDEXES 				 :'./pixmaps/indexes.png' ,
				GSQL_STOCK_LOGO 				 :'./pixmaps/logo.png' ,
				GSQL_STOCK_MOUTPUT 				 :'./pixmaps/msg_dboutput.png' ,
				GSQL_STOCK_MERROR 				 :'./pixmaps/msg_error.png' ,
				GSQL_STOCK_MNORMAL 				 :'./pixmaps/msg_normal.png' ,
				GSQL_STOCK_MNOTICE 				 :'./pixmaps/msg_notice.png' ,
				GSQL_STOCK_MWARNING 			 :'./pixmaps/msg_warning.png' ,
				GSQL_STOCK_MY_SCHEMA 			 :'./pixmaps/my.schema.png' ,
				GSQL_STOCK_OBJ_CLONE 			 :'./pixmaps/object_clone.png' ,
				GSQL_STOCK_OBJ_CODE 			 :'./pixmaps/object_code.png' ,
				GSQL_STOCK_OBJ_NEW 				 :'./pixmaps/object_new.png' ,
				GSQL_STOCK_OBJ_REVERT 			 :'./pixmaps/object_revert.png' ,
				GSQL_STOCK_OBJ_SAVE 			 :'./pixmaps/object_save.png' ,
				GSQL_STOCK_OBJ_SQL 				 :'./pixmaps/object_sql.png' ,
				GSQL_STOCK_PROCEDURES 			 :'./pixmaps/procedures.png' ,
				GSQL_STOCK_PRIVILEGES 			 :'./pixmaps/privileges.png' ,
				GSQL_STOCK_SEQUENCES 			 :'./pixmaps/sequences.png' ,
				GSQL_STOCK_SESSION_CLOSE 		 :'./pixmaps/session_close.png' ,
				GSQL_STOCK_SESSION_COMMIT 		 :'./pixmaps/session_commit.png' ,
				GSQL_STOCK_SESSION_NEW 			 :'./pixmaps/session_new.png' ,
				GSQL_STOCK_SESSION_ROLLBACK 	 :'./pixmaps/session_rollback.png' ,
				GSQL_STOCK_SQL_FETCH_ALL 		 :'./pixmaps/sql_fetch_all.png' ,
				GSQL_STOCK_SQL_FETCH_NEXT 		 :'./pixmaps/sql_fetch_next.png' ,
				GSQL_STOCK_SQL_RUN_AT_CURSOR 	 :'./pixmaps/sql_run_at_cursor.png' ,
				GSQL_STOCK_SQL_RUN 				 :'./pixmaps/sql_run.png' ,
				GSQL_STOCK_SQL_RUN_STEP 		 :'./pixmaps/sql_run_step.png' ,
				GSQL_STOCK_SQL_SHOW_HIDE 		 :'./pixmaps/sql_showhide_result.png' ,
				GSQL_STOCK_SQL_STOP_ON_ERR 		 :'./pixmaps/sql_stop_onerror.png' ,
				GSQL_STOCK_SQL_STOP 			 :'./pixmaps/sql_stop.png' ,
				GSQL_STOCK_SQL_UPDATEABLE 		 :'./pixmaps/sql_updateable.png' ,
				GSQL_STOCK_TABLES 				 :'./pixmaps/tables.png' ,
				GSQL_STOCK_TRIGGERS 			 :'./pixmaps/triggers.png' ,
				GSQL_STOCK_UNKNOWN 				 :'./pixmaps/unknown.png' ,
				GSQL_STOCK_USERS 				 :'./pixmaps/users.png' ,
				GSQL_STOCK_VIEWS 				 :'./pixmaps/views.png' }
				
def icon_store_init(self, store):
	
	
	keys = stock_dict.keys()
	keys.sort()
	
	for i in keys:
		stock_image = i
		file_name = stock_dict[i]
		stock_name = str(i).replace('gsql-','GSQL_STOCK_').upper().replace('-','_')
		
		pixbuf = gtk.gdk.pixbuf_new_from_file(file_name)
		iconset = gtk.IconSet(pixbuf)
		self.icon_factory.add(stock_image, iconset)
		
		store.append([stock_image, stock_name, file_name])

		
	

