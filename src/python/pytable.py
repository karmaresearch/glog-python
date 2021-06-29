from .pytuple import PyTuple
from .pyiterator import PyIterator

class PyTable:
    def __init__(self, predname, arity):
        self._predname = predname
        self._arity = arity

    def arity(self) -> int:
        return self._arity

    def get_cardinality(self, tuple : PyTuple) -> int:
        pass

    def get_cardinality_column(self, tuple : PyTuple, column_nr) -> int:
        pass

    def get_n_terms(self) -> int:
        pass

    def get_size(self, tuple : PyTuple) -> int:
        pass

    def get_iterator(self, tuple : PyTuple) -> PyIterator:
        pass

    def get_sorted_iterator(self, tuple : PyTuple, sorting_fields) -> PyIterator:
        pass