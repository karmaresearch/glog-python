#ifndef _PYEDBTABLE_H
#define _PYEDBTABLE_H

#include <vlog/column.h>
#include <vlog/edbtable.h>
#include <vlog/edbiterator.h>
#include <vlog/segment.h>

#include <unordered_map>
#include <Python.h>

class PyTable : public EDBTable
{
    private:
        const PredId_t predid;
        EDBLayer *layer;

        PyObject *obj;
        PyObject *termClass;
        PyObject *moduleName;
        PyObject *mod;
        PyObject *getItrMethod;
        PyObject *getCardMethod;
        PyObject *isQuAllowedMethod;

        PyObject *convertLiteralIntoPyTuple(const Literal &lit);

    public:

        PyTable(PredId_t predid,
                std::string predname,
                EDBLayer *layer,
                PyObject *obj);

        uint8_t getArity() const;

        void query(QSQQuery *query, TupleTable *outputTable,
                std::vector<uint8_t> *posToFilter,
                std::vector<Term_t> *valuesToFilter);

        size_t estimateCardinality(const Literal &query);

        size_t getCardinality(const Literal &query);

        size_t getCardinalityColumn(const Literal &query, uint8_t posColumn);

        bool isEmpty(const Literal &query, std::vector<uint8_t> *posToFilter,
                std::vector<Term_t> *valuesToFilter);

        EDBIterator *getIterator(const Literal &query);

        EDBIterator *getSortedIterator(const Literal &query,
                const std::vector<uint8_t> &fields);

        bool getDictNumber(const char *text, const size_t sizeText,
                uint64_t &id);

        bool getDictText(const uint64_t id, char *text);

        bool getDictText(const uint64_t id, std::string &text);

        uint64_t getNTerms();

        void releaseIterator(EDBIterator *itr);

        bool acceptQueriesWithFreeVariables();

        uint64_t getSize();

        bool canChange();

        bool areTermsEncoded();

        bool isQueryAllowed(const Literal &query);

        ~PyTable();
};


#endif
