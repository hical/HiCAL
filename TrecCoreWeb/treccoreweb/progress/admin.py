from django.contrib import admin
from treccoreweb.progress.models import Demographic, Task, PreTask, PostTask


admin.site.register(Demographic)
admin.site.register(Task)
admin.site.register(PreTask)
admin.site.register(PostTask)
