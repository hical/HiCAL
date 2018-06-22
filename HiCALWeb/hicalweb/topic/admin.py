from django.contrib import admin
from hicalweb.topic.models import Topic


class TopicAdmin(admin.ModelAdmin):
    readonly_fields = ('seed_query', )

    class Meta:
        model = Topic

    def has_add_permission(self, request):
        return False

    def has_delete_permission(self, request, obj=None):
        return False

    def get_readonly_fields(self, request, obj=None):
        if obj:  # obj is not None, so this is an edit
            return ['seed_query', ]  # Return a list or tuple of readonly fields' names
        else:  # This is an addition
            return []

admin.site.register(Topic, TopicAdmin)
