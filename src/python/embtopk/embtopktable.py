from src.python.pyiterator import PyIterator
from src.python.pytable import PyTable
from src.python.pytuple import PyTuple


class EmbTopKEDBTable(PyTable):
    def __init__(self, predid, predname, n_terms, top_k, edb_layer):
        super().__init__(predid, 3)
        self.n_terms = n_terms
        self.predname
        self.top_k = top_k
        self.edb_layer = edb_layer

    def _is_query_allowed(self, tuple):
        if tuple.get_length() != 3:
            return False
        s = tuple.get_term_at_pos(0)
        o = tuple.get_term_at_pos(1)
        score = tuple.get_term_at_pos(2)
        if s.is_variable() and o.is_variable():
            return False
        if not s.is_variable() and not o.is_variable():
            return False
        if not score.is_variable():
            return False
        n_vars = 0
        unique_vars = set()
        if s.is_variable():
            unique_vars.add(s.get_value())
            n_vars += 1
        if o.is_variable():
            unique_vars.add(o.get_value())
            n_vars += 1
        if score.is_variable():
            unique_vars.add(score.get_value())
            n_vars += 1
        if len(unique_vars) != n_vars:
            return False
        return True

    def get_cardinality(self, tuple: PyTuple) -> int:
        if not self._is_query_allowed(tuple):
            raise Exception("Tuple is not allowed")
        return self.top_k

    def get_unique_values_in_column(self, tuple: PyTuple, column_nr) -> int:
        raise Exception("Not implemented!")

    def get_n_terms(self) -> int:
        self.n_terms + 1000 # I assume I return scores with three decimal digits of precision

    def get_iterator(self, tuple: PyTuple) -> PyIterator:
        if not self._is_query_allowed(tuple):
            raise Exception("Tuple is not allowed")
        # TODO: Compute the embeddings

        # TODO: Return the results with an iterator