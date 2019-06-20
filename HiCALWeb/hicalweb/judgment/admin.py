from django.contrib import admin

from hicalweb.judgment.models import Judgment


class NoAddNoDeleteAdmin(admin.ModelAdmin):
    def has_add_permission(self, request):
        return False

    def has_delete_permission(self, request, obj=None):
        return False


admin.site.register(Judgment, NoAddNoDeleteAdmin)
