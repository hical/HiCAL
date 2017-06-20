from django.conf.urls import url

from treccoreweb.progress import views

urlpatterns = [
    url(r'^$', views.Home.as_view(), name='home'),
    url(r'^pretask/$', views.PretaskView.as_view(), name='pretask'),
    url(r'^posttask/$', views.PosttaskView.as_view(), name='posttask')
]
