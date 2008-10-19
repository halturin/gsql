/***************************************************************************
 *            oracle.c
 *
 *  Sun Sep 16 22:15:46 2007
 *  Copyright  2007  Taras Halturin
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

#include <config.h>
#include <libgsql/session.h>
#include <libgsql/workspace.h>
#include <libgsql/common.h>
#include <libgsql/utils.h>

#include "oracle.h"
#include "oracle_cursor.h"
#include "engine_session.h"

#include <string.h>
#include <sys/utsname.h>


static void
oracle_client_info (gchar *buf);

gboolean
oracle_session_open (GSQLEOracleSession *oracle_session, 
					 gchar *username,
					 gchar *password,
					 gchar *database, 
					 gchar *buffer)
{
	GSQL_TRACE_FUNC;

    unsigned char buf[64];
	gint ret;	

    
	/* initialize the mode to be the threaded and object environment */
	if ( OCIEnvNlsCreate(&(oracle_session->envhp), OCI_THREADED|OCI_OBJECT, (dvoid *)0,
                    0, 0, 0, (size_t) 0, (dvoid **)0, 0, 0)
		== OCI_ERROR
		) 
	{
			g_snprintf (buffer, 256,"OCIEnvNlsCreate... failed");
		 	return FALSE;
	};

	/* allocate a server handle */
	if ( OCIHandleAlloc ((dvoid *)(oracle_session->envhp), 
					   (dvoid **)&(oracle_session->srvhp),
						OCI_HTYPE_SERVER, 0, (dvoid **) 0)
		== OCI_ERROR
		)
	{
			g_snprintf (buffer, 256,"OCIHandleAlloc (allocate a server handle)... failed");
		 	return FALSE;
	};

	/* allocate an error handle */
	if ( OCIHandleAlloc ((dvoid *)(oracle_session->envhp), 
					   (dvoid **)&(oracle_session->errhp),
						OCI_HTYPE_ERROR, 0, (dvoid **) 0)
		== OCI_ERROR
		)
	{
			g_snprintf (buffer, 256,"OCIHandleAlloc (allocate an error handle)... failed");
		 	return FALSE;
	};

	/* create a server context */
		
	if ( OCIServerAttach (oracle_session->srvhp, 
						oracle_session->errhp, 
						(text *) database,
						g_utf8_strlen (database, 64), 
						OCI_DEFAULT)
		== OCI_ERROR
		)
	{
			g_snprintf (buffer, 256, "%s",
						oracle_get_error_string(oracle_session->errhp)
						);
		 	return FALSE;
	};
	   
	/* allocate a service handle */
	if ( OCIHandleAlloc ((dvoid *) (oracle_session->envhp), 
					   (dvoid **)&(oracle_session->svchp),
						OCI_HTYPE_SVCCTX, 0, (dvoid **) 0)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256,"OCIHandleAlloc (allocate a service handle)... failed");
		return FALSE;
	};
	   
	/* set the server attribute in the service context handle*/
	if ( OCIAttrSet ((dvoid *) oracle_session->svchp, 
				   OCI_HTYPE_SVCCTX,
					(dvoid *) oracle_session->srvhp, 
				   (ub4) 0, 
				   OCI_ATTR_SERVER, 
				   oracle_session->errhp)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256,"OCIAttrSet... failed");
		return FALSE;
	};
	   
	/* allocate a user session handle */
	if ( OCIHandleAlloc ((dvoid *) (oracle_session->envhp), 
					   (dvoid **)&(oracle_session->usrhp),
						OCI_HTYPE_SESSION, 0, (dvoid **) 0)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256,"OCIHandleAlloc (allocate a user session handle)... failed");
		return FALSE;
	};
	   
	/* set user name attribute in user session handle */
		
	if ( OCIAttrSet ((dvoid *) oracle_session->usrhp, 
				   OCI_HTYPE_SESSION,
				   (dvoid *) username,
				   (ub4) g_utf8_strlen(username, 64),
				   OCI_ATTR_USERNAME,
				   oracle_session->errhp)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256,"OCIAttrSet (OCI_ATTR_USERNAME)... failed");
		return FALSE;
	};
	   
	/* set password attribute in user session handle */
		
	if ( OCIAttrSet ((dvoid *)(oracle_session->usrhp),
				   OCI_HTYPE_SESSION,
				   (dvoid *)password,
				   (ub4) g_utf8_strlen(password, 64),
				   OCI_ATTR_PASSWORD,
				   oracle_session->errhp)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256,"OCIAttrSet(OCI_ATTR_PASSWORD)... failed");
		return FALSE;
	};


	/* make the connection*/
	if (  OCISessionBegin ((dvoid *) oracle_session->svchp, 
						 oracle_session->errhp, 
						 oracle_session->usrhp,
						 OCI_CRED_RDBMS,
						 oracle_session->mode)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256, "%s",
						oracle_get_error_string(oracle_session->errhp)
						);
		return FALSE;
	};

	oracle_client_info(oracle_session->client_version);

	if ( OCIAttrSet ((dvoid *)(oracle_session->usrhp),
				   OCI_HTYPE_SESSION,
				   (dvoid *) oracle_session->client_version, 
                   (ub4) g_utf8_strlen(oracle_session->client_version, 64),
				   OCI_ATTR_CLIENT_INFO,
				   oracle_session->errhp)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256, "OCIAttrSet(OCI_ATTR_CLIENT_INFO)... failed");
		return FALSE;
	};

	   
	/* set the user session attribute in the service context handle*/
	if ( OCIAttrSet ((dvoid *)oracle_session->svchp, 
				   OCI_HTYPE_SVCCTX,
				   (dvoid *) oracle_session->usrhp,
				   (ub4) 0, 
				   OCI_ATTR_SESSION, oracle_session->errhp)
		== OCI_ERROR
		)
	{
		g_snprintf (buffer, 256, "OCIAttrSet (OCI_ATTR_SESSION)... failed");
		return FALSE;
	};

	OCIServerVersion ((dvoid *) oracle_session->svchp, 
					  oracle_session->errhp, 
					  oracle_session->server_version, 
					  1024, OCI_HTYPE_SVCCTX);
	GSQL_DEBUG ("oracle_session->server_version=[%s]", oracle_session->server_version);
	
	oracle_session->dbms_output = FALSE;
	oracle_session->debug_mode = FALSE;
	   
	return TRUE;   
};

void
oracle_session_commit (GSQLSession *session)
{
	GSQL_TRACE_FUNC;

	sword ret;
	gchar *mess;
	GSQLWorkspace *workspace;
	GSQLEOracleSession *o_session;

	o_session = (GSQLEOracleSession *) session->spec;
	
	workspace = gsql_session_get_workspace (session);
	
	ret = OCITransCommit(o_session->svchp, o_session->errhp, OCI_DEFAULT);
	
	if (ret != OCI_SUCCESS)
	{
		mess = oracle_get_error_string (o_session->errhp);
		gsql_message_add (workspace,
							  GSQL_MESSAGE_ERROR, mess);
		g_free (mess);
	} else
		gsql_message_add (workspace,
							  GSQL_MESSAGE_NOTICE, N_("Transaction commited"));
	
}

void
oracle_session_rollback (GSQLSession *session)
{
	GSQL_TRACE_FUNC;

	sword ret;
	gchar *mess;
	GSQLWorkspace *workspace;
	GSQLEOracleSession * o_session;
	
	o_session = (GSQLEOracleSession *) session->spec;
	
	workspace = gsql_session_get_workspace (session);
	
	ret = OCITransRollback(o_session->svchp, o_session->errhp, OCI_DEFAULT);
	
	if (ret != OCI_SUCCESS)
	{
		mess = oracle_get_error_string (o_session->errhp);
		gsql_message_add (workspace,
							  GSQL_MESSAGE_ERROR, mess);
		g_free (mess);
	} else
		gsql_message_add (workspace,
							  GSQL_MESSAGE_NOTICE, N_("Transaction rolled back"));
	
}

gboolean 
oracle_session_close (GSQLSession *session, gchar *buffer)
{
	GSQL_TRACE_FUNC;

	/* End session and detach from server */
	GSQLEOracleSession *o_session;
	
	o_session = (GSQLEOracleSession *) session->spec;
	
	gsql_session_close (session);
	
	OCISessionEnd (o_session->svchp, o_session->errhp, 
				   o_session->usrhp, OCI_DEFAULT);
	OCIServerDetach (o_session->srvhp, o_session->errhp, 
					 OCI_DEFAULT);
	OCIHandleFree ((dvoid *)o_session->errhp, OCI_HTYPE_ERROR);
	OCIHandleFree ((dvoid *)o_session->srvhp, OCI_HTYPE_SERVER);
	OCIHandleFree ((dvoid *)o_session->svchp, OCI_HTYPE_SVCCTX);
	OCIHandleFree ((dvoid *)o_session->usrhp, OCI_HTYPE_SESSION);
	OCIHandleFree ((dvoid *)o_session->envhp, OCI_HTYPE_ENV);

	g_free(o_session);
	
	return TRUE;
};

static void
oracle_client_info (gchar *buf)
{
	GSQL_TRACE_FUNC;

	struct utsname u;
  
	memset(buf, 0, 64);
	uname (&u);
    // limit size for oracle client info = 64
	g_snprintf(buf, 64, "%s [%s] (%s) on %s [%s]", PROJECT_NAME, VERSION, PROJECT_URL, u.sysname, u.release);

	return;
};

gboolean 
oracle_check_error (GSQLCursor *cursor, gint ret)
{
	GSQL_TRACE_FUNC;

	text  msgbuf[GSQL_MESSAGE_LEN];
	sb4   errcode = 0;       
	gchar *pos = NULL;
	gboolean r = TRUE;
	ub4   n_errs = 0;
#define GSQL_ERR_MAX_ROW 5
	OCIError *errhndl, *errhp2;
	ub4 row_offset[GSQL_ERR_MAX_ROW];
	gint i, ret1;
	GSQLEOracleSession *spec_session;
	GSQLEOracleCursor  *spec_cursor;
	GSQLWorkspace *workspace;
	gchar *mess;
	
	spec_session = (GSQLEOracleSession *) cursor->session->spec;
	spec_cursor = (GSQLEOracleCursor *) cursor->spec;
	workspace = gsql_session_get_workspace (cursor->session);
	
	switch (ret)
	{
		case OCI_SUCCESS:
			return FALSE;
		
		case OCI_SUCCESS_WITH_INFO:
			r = FALSE;
			
			OCIErrorGet ((dvoid *) spec_cursor->errhp, (ub4) 1, (text *) NULL, &errcode,
                        msgbuf, (ub4) GSQL_MESSAGE_LEN, (ub4) OCI_HTYPE_ERROR);
			
			pos = g_strrstr (msgbuf, "\n");
			if (pos) *pos = 0;
			
			mess = gsql_utils_escape_string (msgbuf);
			
			gsql_message_add (workspace, GSQL_MESSAGE_WARNING, mess);
			
			g_free (mess);
			break;
		
		case OCI_ERROR:                 
		default:

			OCIErrorGet ((dvoid *) spec_cursor->errhp, (ub4) 1, (text *) NULL, &errcode,
						 msgbuf, (ub4) GSQL_MESSAGE_LEN, (ub4) OCI_HTYPE_ERROR);

			pos = g_strrstr (msgbuf, "\n");
			if (pos) *pos = 0;

			if ((errcode == 1403)||(errcode == 100)) // No data found (sql/ansi mode)
			{
				r = FALSE;
				break;
			}
			
			GSQL_DEBUG ("Oracle error: [code = %d] [ret = %d] [mess = %s]", errcode, ret, msgbuf);
			
			mess = gsql_utils_escape_string (msgbuf);
			gsql_message_add (workspace, GSQL_MESSAGE_ERROR, mess);
			
			g_free (mess);
			break;
	}
	
	return r;
}


gchar *
oracle_get_error_string (OCIError *error)
{
	GSQL_TRACE_FUNC;

	sb4   errcode = 0;
	text  msgbuf[256];
	gchar *pos = NULL;
	
	OCIErrorGet ((dvoid *) error, (ub4) 1, (text *) NULL, &errcode,
					msgbuf, (ub4) 256, (ub4) OCI_HTYPE_ERROR);

	pos = g_strrstr (msgbuf, "\n");

	if (pos) 
		*pos = 0;

	return g_strndup ((const gchar *) msgbuf, 256);
}

