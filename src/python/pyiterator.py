from pyterm import PyTerm
from abc import ABC, abstractmethod

class PyIterator(ABC):
    def __init__(self):
        pass

    @abstractmethod
    def has_next(self) -> bool:
        pass

    @abstractmethod
    def next(self):
        pass

    @abstractmethod
    def get_term_at_pos(self, pos) -> str:
        pass