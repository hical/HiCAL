from django.conf.urls import url
from django.views.generic import TemplateView

from treccoreweb.search import views

urlpatterns = [
    url(r'^$', views.SearchHomePageView.as_view(),
        name='main'),
    url(r'^get_docs/$',
        views.SearchListView.as_view(), name='get_docs'),
    url(r'^get_single_doc/$',
        views.SearchGetDocAJAXView.as_view(),
        name='get_doc'),
    url(r'^post_visit/$', views.SearchVisitAJAXView.as_view(),
        name='post_visit'),
    url(r'^post_search_status/$', views.SearchInputStatusAJAXView.as_view(),
        name='post_search_status'),
    url(r'^post_keystroke/$', views.SearchKeystrokeAJAXView.as_view(),
        name='post_keystroke'),

]
