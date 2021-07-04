from pyterm import PyTerm

class PyTuple:
    def __init__(self, tuple):
        self._tuple = tuple
        if tuple is None:
            self._length = 0
        else:
            self._length = len(tuple)

    def get_length(self) -> int:
        return self._length

    def get_term_at_pos(self, pos) -> PyTerm:
        return self._tuple[pos]