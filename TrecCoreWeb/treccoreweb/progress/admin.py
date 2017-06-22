from django.contrib import admin
from treccoreweb.progress.models import Demographic, Task, TaskSetting, PreTask, PostTask


admin.site.register(TaskSetting)
admin.site.register(Demographic)
admin.site.register(Task)
admin.site.register(PreTask)
admin.site.register(PostTask)
