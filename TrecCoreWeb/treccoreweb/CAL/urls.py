from django.conf.urls import url
from django.views.generic import TemplateView

from treccoreweb.CAL import views

urlpatterns = [
    url(r'^$', views.CALHomePageView.as_view(), name='main'),

    url(r'^post_visit/$', views.CALVisitAJAXView.as_view(), name='post_visit'),
    url(r'^post_ctrlf/$', views.CALCtrlFAJAXView.as_view(), name='post_ctrlf'),
    url(r'^get_docs/$', views.DocAJAXView.as_view(), name='get_docs'),
    url(r'^post_judgment/$', views.JudgmentAJAXView.as_view(), name='post_judgment'),
]
