Logging
=======

We use python's built in logging module for system and analytics logging.
You can configure logging from the project settings in `settings/base.py`.

The repo currently has one logging handle, TODO, saved in `logs/web.log`.




Using the logger
^^^^^^^^^^^^^^^^
Once you have configured your logger, you can start placing logging calls into your code.
This is very simple to do. Here’s an example:


.. code-block:: python

    import logging
    logger = logging.getLogger(__name__)

    class JudgmentAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                           views.JsonRequestResponseMixin,
                           generic.View):

        # some code
        ...

        # log message
        log_message = {
            "user": self.request.user.username,
            "client_time": client_time,
             # ...
            }
        }
        # log message
        logger.info("{}".format(json.dumps(log_message)))


We suggest you follow a simple logging style that you can easily parse and understand.
Place logging message where ever you need to log/track changes in the systems or user behaviour.
