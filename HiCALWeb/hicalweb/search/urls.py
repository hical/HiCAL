from django.conf.urls import url

from hicalweb.search import views

urlpatterns = [
    url(r'^$', views.SearchHomePageView.as_view(),
        name='main'),
    url(r'^get_docs/$',
        views.SearchListView.as_view(), name='get_docs'),
    url(r'^get_single_doc/$',
        views.SearchGetDocAJAXView.as_view(),
        name='get_doc'),

    # Ajax views
    url(r'^post_search_status/$', views.SearchInputStatusAJAXView.as_view(),
        name='post_search_status'),
    url(r'^post_keystroke/$', views.SearchKeystrokeAJAXView.as_view(),
        name='post_keystroke'),
    url(r'^post_search_request/$', views.SearchButtonView.as_view(),
        name='post_search_request'),
    url(r'^post_search_doc_open/$', views.SearchSearchDocOpenedView.as_view(),
        name='post_search_doc_open'),
]
