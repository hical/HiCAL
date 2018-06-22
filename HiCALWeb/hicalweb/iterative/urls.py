from django.conf.urls import url

from hicalweb.iterative import views

urlpatterns = [
    url(r'^$', views.HomePageView.as_view(), name='main'),

    # Ajax views
    url(r'^post_log/$', views.MessageAJAXView.as_view(), name='post_log_msg'),
    url(r'^get_docs/$', views.DocAJAXView.as_view(), name='get_docs'),
]
