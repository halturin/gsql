#include <pygobject.h>

void gsql_navtree_register_classes (PyObject *d);
extern PyMethodDef gsql_navtree_functions[];

DL_EXPORT(void)
initgsql(void) {
  PyObject *m, *d;

  init_pygobject();
  m = Py_InitModule ("gsql", gsql_navtree_functions);
  d = PyModule_GetDict(m);

  gsql_navtree_register_classes(d);
  
  if ( PyErr_Occurred() ) {
    Py_FatalError ("Unable to initialize module gsql");
  }
}
