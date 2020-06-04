from django.urls import path

from hicalweb.CAL import views

app_name = "CAL"

urlpatterns = [
    path('', views.CALHomePageView.as_view(),
         name='main'),

    # Ajax views
    path('post_log/', views.CALMessageAJAXView.as_view(),
         name='post_log_msg'),
    path('get_docs/', views.DocAJAXView.as_view(),
         name='get_docs'),
]
