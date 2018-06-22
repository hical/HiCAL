from django.conf.urls import url

from hicalweb.topic import views

urlpatterns = [
    url(r'^(?P<pk>[\w.@+-]+)$', views.TopicView.as_view(), name='detail'),
    url(r'^create/$', views.TopicCreateView.as_view(), name='create'),
    url(r'^list/$', views.TopicListView.as_view(), name='list'),
    url(r'^activate/$', views.TopicActivateView.as_view(), name='activate'),
    url(r'^delete/$', views.TopicDeleteView.as_view(), name='delete'),
]
