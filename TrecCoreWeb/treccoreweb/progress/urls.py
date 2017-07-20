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
        views.TasksCompletedView.as_view(),
        name='tasks_completed'),
    url(r'^practice/$',
        views.PracticeView.as_view(),
        name='practice'),
    url(r'^practice_complete/$',
        views.PracticeCompleteView.as_view(),
        name='practice_complete'),

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
    url(r'^post_log/$',
        views.MessageAJAXView.as_view(),
        name='post_log_msg'),


]
