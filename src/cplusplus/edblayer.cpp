/*
 * Copyright 2021 Jacopo Urbani
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 **/


#include <Python.h>
#include <iostream>
#include <vector>

#include <glog-python/glog.h>
#include <glog-python/pyedbtable.h>
#include <kognac/utils.h>

/*** Methods ***/
static PyObject * edblayer_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int edblayer_init(glog_EDBLayer *self, PyObject *args, PyObject *kwds);
static PyObject* edblayer_add_source(PyObject* self, PyObject *args);
static PyObject* edblayer_get_term_id(PyObject* self, PyObject *args);
static void edblayer_dealloc(glog_EDBLayer* self);


static PyMethodDef EDBLayer_methods[] = {
    {"add_source", edblayer_add_source, METH_VARARGS, "Add a new source associated to an EDB predicate." },
    {"get_term_id", edblayer_get_term_id, METH_VARARGS, "Get the numerical ID associated to a term." },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyTypeObject glog_EDBLayerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        "glog.EDBLayer",             /* tp_name */
    sizeof(glog_EDBLayer),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor) edblayer_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "EDB Layer",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    EDBLayer_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)edblayer_init,      /* tp_init */
    0,                         /* tp_alloc */
    edblayer_new,                 /* tp_new */
};

static PyObject * edblayer_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    glog_EDBLayer *self;
    self = (glog_EDBLayer*)type->tp_alloc(type, 0);
    self->conf = NULL;
    self->e = NULL;
    return (PyObject *)self;
}

static int edblayer_init(glog_EDBLayer *self, PyObject *args, PyObject *kwds)
{
    const char *path = NULL;
    if (!PyArg_ParseTuple(args, "|s", &path))
        return -1;

    // Create a new trident database and return and ID to it
    if (path != NULL) {
        //Load EDB Layer at path
        self->conf = new EDBConf(path);
        self->conf->setRootPath(Utils::parentDir(path));
        self->e = new EDBLayer(*self->conf, false);
    } else {
        self->conf = new EDBConf("");
        self->conf->setRootPath(".");
        self->e = new EDBLayer(*self->conf, false);
    }
    return 0;
}

static PyObject* edblayer_add_source(PyObject* self, PyObject *args)
{
    glog_EDBLayer *s = (glog_EDBLayer*)self;
    const char *predName = NULL;
    PyObject *obj = NULL;
    if (!PyArg_ParseTuple(args, "sO", &predName, &obj)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    std::string sPredName(predName);
    auto predId = s->e->addEDBPredicate(sPredName);
    std::shared_ptr<EDBTable> ptr = std::shared_ptr<EDBTable>(
            new PyTable(predId, sPredName, s->e, obj));
    s->e->addEDBTable(predId, "PYTHON", ptr);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* edblayer_get_term_id(PyObject* self, PyObject *args)
{
    const char *term;
    if (!PyArg_ParseTuple(args, "s", &term))
        return NULL;
    glog_EDBLayer *s = (glog_EDBLayer*)self;
    nTerm value;
    auto len = strlen(term);
    bool resp = s->e->getDictNumber(term, len, value);
    if (resp) {
        return PyLong_FromLong(value);
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static void edblayer_dealloc(glog_EDBLayer* self)
{
    if (self->e)
        delete self->e;
    if (self->conf)
        delete self->conf;
    Py_TYPE(self)->tp_free((PyObject*)self);
}
