from django.contrib import admin

from treccoreweb.progress.models import Demographic
from treccoreweb.progress.models import PostTask
from treccoreweb.progress.models import PreTask
from treccoreweb.progress.models import ExitTask
from treccoreweb.progress.models import Task
from treccoreweb.progress.models import TaskSetting


class TaskSettingAdmin(admin.ModelAdmin):
    def has_add_permission(self, request):
        return False

admin.site.register(TaskSetting, TaskSettingAdmin)
admin.site.register(Demographic)
admin.site.register(Task)
admin.site.register(PreTask)
admin.site.register(PostTask)
admin.site.register(ExitTask)
