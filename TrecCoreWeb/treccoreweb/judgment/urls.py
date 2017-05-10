from django.conf.urls import url

from treccoreweb.judgment import views

urlpatterns = [
    url(r'^post_judgment/$', views.JudgmentAJAXView.as_view(), name='post_judgment'),
]
