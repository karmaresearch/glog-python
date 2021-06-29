#include <glog-python/pyedbtable.h>
#include <glog-python/pyedbiterator.h>

#include <vlog/concepts.h>
#include <string>

PyTable::PyTable(PredId_t predid,
        std::string predname,
        EDBLayer *layer,
        PyObject *obj)
    : predid(predid), layer(layer)
{
    Py_INCREF(obj);
    this->obj = obj;
    this->moduleName = PyUnicode_FromString("pyterm");
    this->mod = PyImport_Import(moduleName);
    this->termClass = PyObject_GetAttrString(this->mod, "PyTerm");
    this->sortedItrMethod = PyUnicode_FromString("get_sorted_iterator");
}

uint8_t PyTable::getArity() const
{
    uint8_t card = 0;
    auto resp = PyObject_CallMethod(this->obj, "arity", NULL);
    if (resp != NULL) {
        card = PyLong_AsLong(resp);
        Py_DECREF(resp);
    }
    return card;
}

void PyTable::query(QSQQuery *query, TupleTable *outputTable,
        std::vector<uint8_t> *posToFilter,
        std::vector<Term_t> *valuesToFilter)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

bool PyTable::isEmpty(const Literal &query, std::vector<uint8_t> *posToFilter,
        std::vector<Term_t> *valuesToFilter)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

size_t PyTable::getCardinalityColumn(const Literal &query, uint8_t posColumn)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

size_t PyTable::estimateCardinality(const Literal &query)
{
    return getCardinality(query);
}

size_t PyTable::getCardinality(const Literal &query)
{
    uint64_t out = 0;
    auto obj = convertLiteralIntoPyTuple(query);
    auto arglist = Py_BuildValue("(O)", obj);
    auto resp = PyObject_CallMethod(this->obj, "get_cardinality", "(O)",
            arglist);
    if (resp != NULL) {
        out = PyLong_AsLong(resp);
        Py_DECREF(resp);
    }
    Py_DECREF(arglist);
    Py_DECREF(obj);
    return out;
}

EDBIterator *PyTable::getIterator(const Literal &query)
{
    std::vector<uint8_t> fields;
    return getSortedIterator(query, fields);
}

EDBIterator *PyTable::getSortedIterator(const Literal &query,
        const std::vector<uint8_t> &fields)
{
    EDBIterator *itr = NULL;
    auto argFields = PyTuple_New(fields.size());
    for(size_t i = 0; i < fields.size(); ++i)
    {
        PyTuple_SetItem(argFields, i, PyLong_FromLong(fields[i]));
    }
    auto pQuery = convertLiteralIntoPyTuple(query);
    auto resp = PyObject_CallMethodObjArgs(this->obj, sortedItrMethod, pQuery, argFields, NULL);
    if (resp != NULL) {
        itr = new PyEDBIterator(predid, resp, layer);
    }
    Py_DECREF(argFields);
    Py_DECREF(pQuery);
    return itr;
}

bool PyTable::getDictNumber(const char *text, const size_t sizeText,
        uint64_t &id)
{
    return false;
}

bool PyTable::getDictText(const uint64_t id, char *text)
{
    return false;
}

bool PyTable::getDictText(const uint64_t id, std::string &text)
{
    return false;
}

uint64_t PyTable::getNTerms()
{
    uint64_t out = 0;
    auto resp = PyObject_CallMethod(this->obj, "get_n_terms", NULL);
    if (resp != NULL) {
        out = PyLong_AsLong(resp);
        Py_DECREF(resp);
    }
    return out;
}

void PyTable::releaseIterator(EDBIterator *itr)
{
    delete itr;
}

uint64_t PyTable::getSize()
{
    uint64_t out = 0;
    auto resp = PyObject_CallMethod(this->obj, "get_size", NULL);
    if (resp != NULL) {
        out = PyLong_AsLong(resp);
        Py_DECREF(resp);
    }
    return out;
}

PyObject *PyTable::convertLiteralIntoPyTuple(const Literal &lit)
{
    //Create a tuple of PyTerm
    auto arity = lit.getTupleSize();
    auto out = PyTuple_New(arity);
    for(size_t i = 0; i < arity; ++i) {
        auto t = lit.getTermAtPos(i);
        PyObject *arglist;
        if (t.isVariable()) {
            auto sId = std::to_string(t.getId());
            arglist = Py_BuildValue("(bs)", true, sId.c_str());
        } else {
            //Get the textual term
            std::string text = "";
            getDictText(t.getValue(), text);
            arglist = Py_BuildValue("(bs)", false, text.c_str());
        }
        auto a = PyObject_CallObject(termClass, arglist);
        PyTuple_SetItem(out, i, a);
    }
    return out;
}

PyTable::~PyTable()
{
    if (this->obj != NULL)
    {
        Py_DECREF(this->obj);
        this->obj = NULL;
    }
    if (this->moduleName != NULL)
    {
        Py_DECREF(this->moduleName);
        this->moduleName = NULL;
    }
    if (this->mod != NULL)
    {
        Py_DECREF(this->mod);
        this->mod = NULL;
    }
    if (this->termClass != NULL)
    {
        Py_DECREF(this->termClass);
        this->termClass = NULL;
    }
    if (this->sortedItrMethod != NULL)
    {
        Py_DECREF(this->sortedItrMethod);
        this->sortedItrMethod = NULL;
    }
}
