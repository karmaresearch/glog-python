from rule import Rule
from atom import Atom

class Parser:
    def __init__(self):
        pass

    def parse_atom(self, atom_str):
        start_terms = atom_str.index('(')
        predicate_name = atom_str[:start_terms]
        end_terms = atom_str.index(')')
        terms_str = atom_str[start_terms + 1 : end_terms]
        terms = terms_str.split(',')
        return Atom(predicate_name, terms)

    def parse_conjunction_atoms(self, cnj_str):
        out = []
        while cnj_str is not None and cnj_str != '':
            atom, cnj_str = parse_atom(cnj_str)
            cnj_str = cnj_str.strip()
            if cnj_str.startswith(','):
                cnj_str = cnj_str[1:]
        return out

    def parse_rule(self, rule_str):
        head, body = rule_str.split(':-')
        head = head.strip()
        body = body.str()
        head_atoms = parse_conjunction_atoms(head)
        body_atoms = parse_conjunction_atoms(body)
        return Rule(head_atoms, body_atoms)