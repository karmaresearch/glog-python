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
static PyObject * querier_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int querier_init(glog_Querier *self, PyObject *args, PyObject *kwds);
static void querier_dealloc(glog_Querier* self);
static PyObject* querier_get_derivation_tree(PyObject* self, PyObject *args);
static PyObject* querier_get_derivation_tree_in_TupleSet(PyObject* self, PyObject *args);
static PyObject* querier_get_list_predicates(PyObject* self, PyObject *args);
static PyObject* querier_get_node_details_predicate(PyObject* self, PyObject *args);
static PyObject* querier_get_facts_in_TG_node(PyObject* self, PyObject *args);
static PyObject* querier_get_fact_in_TupleSet(PyObject* self, PyObject *args);
static PyObject* querier_get_facts_coordinates_with_predicate(PyObject* self, PyObject *args);
static PyObject* querier_get_leaves(PyObject* self, PyObject *args);
static PyObject* querier_get_all_facts(PyObject* self, PyObject *args);
static PyObject* querier_get_predicate_name(PyObject* self, PyObject *args);
static PyObject* querier_get_term_name(PyObject* self, PyObject *args);

static PyMethodDef Querier_methods[] = {
    {"get_derivation_tree", querier_get_derivation_tree, METH_VARARGS, "Get derivation tree of a fact." },
    {"get_derivation_tree_in_TupleSet", querier_get_derivation_tree_in_TupleSet, METH_VARARGS, "Get derivation tree of a fact in a given TupleSet." },
    {"get_list_predicates", querier_get_list_predicates, METH_VARARGS, "Get list predicates stored in the TG." },
    {"get_node_details_predicate", querier_get_node_details_predicate, METH_VARARGS, "Get the nodes for a given predicate." },
    {"get_facts_in_TG_node", querier_get_facts_in_TG_node, METH_VARARGS, "Get the facts stored on a node in the TG." },
    {"get_fact_in_TupleSet", querier_get_fact_in_TupleSet, METH_VARARGS, "Get the fact stored on a given TupleSet." },
    {"get_facts_coordinates_with_predicate", querier_get_facts_coordinates_with_predicate, METH_VARARGS, "Get all the facts with a given predicate." },
    {"get_leaves", querier_get_leaves, METH_VARARGS, "Get the leaves for a given fact." },
    {"get_all_facts", querier_get_all_facts, METH_VARARGS, "Get all the facts in the TG." },
    {"get_predicate_name", querier_get_predicate_name, METH_VARARGS, "Get predicate name." },
    {"get_term_name", querier_get_term_name, METH_VARARGS, "Get textual name of a term." },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyTypeObject glog_QuerierType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        "glog.Querier",             /* tp_name */
    sizeof(glog_Querier),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor) querier_dealloc, /* tp_dealloc */
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
    "Querier",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Querier_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)querier_init,      /* tp_init */
    0,                         /* tp_alloc */
    querier_new,                 /* tp_new */
};

static PyObject * querier_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    glog_Querier *self;
    self = (glog_Querier*)type->tp_alloc(type, 0);
    self->g = NULL;
    return (PyObject *)self;
}

static int querier_init(glog_Querier *self, PyObject *args, PyObject *kwds) {
    PyObject *arg = NULL;
    if (!PyArg_ParseTuple(args, "O", &arg))
        return -1;
    if (arg != NULL) {
        if (strcmp(arg->ob_type->tp_name, "glog.TG") != 0)
            return -1;
        Py_INCREF(arg);
        self->g = (glog_TG*)arg;
        self->q = self->g->g->getQuerier();
    }
    return 0;
}

static void querier_dealloc(glog_Querier* self) {
    if (self->g != NULL) {
        Py_DECREF(self->g);
    }
    self->q = NULL;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* querier_get_derivation_tree(PyObject* self, PyObject *args) {
    size_t nodeId;
    size_t factId;
    if (!PyArg_ParseTuple(args, "ll", &nodeId, &factId)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    auto out = ((glog_Querier*)self)->q->getDerivationTree(nodeId, factId);
    std::stringstream ssOut;
    JSON::write(ssOut, out);
    std::string sOut = ssOut.str();
    return PyUnicode_FromString(sOut.c_str());
}

static PyObject* querier_get_derivation_tree_in_TupleSet(PyObject* self,
        PyObject *args) {
    PyObject *node;
    size_t factId = 0;
    if (!PyArg_ParseTuple(args, "Ol", &node, &factId)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    glog_Querier *s = ((glog_Querier*)self);
    glog_TupleSet* n = (glog_TupleSet*)node;
    if (factId >= n->data->getNRows()) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    auto &ie = n->nodes[factId];
    DuplicateChecker checker;
    auto out = s->q->getDerivationTree(n->data,
            n->nodeId,
            factId,
            n->predId,
            n->ruleIdx,
            n->step,
            ie,
            &checker);
    std::stringstream ssOut;
    JSON::write(ssOut, out);
    std::string sOut = ssOut.str();
    return PyUnicode_FromString(sOut.c_str());
}

static PyObject* querier_get_list_predicates(PyObject* self, PyObject *args) {
    glog_Querier *s = (glog_Querier*)self;
    PyObject *obj = PyList_New(0);
    auto predIds = s->q->getListPredicates();
    for(auto pId : predIds) {
        auto value = PyUnicode_FromString(pId.c_str());
        PyList_Append(obj, value);
        Py_DECREF(value);
    }
    return obj;
}

static PyObject* querier_get_node_details_predicate(PyObject* self, PyObject *args) {
    const char *predName = NULL;
    if (!PyArg_ParseTuple(args, "s", &predName) || predName == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    glog_Querier *s = (glog_Querier*)self;
    auto out = ((glog_Querier*)self)->q->getNodeDetailsWithPredicate(
            std::string(predName));
    std::stringstream ssOut;
    JSON::write(ssOut, out);
    std::string sOut = ssOut.str();
    return PyUnicode_FromString(sOut.c_str());
}

static PyObject* querier_get_facts_in_TG_node(PyObject* self, PyObject *args) {
    size_t nodeId;
    if (!PyArg_ParseTuple(args, "l", &nodeId)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    auto out = ((glog_Querier*)self)->q->getNodeFacts(nodeId);
    std::stringstream ssOut;
    JSON::write(ssOut, out);
    std::string sOut = ssOut.str();
    return PyUnicode_FromString(sOut.c_str());
}

static PyObject* querier_get_fact_in_TupleSet(PyObject* self, PyObject *args) {
    PyObject *tupleSetObj = NULL;
    size_t factId = 0;
    if (!PyArg_ParseTuple(args, "Ol", &tupleSetObj, &factId)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    auto s = ((glog_Querier*)self);
    auto ts = ((glog_TupleSet*)tupleSetObj);
    if (factId >= ts->data->getNRows()) {
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        auto n_columns = ts->data->getNColumns();
        PyObject *outObj = PyList_New(0);
        for(size_t i = 0; i < n_columns; ++i) {
            Term_t c = ts->data->getValueAtRow(factId, i);
            //Convert into text
            std::string sTerm = s->q->getTermText(c);
            auto sTermObj = PyUnicode_FromString(sTerm.c_str());
            PyList_Append(outObj, sTermObj);
            Py_DECREF(sTermObj);
        }
        return outObj;
    }
}

static PyObject* querier_get_leaves(PyObject* self, PyObject *args) {
    size_t nodeId, factId;
    if (!PyArg_ParseTuple(args, "ll", &nodeId, &factId)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    std::vector<std::vector<Literal>> leaves;
    ((glog_Querier*)self)->q->getLeavesInDerivationTree(nodeId,
            factId, leaves);
    PyObject *outAll = PyList_New(0);
    for(auto &leafSet : leaves)
    {
        PyObject *outObj = PyList_New(0);
        for(auto &leaf : leafSet)
        {
            auto tuple = PyTuple_New(leaf.getTupleSize() + 1);
            PyTuple_SetItem(tuple, 0, PyLong_FromLong(leaf.getPredicate().getId()));
            for(size_t j = 0; j < leaf.getTupleSize(); ++j)
            {
                auto t = leaf.getTermAtPos(j);
                PyTuple_SetItem(tuple, 1 + j, PyLong_FromLong(t.getValue()));
            }
            PyList_Append(outObj, tuple);
            Py_DECREF(tuple);
        }
        PyList_Append(outAll, outObj);
        Py_DECREF(outObj);
    }
    return outAll;
}

PyObject* querier_get_all_facts(PyObject* self, PyObject *args)
{
    auto s = ((glog_Querier*)self);
    PyObject *out = PyDict_New();
    auto facts = s->q->getAllFacts();
    for(auto p : facts)
    {
        auto predName = PyUnicode_FromString(p.first.c_str());
        PyObject *predFacts = PyList_New(0);
        for(auto predFact : p.second)
        {
            PyObject *tuple = PyTuple_New(predFact.size());
            for (size_t i = 0; i < predFact.size(); ++i)
            {
                auto termFact = predFact[i];
                auto term = PyUnicode_FromString(termFact.c_str());
                PyTuple_SetItem(tuple, i, term);
            }
            PyList_Append(predFacts, tuple);
            Py_DECREF(tuple);
        }
        PyDict_SetItem(out, predName, predFacts);
        Py_DECREF(predFacts);
        Py_DECREF(predName);
    }
    return out;
}

PyObject* querier_get_predicate_name(PyObject* self, PyObject *args)
{
    size_t predicateId;
    if (!PyArg_ParseTuple(args, "l", &predicateId)) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    auto s = ((glog_Querier*)self);
    auto predicateNameStr = s->g->reasoner->program->program->
        getPredicateName(predicateId);
    auto predicateName = PyUnicode_FromString(predicateNameStr.c_str());
    return predicateName;
}

PyObject* querier_get_term_name(PyObject* self, PyObject *args)
{
    size_t termId;
    if (!PyArg_ParseTuple(args, "l", &termId)) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    auto s = ((glog_Querier*)self);
    auto termNameStr = s->g->reasoner->e->e->getDictText(termId);
    auto termName = PyUnicode_FromString(termNameStr.c_str());
    return termName;

}

static PyObject* querier_get_facts_coordinates_with_predicate(PyObject* self, PyObject *args)
{
    const char *predName = NULL;
    if (!PyArg_ParseTuple(args, "s", &predName) || predName == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject *out = PyList_New(0);
    auto s = ((glog_Querier*)self);
    auto pairs = s->q->getAllFactsPredicate(std::string(predName));
    auto card = pairs.second.size() / pairs.first.size();
    for(size_t i = 0; i < pairs.first.size(); ++i)
    {
        auto c = pairs.first[i];
        PyObject *fact = PyTuple_New(2);
        PyObject *tuple = PyTuple_New(card);
        for(size_t j = 0; j < card; ++j)
        {
            auto term = pairs.second[i * card + j];
            PyTuple_SetItem(tuple, j, PyLong_FromLong(term));
        }
        PyTuple_SetItem(fact, 0, tuple);
        //Py_DECREF(tuple);
        PyObject *coordinates = PyTuple_New(2);
        PyTuple_SetItem(coordinates, 0, PyLong_FromLong(c.first)); //nodeId
        PyTuple_SetItem(coordinates, 1, PyLong_FromLong(c.second)); //offset
        PyTuple_SetItem(fact, 1, coordinates);
        //Py_DECREF(coordinates);
        PyList_Append(out, fact);
        //Py_DECREF(fact);
    }
    return out;
}
