
class Rule:
    def __init__(self, head_atoms, body_atoms):
        self.head_atoms = head_atoms
        self.body_atoms = body_atoms

    def str(self):
        out = ''
        for head_atom in self.head_atoms:
            out += head_atom.str() + ','
        out = out[:-1] + ' :- '
        for body_atom in self.body_atoms:
            out += body_atom.str() + ','
        out = out[:-1]
        return out