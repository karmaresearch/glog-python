from pyiterator import PyIterator
from pytable import PyTable

import torch
from kge.model import KgeModel

class EmbTopKEDBIterator(PyIterator):
    def __init__(self, data):
        super().__init__()
        self.idx = -1
        self.data = data

    def has_next(self) -> bool:
        return (self.idx + 1) < len(self.data)

    def next(self):
        self.idx += 1

    def get_term_at_pos(self, pos) -> str:
        v = self.data[self.idx][pos]
        if pos == 2:
            v = str(v)
        return v


class EmbTopKEDBTable(PyTable):
    def __init__(self, predname, relname, top_k, edb_layer, model : KgeModel):
        super().__init__(predname, 3)
        self.predname = predname
        self.top_k = top_k
        self.edb_layer = edb_layer
        self.model = model
        self.n_terms = self.model.dataset.num_entities()
        self.rel_name = relname
        self.rel_id = None
        for id, rel in enumerate(self.model.dataset.relation_ids()):
            if rel == relname:
                self.rel_id = id
        if self.rel_id is None:
            raise Exception("Relation not found")
        # Load the entities
        self.entities_dict = {}
        for idx, e in enumerate(self.model.dataset.entity_strings()):
            self.entities_dict[e] = idx

    def _is_query_allowed(self, t):
        l = len(t)
        if l != 3:
            return False
        s = t[0]
        o = t[1]
        score = t[2]
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

    def get_cardinality(self, t: tuple) -> int:
        if not self._is_query_allowed(t):
            print("EMBTopkTable: Tuple is not allowed!")
            raise Exception("Tuple is not allowed")
        return self.top_k

    def get_unique_values_in_column(self, t: tuple, column_nr) -> int:
        raise Exception("Not implemented!")

    def get_n_terms(self) -> int:
        self.n_terms + 1000 # I assume I return scores with three decimal digits of precision

    def _normalize_scores(self, scores):
        sum = 0.0
        for s in scores:
            sum += 1 / s
        for idx, s in enumerate(scores):
            scores[idx] = 1 / (s.item()) / sum.item()
            scores[idx] = i
        return scores

    def get_iterator(self, t: tuple) -> PyIterator:
        if not self._is_query_allowed(t):
            print("EMBTopkTable: Tuple is not allowed!")
            raise Exception("Tuple is not allowed")

        answers = []
        if t[0].is_variable():
            # Retrieve ID of the object
            value = t[1].get_value()
            value_id = self.entities_dict[value]
            o = torch.Tensor([value_id, ]).long()
            p = torch.Tensor([self.rel_id, ]).long()
            scores = self.model.score_po(p, o)[0]
            s = torch.argsort(scores, descending=True)
            top_k_s = s[:self.top_k]
            top_k_scores = scores[top_k_s]
            top_k_scores = self._normalize_scores(top_k_scores)
            txt_top_k_s = self.model.dataset.entity_strings(top_k_s)
            for idx, e in enumerate(txt_top_k_s):
                answers.append((e, value, top_k_scores[idx].item()))
        else:
            # Retrieve ID of the subject
            value = t[0].get_value()
            value_id = self.entities_dict[value]
            s = torch.Tensor([value_id,]).long()
            p = torch.Tensor([self.rel_id, ]).long()
            scores = self.model.score_sp(s, p)[0]
            o = torch.argsort(scores, descending=True)
            top_k_o = o[:self.top_k]
            top_k_scores = scores[top_k_o]
            top_k_scores = self._normalize_scores(top_k_scores)
            txt_top_k_o = self.model.dataset.entity_strings(top_k_o)
            for idx, e in enumerate(txt_top_k_o):
                answers.append((value, e, top_k_scores[idx].item()))
        return EmbTopKEDBIterator(answers)