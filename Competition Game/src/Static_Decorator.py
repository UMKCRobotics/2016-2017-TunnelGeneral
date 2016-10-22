# a decorator for function static variables in python
def static_vars(**kwargs):
    def decorate(func):
        for k in kwargs:
            setattr(func, k, kwargs[k])
        return func
    return decorate

"""
example usage:

@static_vars(my_static_variable=0)
def my_function_with_a_static_variable():
    my_function_with_a_static_variable.my_static_variable += 1
    print(my_function_with_a_static_variable.my_static_variable)

"""