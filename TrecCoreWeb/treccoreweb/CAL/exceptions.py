
class CALError(Exception):
    """
    Base class for all CAL exceptions.

    """
    pass


class CALServerError(CALError):
    """
    CAL Server error

    """
    def __init__(self, status_code, msg=None):
        if msg is None:
            msg = "An error occurred with CAL server. " \
                  "Returned status code {}.".format(status_code)
        super(CALServerError, self).__init__(msg)
