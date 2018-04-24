from django.contrib import admin

from treccoreweb.progress.models import Task


class NoAddNoDeleteAdmin(admin.ModelAdmin):
    def has_add_permission(self, request):
        return False

    def has_delete_permission(self, request, obj=None):
        return False

admin.site.register(Task, NoAddNoDeleteAdmin)
