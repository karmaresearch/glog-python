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

/*** Methods ***/
static PyObject * tupleset_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int tupleset_init(glog_TupleSet *self, PyObject *args, PyObject *kwds);
static void tupleset_dealloc(glog_TupleSet* self);
static PyObject* tupleset_get_n_facts(PyObject* self, PyObject *args);

static PyMethodDef TupleSet_methods[] = {
    {"get_n_facts", tupleset_get_n_facts, METH_VARARGS, "Get number of facts." },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyTypeObject glog_TupleSetType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        "glog.TupleSet",             /* tp_name */
    sizeof(glog_TupleSet),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor) tupleset_dealloc, /* tp_dealloc */
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
    "TupleSet",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    TupleSet_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)tupleset_init,      /* tp_init */
    0,                         /* tp_alloc */
    tupleset_new,                 /* tp_new */
};

static PyObject * tupleset_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    glog_TupleSet *self;
    self = (glog_TupleSet*)type->tp_alloc(type, 0);
    self->data = NULL;
    self->nodeId = 0;
    self->ruleIdx = 0;
    self->step = 0;
    self->predId = 0;
    return (PyObject *)self;
}

static int tupleset_init(glog_TupleSet *self, PyObject *args, PyObject *kwds) {
    return 0;
}

static void tupleset_dealloc(glog_TupleSet* self) {
    glog_TupleSet *s = self;
    s->data = NULL;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* tupleset_get_n_facts(PyObject* self, PyObject *args) {
    auto s = ((glog_TupleSet*)self);
    auto n = s->data->getNRows();
    return PyLong_FromLong(n);
}
