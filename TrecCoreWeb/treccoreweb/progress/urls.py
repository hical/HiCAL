from django.conf.urls import url
from django.views.generic import TemplateView

from treccoreweb.progress import views

urlpatterns = [
    url(r'^$', views.Home.as_view(),
        name='home'),
    url(r'^demographic/$',
        views.DemographicCreateView.as_view(),
        name='demographic'),
    url(r'^tutorial/$',
        views.TutorialView.as_view(),
        name='tutorial'),
    url(r'^pretask/$',
        views.PretaskView.as_view(),
        name='pretask'),
    url(r'^posttask/$',
        views.PosttaskView.as_view(),
        name='posttask'),
    url(r'^completed/$',
        views.Completed.as_view(),
        name='completed'),
    url(r'^exit/$',
        views.ExitCreateView.as_view(),
        name='exit'),
    url(r'^tasks_completed/$',
        TemplateView.as_view(template_name="progress/task_completed.html"),
        name='tasks_completed'),

    # Ajax views
    url(r'^post_ctrlf/$',
        views.CtrlFAJAXView.as_view(),
        name='post_ctrlf'),
    url(r'^post_find_keystroke/$',
        views.FindKeystrokeAJAXView.as_view(),
        name='post_find_keystroke'),
    url(r'^post_visit/$',
        views.VisitAJAXView.as_view(),
        name='post_visit'),

]
