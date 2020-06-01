from django.conf.urls import url

from hicalweb.judgment import views

urlpatterns = [

    url(r'^view/$', views.JudgmentsView.as_view(), name='view'),

    # Ajax views
    url(r'^post_judgment/$',
        views.JudgmentAJAXView.as_view(),
        name='post_judgment'),
    url(r'^post_nojudgment/$',
        views.NoJudgmentAJAXView.as_view(),
        name='post_nojudgment'),
    url(r'^get_latest/(?P<number_of_docs_to_show>\d+)/$',
        views.GetLatestAJAXView.as_view(),
        name='get_latest'),

]
