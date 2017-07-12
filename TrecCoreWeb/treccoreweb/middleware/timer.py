from datetime import datetime
import time
INACTIVE = float(15)

def timer_middleware(get_response):
    def middleware(request):
        print(request.user)
        print(request.user.current_task)
        print(request.user._cur_task_active_time)
        print(request.user.cur_task_last_activity)
        # request.user.last_activity = None
        # request.user.cur_task_active_time = 0.0
        # request.user.save()

        cur_time = time.time()
        prev_time = request.user.cur_task_last_activity
        if prev_time == None:
            prev_time = cur_time
        request.user._cur_task_active_time += min(cur_time - prev_time, INACTIVE)
        request.user.cur_task_last_activity = cur_time
        request.user.save()

        response = get_response(request)
        return response

    return middleware
