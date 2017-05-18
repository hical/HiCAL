from django.conf.urls import url

from treccoreweb.judgment import views

urlpatterns = [
    url(r'^post_judgment/$',
        views.JudgmentAJAXView.as_view(),
        name='post_judgment'),
    url(r'^get_latest/(?P<number_of_docs_to_show>\d+)/$',
        views.GetLatestAJAXView.as_view(),
        name='get_latest'),
]
