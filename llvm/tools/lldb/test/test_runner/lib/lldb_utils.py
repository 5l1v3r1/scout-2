"""
The LLVM Compiler Infrastructure

This file is distributed under the University of Illinois Open Source
License. See LICENSE.TXT for details.

Provides classes used by the test results reporting infrastructure
within the LLDB test suite.


This module contains utilities used by the lldb test framwork.
"""


class OptionalWith(object):
    # pylint: disable=too-few-public-methods
    # This is a wrapper - it is not meant to provide any extra methods.
    """Provides a wrapper for objects supporting "with", allowing None.

    This lets a user use the "with object" syntax for resource usage
    (e.g. locks) even when the wrapped with object is None.

    e.g.

    wrapped_lock = OptionalWith(thread.Lock())
    with wrapped_lock:
        # Do something while the lock is obtained.
        pass

    might_be_none = None
    wrapped_none = OptionalWith(might_be_none)
    with wrapped_none:
        # This code here still works.
        pass

    This prevents having to write code like this when
    a lock is optional:

    if lock:
        lock.acquire()

    try:
        code_fragament_always_run()
    finally:
        if lock:
            lock.release()

    And I'd posit it is safer, as it becomes impossible to
    forget the try/finally using OptionalWith(), since
    the with syntax can be used.
    """
    def __init__(self, wrapped_object):
        self.wrapped_object = wrapped_object

    def __enter__(self):
        if self.wrapped_object is not None:
            return self.wrapped_object.__enter__()
        else:
            return self

    def __exit__(self, the_type, value, traceback):
        if self.wrapped_object is not None:
            return self.wrapped_object.__exit__(the_type, value, traceback)
        else:
            # Don't suppress any exceptions
            return False
