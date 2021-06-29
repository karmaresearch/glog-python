#include <glog-python/pyedbitr.h>

PyEDBIterator::PyEDBIterator(PredId_t predid, PyObject *obj,
        std::shared_ptr<EDBTable> dictTable) : predid(predid)
{
    this->obj = obj;
    this->dictTable = dictTable;
    this->getTermMethod = PyUnicode_FromString("get_term_at_pos");
}

bool PyEDBIterator::hasNext()
{
    auto resp = PyObject_CallMethod(this->obj, "has_next", NULL);
    bool out = (resp == Py_True);
    return out;
}

void PyEDBIterator::next()
{
    PyObject_CallMethod(this->obj, "next", NULL);
}

Term_t PyEDBIterator::getElementAt(const uint8_t p)
{
    auto sTerm = PyObject_CallMethodObjArgs(this->obj, this->getTermMethod, PyLong_FromLong(p), NULL);
    Py_DECREF(this->getTermMethod);

    //Translate PyTerm into a Term_t
    Py_ssize_t size = 0;
    const char *ptr = PyUnicode_AsUTF8AndSize(sTerm, &size);
    LOG(DEBUGL) << "Retrieved " << std::string(ptr, size);
    Term_t id = 0;
    bool resp = dictTable->getDictNumber(ptr, size, id);
    assert(resp);
    LOG(DEBUGL) << "Got ID=" << id;

    return id;
}

void PyEDBIterator::clear()
{
}

PyEDBIterator::~PyEDBIterator()
{
    if (obj != NULL)
    {
        Py_DECREF(obj);
        obj = NULL;
    }
}
