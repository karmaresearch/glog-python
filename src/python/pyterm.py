class PyTerm:
    def __init__(self, is_var, value):
        self._is_var = is_var
        self._value = value

    def is_variable(self):
        return self._is_var

    def get_value(self):
        return self._value