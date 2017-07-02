from django.contrib import admin
from treccoreweb.progress.models import Demographic, Task, TaskSetting, PreTask, PostTask


class TaskSettingAdmin(admin.ModelAdmin):
    def has_add_permission(self, request):
        return False

admin.site.register(TaskSetting, TaskSettingAdmin)
admin.site.register(Demographic)
admin.site.register(Task)
admin.site.register(PreTask)
admin.site.register(PostTask)
