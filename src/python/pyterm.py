class PyTerm:
    def __init__(self, is_var, value):
        if is_var == 1:
            self._is_var = True
        else:
            self._is_var = False
        self._value = value

    def is_variable(self):
        return self._is_var

    def get_value(self):
        return self._value