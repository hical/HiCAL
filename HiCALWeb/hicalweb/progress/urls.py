from django.urls import path

from hicalweb.progress import views

app_name = "progress"

urlpatterns = [
    path('', views.Home.as_view(),
         name='home'),
    path('sessions/', views.SessionListView.as_view(),
         name='sessions'),
    path('practice/',
         views.PracticeView.as_view(),
         name='practice'),
    path('practice_complete/', views.PracticeCompleteView.as_view(),
         name='practice_complete'),

    # Ajax views
    path('post_ctrlf/', views.CtrlFAJAXView.as_view(),
         name='post_ctrlf'),
    path('post_find_keystroke/', views.FindKeystrokeAJAXView.as_view(),
         name='post_find_keystroke'),
    path('post_visit/', views.VisitAJAXView.as_view(),
         name='post_visit'),
    path('post_log/', views.MessageAJAXView.as_view(),
         name='post_log_msg'),

    path('get_session_details/', views.SessionDetailsAJAXView.as_view(),
         name='get_session_details'),
]
