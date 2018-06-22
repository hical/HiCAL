from django.conf.urls import url
from django.views.generic import TemplateView

from hicalweb.CAL import views

urlpatterns = [
    url(r'^$', views.CALHomePageView.as_view(), name='main'),

    # Ajax views
    url(r'^post_log/$', views.CALMessageAJAXView.as_view(), name='post_log_msg'),
    url(r'^get_docs/$', views.DocAJAXView.as_view(), name='get_docs'),
]
