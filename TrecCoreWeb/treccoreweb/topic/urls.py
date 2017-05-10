from django.conf.urls import url
from django.views.generic import TemplateView

from treccoreweb.topic import views

urlpatterns = [
    url(r'^(?P<pk>[\w.@+-]+)$', views.TopicView.as_view(), name='detail'),
    url(r'^create/$', views.TopicCreateView.as_view(), name='create'),
    url(r'^list/$', views.TopicListView.as_view(), name='list'),
    url(r'^activate/$', views.TopicActivateView.as_view(), name='activate'),
    url(r'^delete/$', views.TopicDeleteView.as_view(), name='delete'),
    url(r'^post_visit/$', views.TopicVisitAJAXView.as_view(), name='post_visit'),


]
