from django.conf.urls import url
from django.views.generic import TemplateView

from treccoreweb.search import views

urlpatterns = [
    url(r'^$', views.SearchHomePageView.as_view(), name='main'),
    url(r'^get_docs/$',
        views.SearchListView.as_view(), name='get_docs'),

]
