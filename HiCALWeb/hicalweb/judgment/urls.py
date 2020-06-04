from django.urls import path, re_path

from hicalweb.judgment import views

app_name = "judgment"

urlpatterns = [

    path('view/', views.JudgmentsView.as_view(),
         name='view'),

    # Ajax views
    path(r'post_judgment/', views.JudgmentAJAXView.as_view(),
         name='post_judgment'),
    path(r'post_nojudgment/', views.NoJudgmentAJAXView.as_view(),
         name='post_nojudgment'),
    re_path(r'^get_latest/(?P<number_of_docs_to_show>\d+)/$',
            views.GetLatestAJAXView.as_view(),
            name='get_latest'),

]
