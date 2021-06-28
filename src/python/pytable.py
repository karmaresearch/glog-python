from pytuple import PyTuple

class PyTable:
    def __init__(self, predname, arity):
        self._predname = predname
        self._arity = arity

    def arity(self):
        return self._arity

    def get_cardinality(self, tuple : PyTuple):
        pass

    def get_cardinality_column(self):
        pass

    def get_n_terms(self):
        pass

    def get_size(self):
        pass

    def get_iterator(self, tuple : PyTuple):
        pass

    def get_sorted_iterator(self, tuple : PyTuple):
        pass