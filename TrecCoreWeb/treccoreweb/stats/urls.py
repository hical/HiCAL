from django.conf.urls import url

from treccoreweb.stats import views

urlpatterns = [
    url(r'^$',
        views.StatsHomePageView.as_view(),
        name='home'),
]
