#include <glog-python/pyedbitr.h>

PyEDBIterator::PyEDBIterator(PredId_t predid, PyObject *obj,
        std::shared_ptr<EDBTable> dictTable) : predid(predid)
{
    this->obj = obj;
    this->dictTable = dictTable;
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
    auto arg = Py_BuildValue("(i)", p);
    auto sTerm = PyObject_CallMethod(this->obj, "get_term_at_pos", "(i)", arg);
    //Translate PyTerm into a Term_t
    Py_ssize_t size;
    const char *ptr = PyUnicode_AsUTF8AndSize(sTerm, &size);
    Term_t id = 0;
    dictTable->getDictNumber(ptr, size, id);
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
