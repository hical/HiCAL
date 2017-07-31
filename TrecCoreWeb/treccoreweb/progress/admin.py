from django.contrib import admin

from treccoreweb.progress.models import Demographic
from treccoreweb.progress.models import PostTask
from treccoreweb.progress.models import PreTask
from treccoreweb.progress.models import ExitTask
from treccoreweb.progress.models import Task
from treccoreweb.progress.models import TaskSetting


class NoAddNoDeleteAdmin(admin.ModelAdmin):
    def has_add_permission(self, request):
        return False

    def has_delete_permission(self, request, obj=None):
        return False

admin.site.register(TaskSetting, NoAddNoDeleteAdmin)
admin.site.register(Demographic)
admin.site.register(Task, NoAddNoDeleteAdmin)
admin.site.register(PreTask, NoAddNoDeleteAdmin)
admin.site.register(PostTask, NoAddNoDeleteAdmin)
admin.site.register(ExitTask)
