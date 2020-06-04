from django.urls import path

from hicalweb.search import views

app_name = "search"

urlpatterns = [
    path('', views.SearchHomePageView.as_view(),
         name='main'),
    path('get_docs/', views.SearchListView.as_view(),
         name='get_docs'),
    path('get_single_doc/', views.SearchGetDocAJAXView.as_view(),
         name='get_doc'),

    # Ajax views
    path('post_search_status/', views.SearchInputStatusAJAXView.as_view(),
         name='post_search_status'),
    path('post_keystroke/', views.SearchKeystrokeAJAXView.as_view(),
         name='post_keystroke'),
    path('post_search_request/', views.SearchButtonView.as_view(),
         name='post_search_request'),
    path('post_search_doc_open/', views.SearchSearchDocOpenedView.as_view(),
         name='post_search_doc_open'),
]
