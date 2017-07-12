import time
from datetime import datetime
from django.http import HttpResponseRedirect
from django.urls import reverse_lazy
from config.settings.base import INACTIVE_TRIGGER_TIME

REDIRECT_EXCEPTION = [
    reverse_lazy('progress:completed'),
    reverse_lazy('progress:posttask')
]


def timer_middleware(get_response):
    def middleware(request):
        cur_time = time.time()
        prev_time = request.user.current_task.last_activity
        if prev_time == None:
            prev_time = cur_time
        request.user.current_task.timespent += min(cur_time - prev_time, INACTIVE_TRIGGER_TIME)
        request.user.current_task.last_activity = cur_time
        request.user.current_task.save()

        if request.user.current_task.is_time_past() and request.path not in REDIRECT_EXCEPTION:
            return HttpResponseRedirect(reverse_lazy('progress:completed'))

        response = get_response(request)
        return response

    return middleware
