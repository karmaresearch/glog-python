class PyTerm:
    def __init__(self, is_var, value, numerical_id, is_null):
        if is_var == 1:
            self._is_var = True
        else:
            self._is_var = False
        self._value = value
        self._numerical_id = numerical_id
        if is_null == 1:
            self._is_null = True
        else:
            self._is_null = False

    def is_variable(self):
        return self._is_var

    def get_value(self):
        return self._value

    def get_numerical_value(self):
        return self._numerical_id

    def is_null_value(self):
        return self._is_null

    def __str__(self):
        if self._is_var:
            return "?" + str(self._value)
        elif self._is_null:
            return "NULL_" + str(self._numerical_id)
        else:
            return "CONST_" + str(self._numerical_id)