from django.conf.urls import url

from treccoreweb.archive import views

urlpatterns = [
    url(r'^$', views.HomePageView.as_view(), name='main'),

]
