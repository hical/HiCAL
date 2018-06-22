from django.conf.urls import url

from hicalweb.archive import views

urlpatterns = [
    url(r'^$', views.HomePageView.as_view(), name='main'),

]
