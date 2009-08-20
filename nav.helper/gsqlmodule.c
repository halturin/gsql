#include <pygobject.h>
#include <pygtk/pygtk.h>

#define PYGSQL_MAJOR_VERSION 0
#define PYGSQL_MINOR_VERSION 1
#define PYGSQL_MICRO_VERSION 0

void pygsql_register_classes (PyObject *d);
void pygsql_add_constants (PyObject *module, const gchar *strip_prefix);

extern PyMethodDef pygsql_functions[];

DL_EXPORT (void);
initgsql (void)
{
    PyObject *m, *d;
    
    m = Py_InitModule ("gsql", pygsql_functions);
    d = PyModule_GetDict (m);
    
    init_pygobject ();
    init_pygtk ();
    
    pygsql_register_classes (d);
    pygsql_add_constants (m, "GSQL_");
    
    PyModule_AddObject (m, "pygsql_version",
	    PyBuildValue ("iii",
			PYGSQL_MAJOR_VERSION,
			PYGSQL_MINOR_VERSION,
			PYGSQL_MICRO_VERSION));
			
    if (PyErr_Occured ())
	Py_FatalError ("can't initialise module gsql");


}

