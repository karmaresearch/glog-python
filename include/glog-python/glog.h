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


#ifndef _GLOG_PYTHON_H
#define _GLOG_PYTHON_H

#include <Python.h>

#include <glog/gbchase.h>
#include <glog/gbgraph.h>
#include <glog/gbquerier.h>

#include <vlog/edb.h>
#include <vlog/concepts.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

typedef struct {
    PyObject_HEAD
        EDBConf *conf;
    EDBLayer *e;
} glog_EDBLayer;

typedef struct {
    PyObject_HEAD
        glog_EDBLayer *e;
    std::shared_ptr<Program> program;
} glog_Program;

typedef struct {
    PyObject_HEAD
        glog_EDBLayer *e;
    glog_Program *program;
    std::shared_ptr<GBChase> sn;
} glog_Reasoner;

typedef struct {
    PyObject_HEAD
} glog_Wizard;

typedef struct {
    PyObject_HEAD
        GBGraph *g;
    glog_Reasoner *reasoner;
} glog_TG;

typedef struct {
    PyObject_HEAD
        glog_TG *g;
    std::shared_ptr<GBQuerier> q;
} glog_Querier;

typedef struct {
    PyObject_HEAD
        std::shared_ptr<const TGSegment> data;
    size_t nodeId;
    size_t ruleIdx;
    size_t step;
    PredId_t predId;
    std::vector<std::vector<size_t>> nodes;
} glog_TupleSet;

#endif
