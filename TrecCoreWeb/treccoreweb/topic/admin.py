from django.contrib import admin
from treccoreweb.topic.models import Topic, Task, PreTask, PostTask


class TopicAdmin(admin.ModelAdmin):
    readonly_fields = ('seed_query', 'uuid')

    class Meta:
        model = Topic

    def get_readonly_fields(self, request, obj=None):
        if obj:  # obj is not None, so this is an edit
            return ['seed_query',]  # Return a list or tuple of readonly fields' names
        else:  # This is an addition
            return []

admin.site.register(Topic, TopicAdmin)
admin.site.register(Task)
admin.site.register(PreTask)
admin.site.register(PostTask)
