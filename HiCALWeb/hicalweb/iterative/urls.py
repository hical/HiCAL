from django.urls import path

from hicalweb.iterative import views

app_name = "iterative"

urlpatterns = [
    path('', views.HomePageView.as_view(),
         name='main'),

    # Ajax views
    path('post_log/', views.MessageAJAXView.as_view(),
         name='post_log_msg'),
    path('get_docs/', views.DocAJAXView.as_view(),
         name='get_docs'),
]
