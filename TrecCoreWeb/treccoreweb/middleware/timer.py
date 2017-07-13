import time
from django.http import HttpResponseRedirect
from django.urls import reverse, reverse_lazy
from config.settings.base import INACTIVE_TRIGGER_TIME

ACTIVE_URLS = [
    reverse('CAL:post_log_msg'),
    reverse('CAL:get_docs'),
    reverse('CAL:main'),
    reverse('iterative:post_log_msg'),
    reverse('iterative:get_docs'),
    reverse('judgment:post_judgment'),
    reverse('judgment:post_nojudgment'),
    reverse('progress:post_ctrlf'),
    reverse('progress:post_find_keystroke'),
    reverse('search:main'),
    reverse('search:get_docs'),
    reverse('search:get_doc'),
    reverse('search:post_search_status'),
    reverse('search:post_keystroke')
]


def timer_middleware(get_response):
    def middleware(request):
        for url_exception in ACTIVE_URLS:
            if request.path.startswith(url_exception):
                cur_time = time.time()
                prev_time = request.user.current_task.last_activity
                if prev_time is None:
                    prev_time = cur_time
                request.user.current_task.timespent += min(cur_time - prev_time, INACTIVE_TRIGGER_TIME)
                request.user.current_task.last_activity = cur_time
                request.user.current_task.save()

                if request.user.current_task.is_time_past():
                    response = HttpResponseRedirect(reverse_lazy('progress:completed'))
                    return response
                break

        response = get_response(request)
        if request.path.startswith(reverse('progress:completed')):
            response['completed'] = 1
        return response

    return middleware
