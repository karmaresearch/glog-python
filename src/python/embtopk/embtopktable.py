from pyiterator import PyIterator
from pytable import PyTable

import torch
import numpy as np
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
            # Round to three decimals
            v = "%.3f" % v
        return v


class EmbTopKEDBTable(PyTable):
    def __init__(self, predname, relname, top_k, edb_layer, model : KgeModel, entities_dict : dict = None,
                 known_answers_sp = None, known_answers_po = None, triples = None, so_map = None, score_threshold=None):
        super().__init__(predname, 3)
        self.predname = predname
        self.top_k = top_k
        assert(top_k > 0)
        self.edb_layer = edb_layer
        self.model = model
        self.n_terms = self.model.dataset.num_entities()
        self.rel_name = relname
        self.rel_id = None
        self.score_threshold = score_threshold
        for id, rel in enumerate(self.model.dataset.relation_ids()):
            if rel == relname:
                self.rel_id = id
        if self.rel_id is None:
            raise Exception("Relation not found")
        if entities_dict is not None:
            self.entities_dict = entities_dict
        else:
            print("Entity map created from scratch. Consider sharing it with other relations to save RAM!")
            self.entities_dict = {}
            for idx, e in enumerate(self.model.dataset.entity_strings()):
                self.entities_dict[e] = idx
        if known_answers_sp is None:
            self.known_answers_sp = model.dataset.index('train_sp_to_o')
        else:
            self.known_answers_sp = known_answers_sp
        if known_answers_po is None:
            self.known_answers_po = model.dataset.index('train_po_to_s')
        else:
            self.known_answers_po = known_answers_po
        if triples is None:
            triples = model.dataset.load_triples('train')
        if so_map is None:
            self.so = [(t[0].item(), t[2].item()) for t in triples if t[1].item() == self.rel_id]
        else:
            if self.rel_id not in so_map:
                self.so = []
            else:
                self.so = so_map[self.rel_id]
        if len(self.so) == 0:
            # Best possible scores
            self.known_min_score_s = 0
            self.known_min_score_o = 0
        else:
            # Calculate scores for the best best 'o'
            idx = np.random.randint(0, len(self.so), 1)[0]
            chosen_s = self.so[idx][0]
            s = torch.Tensor([chosen_s, ]).long()
            p = torch.Tensor([self.rel_id, ]).long()
            scores = self.model.score_sp(s, p)[0]
            o = torch.argsort(scores, descending=True)
            top_k_o = o[:self.top_k]
            top_k_scores = scores[top_k_o]
            self.known_min_score_o = top_k_scores[0]
            # Calculate scores for the best best 's'
            chosen_o = self.so[idx][1]
            o = torch.Tensor([chosen_o, ]).long()
            scores = self.model.score_po(p, o)[0]
            s = torch.argsort(scores, descending=True)
            top_k_s = s[:self.top_k]
            top_k_scores = scores[top_k_s]
            self.known_min_score_s = top_k_scores[0]

    def _is_query_allowed(self, t):
        l = len(t)
        if l != 3:
            return False
        s = t[0]
        o = t[1]
        score = t[2]

        if not s.is_variable() and not o.is_variable() and score.is_variable():
            return True

        if s.is_variable() and o.is_variable():
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

    def is_query_allowed(self, t: tuple) -> bool:
        return self._is_query_allowed(t)

    def get_cardinality(self, t: tuple) -> int:
        if not self._is_query_allowed(t):
            if t[0].is_variable() and t[1].is_variable() and t[2].is_variable() and t[0].get_value() != t[1].get_value() and t[1].get_value() != t[2].get_value():
                return self.n_terms * self.top_k
            else:
                print("EMBTopkTable: Tuple is not allowed!")
                raise Exception("Tuple is not allowed")
        elif not t[0].is_variable() and not t[1].is_variable():
            return 1
        return self.top_k

    def get_unique_values_in_column(self, t: tuple, column_nr) -> int:
        raise Exception("Not implemented!")

    def get_n_terms(self) -> int:
        self.n_terms + 1000 # I assume I return scores with three decimal digits of precision

    def _normalize_scores_simple(self, t : tuple, scores):
        sum = 0.0
        for s in scores:
            sum += 1 / s
        for idx, s in enumerate(scores):
            scores[idx] = 1 / (s.item()) / sum.item()
        return scores

    def _normalize_scores(self, t : tuple, scores):
        # Collect up to k max/min scores
        true_answers = self._get_true_answers(t)
        if len(true_answers) > 0:
            true_scores = self._get_answer_scores(t, true_answers)
        else:
            if t[0].is_variable():
                true_scores = self.known_min_score_s
            else:
                true_scores = self.known_min_score_o

        false_answers = self._get_false_answers(t)
        false_scores = self._get_answer_scores(t, false_answers)
        # Normalise "scores" depending on true_scores and false_scores
        best_score = torch.max(true_scores).item()
        assert(best_score <= 0)
        worst_score = torch.mean(false_scores).item()
        assert (worst_score <= 0)
        assert(best_score >= worst_score)
        clamped_scores = torch.clamp(scores, min=worst_score, max=best_score)
        scores = clamped_scores - worst_score
        range = best_score - worst_score
        prob = scores / range
        return prob

    def _get_answer_scores(self, t : tuple, a : torch.Tensor):
        if t[0].is_variable():
            # Retrieve ID of the object
            value = t[1].get_value()
            value_id = self.entities_dict[value]
            o = torch.Tensor([value_id, ]).long()
            p = torch.Tensor([self.rel_id, ]).long()
            scores = self.model.score_po(p, o, a)[0]
        #elif t[1].is_variable():
        else:
            # Retrieve ID of the subject
            value = t[0].get_value()
            value_id = self.entities_dict[value]
            s = torch.Tensor([value_id, ]).long()
            p = torch.Tensor([self.rel_id, ]).long()
            scores = self.model.score_sp(s, p, a)[0]
        #else:
        #    raise Exception("Not implemented")
        return scores

    def _get_true_answers(self, t : tuple):
        if t[0].is_variable():
            # Retrieve ID of the object
            value = t[1].get_value()
            value_id = self.entities_dict[value]
            o = torch.Tensor([value_id, ]).long().item()
            p = torch.Tensor([self.rel_id, ]).long().item()
            answers = self.known_answers_po.get([p, o])
        #elif t[1].is_variable():
        else:
            # Retrieve ID of the subject
            value = t[0].get_value()
            value_id = self.entities_dict[value]
            s = torch.Tensor([value_id, ]).long().item()
            p = torch.Tensor([self.rel_id, ]).long().item()
            answers = self.known_answers_sp.get([s, p])
        #else:
        #    raise Exception("Not implemented")
        return answers

    def _get_false_answers(self, t : tuple):
        random_entities = np.random.randint(0, self.n_terms, 10)
        return torch.from_numpy(random_entities)

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
            top_k_scores = self._normalize_scores(t, top_k_scores)
            if self.score_threshold is not None:
                # Filter values with low scores
                top_k_scores = top_k_scores.double()
                top_k_scores = torch.where(top_k_scores < self.score_threshold, 0., top_k_scores)
                retained_entities = torch.nonzero(top_k_scores, as_tuple=True)
                top_k_s = top_k_s[retained_entities]
                top_k_scores = top_k_scores[retained_entities]
            txt_top_k_s = self.model.dataset.entity_strings(top_k_s)
            for idx, e in enumerate(txt_top_k_s):
                answers.append((e, value, top_k_scores[idx].item()))
        elif t[1].is_variable():
            # Retrieve ID of the subject
            value = t[0].get_value()
            value_id = self.entities_dict[value]
            s = torch.Tensor([value_id,]).long()
            p = torch.Tensor([self.rel_id, ]).long()
            scores = self.model.score_sp(s, p)[0]
            o = torch.argsort(scores, descending=True)
            top_k_o = o[:self.top_k]
            top_k_scores = scores[top_k_o]
            top_k_scores = self._normalize_scores(t, top_k_scores)
            if self.score_threshold is not None:
                # Filter values with low scores
                top_k_scores = top_k_scores.double()
                top_k_scores = torch.where(top_k_scores < self.score_threshold, 0., top_k_scores)
                retained_entities = torch.nonzero(top_k_scores, as_tuple=True)
                top_k_o = top_k_o[retained_entities]
                top_k_scores = top_k_scores[retained_entities]
            txt_top_k_o = self.model.dataset.entity_strings(top_k_o)
            for idx, e in enumerate(txt_top_k_o):
                answers.append((value, e, top_k_scores[idx].item()))
        else:
            value_s = t[0].get_value()
            value_o = t[1].get_value()
            value_s_id = self.entities_dict[value_s]
            value_o_id = self.entities_dict[value_o]
            s = torch.Tensor([value_s_id, ]).long()
            o = torch.Tensor([value_o_id, ]).long()
            p = torch.Tensor([self.rel_id, ]).long()
            score = self.model.score_spo(s, p, o)
            score = self._normalize_scores(t, score)
            answers.append((value_s, value_o, score[0].item()))

        return EmbTopKEDBIterator(answers)