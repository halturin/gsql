#include <pygobject.h>
#include <pygtk/pygtk.h>
#include <navtree.h>

#define PYGSQL_MAJOR_VERSION 0
#define PYGSQL_MINOR_VERSION 1
#define PYGSQL_MICRO_VERSION 0

static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)


//static const PyMethodDef _PyGSQLNavTree_methods[] = 
//    {NULL, NULL, 0, NULL}
//},

static PyObject *
_wrap_gsql_navtree_new (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    PyErr_SetString (PyExc_TypeError,
		     "cannot create instance of type GSQLNavTree");
    
    return NULL;
}

PyTypeObject G_GNUC_INTERNAL PyGSQLNavTree_Type = {
    PyObject_HEAD_INIT(NULL)
    0,			/* ob_size */
    "gsql.GSQLNavTree",	// tp_name
    sizeof(PyGObject),	//tp_basicsize
    0,			// item_size
    /*methods*/
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)0, /* _PyGSQLNavTree_methods,  tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,//_wrap_gsql_navtree_new,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */

};


DL_EXPORT (void);
initgsql (void)
{
    PyObject *m, *d, *in;
    
    m = Py_InitModule ("gsql", NULL);//pygsql_methods);
    d = PyModule_GetDict (m);
    
    init_pygobject ();
    init_pygtk ();
    
    if ((in = PyImport_ImportModule("gobject")) != NULL)
    {
	_PyGObject_Type = (PyTypeObject *)PyObject_GetAttrString(in, "GObject");
	if (_PyGObject_Type == NULL)
	{
	    PyErr_SetString(PyExc_ImportError,
			    "cannot import name GObject from gobject");
	    return;	
	}
    
    }
    
    pygobject_register_class (d, "GSQLNavTree", GSQL_NAVTREE_TYPE, 
				&PyGSQLNavTree_Type, 
				Py_BuildValue("(O)", &PyGObject_Type));
//    pyg_set_object_has_new_constructor (GSQL_NAVTREE_TYPE);
    
    PyModule_AddObject (m, "version",
	    Py_BuildValue ("iii",
			PYGSQL_MAJOR_VERSION,
			PYGSQL_MINOR_VERSION,
			PYGSQL_MICRO_VERSION));
/*			
    if (PyErr_Occured ())
	Py_FatalError ("can't initialise module gsql");
*/


}

