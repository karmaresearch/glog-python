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
#include <vlog/reasoner.h>
#include <vlog/utils.h>
#include <kognac/utils.h>

/*** Methods ***/
static PyObject * reasoner_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int reasoner_init(glog_Reasoner *self, PyObject *args, PyObject *kwds);
static void reasoner_dealloc(glog_Reasoner* self);
static PyObject *reasoner_create_model(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *reasoner_execute_rule(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *reasoner_get_TG(PyObject *self, PyObject *args);

static PyMethodDef Reasoner_methods[] = {
    {"create_model", (PyCFunction)reasoner_create_model, METH_VARARGS | METH_KEYWORDS, "Create a model." },
    {"execute_rule", (PyCFunction)reasoner_execute_rule, METH_VARARGS | METH_KEYWORDS, "Execute a rule." },
    {"get_TG", reasoner_get_TG, METH_VARARGS, "Get the TG associated with the model." },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyTypeObject glog_ReasonerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        "glog.Reasoner",             /* tp_name */
    sizeof(glog_Reasoner),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor) reasoner_dealloc, /* tp_dealloc */
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
    "Reasoner",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Reasoner_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)reasoner_init,      /* tp_init */
    0,                         /* tp_alloc */
    reasoner_new,                 /* tp_new */
};

static PyObject * reasoner_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    glog_Reasoner *self;
    self = (glog_Reasoner*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int reasoner_init(glog_Reasoner *self, PyObject *args, PyObject *kwds)
{
    //Params and default values
    const char *typeChase = NULL;
    PyObject *edbLayer = NULL;
    PyObject *program = NULL;
    bool queryCont = true;
    bool edbCheck = true;
    bool rewriteCliques = true;
    bool delProofs = true;
    const char *tgpath = "";
    const char *typeProv = "NOPROV";
    static char *kwlist[] = {
        (char *)"typeChase",
        (char *)"edbLayer",
        (char *)"program",
        (char *)"queryCont",
        (char *)"edbCheck",
        (char *)"rewriteCliques",
        (char *)"tgpath",
        (char *)"typeProv",
        (char *)"delProofs",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sOO|bbbssb",
                kwlist,
                &typeChase,
                &edbLayer,
                &program,
                &queryCont,
                &edbCheck,
                &rewriteCliques,
                &tgpath,
                &typeProv,
                &delProofs)) {
        return -1;
    }

    if (edbLayer == NULL || strcmp(edbLayer->ob_type->tp_name, "glog.EDBLayer") != 0)
        return -1;

    if (program == NULL || strcmp(program->ob_type->tp_name, "glog.Program") != 0)
        return -1;

    if (typeChase == NULL) {
        return -1;
    }
    std::string tChase = std::string(typeChase);

    GBChaseAlgorithm tc = GBChaseAlgorithm::GBCHASE;
    auto tp = std::string(typeProv);
    if (tChase == "tgchase_static") {
        tc = GBChaseAlgorithm::TGCHASE_STATIC;
    } else if (tChase == "tgchase") {
        if (tp == "NOPROV") {
            tc = GBChaseAlgorithm::GBCHASE;
        } else if (tp == "NODEPROV") {
            tc = GBChaseAlgorithm::TGCHASE_DYNAMIC;
        } else if (tp == "FULLPROV") {
            tc = GBChaseAlgorithm::TGCHASE_DYNAMIC_FULLPROV;
        } else {
            return -1;
        }
    } else if (tChase == "probtgchase") {
        tc = GBChaseAlgorithm::PROBTGCHASE;
        tp = "FULLPROV";
    }

    if (tp == "FULLPROV") {
        rewriteCliques = false;
    }


    LOG(INFOL) << "Reasoner created with parameters "
        "queryCont=" << queryCont << " "
        "edbCheck=" << edbCheck << " "
        "rewriteCliques=" << rewriteCliques << " "
        "optDelProofs=" << delProofs << " "
        "tgpath=" << tgpath << " "
        "typeProv=" << typeProv;

    self->e = (glog_EDBLayer*)edbLayer;
    self->program = (glog_Program*)program;
    Py_INCREF(edbLayer);
    Py_INCREF(program);
    EDBLayer *db = ((glog_EDBLayer*)edbLayer)->e;
    auto p = ((glog_Program*)program)->program;
    if (tc != GBChaseAlgorithm::PROBTGCHASE) {
        self->sn = Reasoner::getGBChase(*db, p.get(), tc,
                queryCont,
                edbCheck,
                rewriteCliques,
                std::string(tgpath));
    } else {
        self->sn = Reasoner::getProbTGChase(*db, p.get(), delProofs);
    }
    return 0;
}

static void reasoner_dealloc(glog_Reasoner* self)
{
    if (self->e != NULL) {
        Py_DECREF(self->e);
    }
    if (self->program != NULL) {
        Py_DECREF(self->program);
    }
    self->sn = NULL;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *reasoner_create_model(PyObject *self, PyObject *args, PyObject *kw)
{
    size_t startStep = 0;
    size_t maxStep = ~0ul;
    static char *kwlist[] = {(char*)"startStep", (char*)"maxStep", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|ll", kwlist,
                &startStep, &maxStep)) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    glog_Reasoner *s = (glog_Reasoner*)self;
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    s->sn->prepareRun(startStep, maxStep);
    s->sn->run();
    std::chrono::duration<double> secMat = std::chrono::system_clock::now() - start;
    LOG(INFOL) << "Runtime materialization = " << secMat.count() * 1000 << " milliseconds";
    LOG(INFOL) << "Derived tuples = " << s->sn->getNDerivedFacts();
    LOG(INFOL) << "N. nodes = " << s->sn->getNnodes();
    LOG(INFOL) << "N. edges = " << s->sn->getNedges();
    LOG(INFOL) << "Triggers = " << s->sn->getNTriggers();

    std::stringstream ssOut;
    JSON out;
    out.put("n_nodes", s->sn->getNnodes());
    out.put("n_edges", s->sn->getNedges());
    out.put("n_triggers", s->sn->getNTriggers());
    out.put("n_derivations", s->sn->getNDerivedFacts());
    out.put("steps", s->sn->getNSteps());
    out.put("max_mem_mb", Utils::get_max_mem());
    out.put("runtime_ms", secMat.count() * 1000);
    JSON::write(ssOut, out);
    std::string sOut = ssOut.str();
    return PyUnicode_FromString(sOut.c_str());
}

extern PyTypeObject glog_TupleSetType;
static PyObject *reasoner_execute_rule(PyObject *self, PyObject *args, PyObject *kwds)
{
    size_t ruleIdx = 0;
    if (!PyArg_ParseTuple(args, "l", &ruleIdx)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    glog_Reasoner *s = (glog_Reasoner*)self;
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    std::vector<GBRuleOutput> out = s->sn->executeRule(ruleIdx);
    std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
    if (out.empty()) {
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        assert(out.size() == 1);
        auto arglist = Py_BuildValue("()");
        PyObject *obj = PyObject_CallObject((PyObject *) &glog_TupleSetType, arglist);
        glog_TupleSet *tupleset = (glog_TupleSet *) obj;
        tupleset->ruleIdx = ruleIdx;
        tupleset->data = out[0].segment;
        tupleset->nodeId = ~0ul;
        tupleset->step = ~0ul;
        const Rule &rule = s->program->program->getRule(ruleIdx);
        auto &heads = rule.getHeads();
        tupleset->predId = heads[0].getPredicate().getId();

        auto& inNodes = out[0].nodes;
        size_t nnodes = inNodes.size();
        if (nnodes == 0) {
            auto body = rule.getBody();
            if (body.size() > 1) {
                LOG(ERRORL) << "The number of nodes must be > 0";
                throw 10;
            } else if (body[0].getPredicate().getType() != EDB) {
                auto itr = tupleset->data->iterator();
                while (itr->hasNext()) {
                    itr->next();
                    tupleset->nodes.emplace_back();
                    tupleset->nodes.back().push_back(itr->getNodeId());
                }
            }
        } else {
            //Fix the provenance
            if (nnodes > 2) {
                const std::vector<std::shared_ptr<Column>> &provenance = out[0].nodes;
                const size_t nrows = out[0].segment->getNRows();
                std::vector<size_t> provnodes = GBGraph::postprocessProvenance(
                        out[0].segment, provenance, nrows);
                const auto nnodes = (provenance.size() + 2) / 2;
                assert(provnodes.size() == nnodes * nrows);
                for(size_t i = 0; i < nrows; ++i) {
                    tupleset->nodes.emplace_back();
                    for(size_t j = 0; j < nnodes; ++j) {
                        tupleset->nodes.back().push_back(
                                provnodes[i * nnodes + j]);
                    }
                }
            } else {
                std::vector<std::unique_ptr<ColumnReader>> readers;
                for(size_t i = 0; i < nnodes; ++i) {
                    readers.push_back(inNodes[i]->getReader());
                }
                //Prepare the nodes
                while(readers[0]->hasNext()) {
                    for(size_t j = 1; j < nnodes; ++j) {
                        if (!readers[j]->hasNext()) {
                            throw 10;
                        }
                    }
                    tupleset->nodes.emplace_back();
                    for(size_t j = 0; j < nnodes; ++j) {
                        tupleset->nodes.back().push_back(readers[j]->next());
                    }
                }
            }
        }

        PyObject *outObj = PyList_New(0);
        PyList_Append(outObj, obj);
        Py_DECREF(obj);

        //Statistics
        PyObject *stats = PyDict_New();
        assert(stats != NULL);
        int resp = PyDict_SetItem(stats, PyUnicode_FromString("n_answers"),
                PyLong_FromLong(out[0].segment->getNRows()));
        assert(resp == 0);
        resp = PyDict_SetItem(stats, PyUnicode_FromString("runtime_ms"),
                PyFloat_FromDouble(dur.count() * 1000));
        assert(resp == 0);

        PyList_Append(outObj, stats);
        Py_DECREF(stats);
        return outObj;
    }
}

extern PyTypeObject glog_TGType;
static PyObject *reasoner_get_TG(PyObject *self, PyObject *args) {
    auto arglist = Py_BuildValue("(O)", (PyObject*)self);
    PyObject *obj = PyObject_CallObject((PyObject *) &glog_TGType, arglist);
    Py_DECREF(arglist);
    return obj;
}
