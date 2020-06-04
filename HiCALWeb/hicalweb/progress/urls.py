from django.conf.urls import url

from hicalweb.progress import views

urlpatterns = [
    url(r'^$', views.Home.as_view(),
        name='home'),
    url(r'^sessions/$', views.SessionListView.as_view(),
        name='sessions'),
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

    url(r'^get_session_details/$',
        views.SessionDetailsAJAXView.as_view(),
        name='get_session_details'),
]
