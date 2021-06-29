#ifndef _PYEDBITR_H
#define _PYEDBITR_H

#include <vlog/column.h>
#include <vlog/edbtable.h>
#include <vlog/edbiterator.h>
#include <vlog/segment.h>

#include <Python.h>

class PyEDBIterator : public EDBIterator
{
    private:
        const PredId_t predid;
        PyObject *obj;
        PyObject *getTermMethod;
        std::shared_ptr<EDBTable> dictTable;

    public:
        PyEDBIterator(PredId_t predid, PyObject *obj,
                std::shared_ptr<EDBTable> dictTable);

        bool hasNext();

        void next();

        Term_t getElementAt(const uint8_t p);

        void clear();

        PredId_t getPredicateID() {
            return predid;
        }

        void skipDuplicatedFirstColumn() {
        }

        ~PyEDBIterator();
};

#endif
