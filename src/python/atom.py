class atom:
    def __init__(self, predicate_name, terms):
        self.predicate_name = predicate_name
        self.terms = terms

    def get_predicate_name(self):
        return self.predicate_name

    def get_terms(self):
        return self.terms

    def str(self):
        terms = ''
        for term in self.terms:
            terms += term + ','
        terms = terms[:-1]
        return predicate_name + '(' + terms + ')'