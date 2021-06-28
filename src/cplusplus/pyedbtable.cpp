#include <glog-python/pyedbtable.h>

PyTable::PyTable(PredId_t predid,
        std::string predname,
        EDBLayer *layer,
        std::string dictPredName,
        PyObject *obj)
    : predid(predid)
{
    Py_INCREF(obj);
    this->obj = obj;
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

uint8_t PyTable::getArity() const
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

void PyTable::query(QSQQuery *query, TupleTable *outputTable,
        std::vector<uint8_t> *posToFilter,
        std::vector<Term_t> *valuesToFilter)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

size_t PyTable::estimateCardinality(const Literal &query)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

size_t PyTable::getCardinality(const Literal &query)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

size_t PyTable::getCardinalityColumn(const Literal &query, uint8_t posColumn)
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

EDBIterator *PyTable::getIterator(const Literal &query)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

EDBIterator *PyTable::getSortedIterator(const Literal &query,
        const std::vector<uint8_t> &fields)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

bool PyTable::getDictNumber(const char *text, const size_t sizeText,
        uint64_t &id)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

bool PyTable::getDictText(const uint64_t id, char *text)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

bool PyTable::getDictText(const uint64_t id, std::string &text)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

uint64_t PyTable::getNTerms()
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

void PyTable::releaseIterator(EDBIterator *itr)
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

uint64_t PyTable::getSize()
{
    LOG(ERRORL) << "Method not implemented";
    throw 10;
}

PyTable::~PyTable()
{
    if (this->obj != NULL)
    {
        Py_DECREF(this->obj);
        this->obj = NULL;
    }
}
