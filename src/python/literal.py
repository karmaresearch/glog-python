import copy

class Literal:
    def __init__(self, predicate_name, terms, is_negated = False):
        self.predicate_name = predicate_name
        self.terms = terms
        self._is_negated = is_negated

    def get_predicate_name(self):
        return self.predicate_name

    def get_terms(self):
        return copy.deepcopy(self.terms)

    def is_negated(self):
        return self._is_negated

    def str(self):
        terms = ''
        for term in self.terms:
            terms += term + ','
        terms = terms[:-1]
        if self.is_negated():
            return '~' + self.predicate_name + '(' + terms + ')'
        else:
            return self.predicate_name + '(' + terms + ')'

    def __str__(self):
        return self.str()