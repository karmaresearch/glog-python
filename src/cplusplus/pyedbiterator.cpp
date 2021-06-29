#include <glog-python/pyedbiterator.h>

PyEDBIterator::PyEDBIterator(PredId_t predid, PyObject *obj, EDBLayer *layer) : predid(predid)
{
    this->obj = obj;
    this->layer = layer;
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
    //Translate PyTerm into a Term_t
    Py_ssize_t size = 0;
    const char *ptr = PyUnicode_AsUTF8AndSize(sTerm, &size);
    LOG(DEBUGL) << "Retrieved " << std::string(ptr, size);
    Term_t id = 0;
    bool resp = layer->getDictNumber(ptr, size, id);
    assert(resp);
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
    if (getTermMethod != NULL)
    {
        Py_DECREF(getTermMethod);
        getTermMethod = NULL;
    }
}
