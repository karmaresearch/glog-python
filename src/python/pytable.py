from pyiterator import PyIterator
from abc import ABC, abstractmethod

class PyTable(ABC):
    def __init__(self, predname, arity):
        self._predname = predname
        self._arity = arity

    def arity(self) -> int:
        return self._arity

    @abstractmethod
    def get_cardinality(self, t: tuple) -> int:
        pass

    @abstractmethod
    def get_unique_values_in_column(self, t: tuple, column_nr) -> int:
        pass

    @abstractmethod
    def get_n_terms(self) -> int:
        pass

    @abstractmethod
    def get_iterator(self, t: tuple) -> PyIterator:
        pass