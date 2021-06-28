#include <Python.h>

#include <kognac/logs.h>

static PyObject *glob_set_logging_level(PyObject *self, PyObject *args) {
    int level = 0;
    if (!PyArg_ParseTuple(args, "i", &level))
        return NULL;
    Logger::setMinLevel(level);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef globalFunctions[] = {
    {"set_logging_level", glob_set_logging_level, METH_VARARGS,
        "Set the logging level. From 0 (trace) to 5 (error)." },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef glogmodule = {
    PyModuleDef_HEAD_INIT,
    "glog",   /* name of module */
    NULL,        /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module,
                    or -1 if the module keeps state in global variables. */
    NULL
};

extern PyTypeObject glog_EDBLayerType;
extern PyTypeObject glog_ProgramType;
extern PyTypeObject glog_ReasonerType;
extern PyTypeObject glog_WizardType;
extern PyTypeObject glog_TGType;
extern PyTypeObject glog_QuerierType;
extern PyTypeObject glog_TupleSetType;

PyMODINIT_FUNC PyInit_glog(void) {
    PyObject *m;
    m = PyModule_Create(&glogmodule);
    if (m == NULL)
        return NULL;

    if (PyType_Ready(&glog_EDBLayerType) < 0)
        return NULL;
    if (PyType_Ready(&glog_ProgramType) < 0)
        return NULL;
    if (PyType_Ready(&glog_ReasonerType) < 0)
        return NULL;
    if (PyType_Ready(&glog_WizardType) < 0)
        return NULL;
    if (PyType_Ready(&glog_TGType) < 0)
        return NULL;
    if (PyType_Ready(&glog_QuerierType) < 0)
        return NULL;
    if (PyType_Ready(&glog_TupleSetType) < 0)
        return NULL;


    Py_INCREF(&glog_EDBLayerType);
    Py_INCREF(&glog_ProgramType);
    Py_INCREF(&glog_ReasonerType);
    Py_INCREF(&glog_WizardType);
    Py_INCREF(&glog_TGType);
    Py_INCREF(&glog_QuerierType);
    Py_INCREF(&glog_TupleSetType);
    PyModule_AddObject(m, "EDBLayer", (PyObject *)&glog_EDBLayerType);
    PyModule_AddObject(m, "Program", (PyObject *)&glog_ProgramType);
    PyModule_AddObject(m, "Reasoner", (PyObject *)&glog_ReasonerType);
    PyModule_AddObject(m, "Wizard", (PyObject *)&glog_WizardType);
    PyModule_AddObject(m, "TG", (PyObject *)&glog_TGType);
    PyModule_AddObject(m, "Querier", (PyObject *)&glog_QuerierType);
    PyModule_AddObject(m, "TupleSet", (PyObject *)&glog_TupleSetType);
    PyModule_AddFunctions(m, globalFunctions);

    //Default logging level to info
    Logger::setMinLevel(INFOL);

    return m;
}
