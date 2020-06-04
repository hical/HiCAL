from django.conf import settings
from django.conf.urls import include, url
from django.conf.urls.static import static
from django.contrib import admin
from django.urls import path
from django.views.generic import TemplateView
from django.views import defaults as default_views

urlpatterns = [
    path('', include('hicalweb.progress.urls', namespace='progress')),
    path('about/', TemplateView.as_view(template_name='pages/about.html'), name='about'),

    # Django Admin, use {% url 'admin:index' %}
    url(settings.ADMIN_URL, admin.site.urls),

    # User management
    path('users/', include('hicalweb.users.urls', namespace='users')),

    path('accounts/', include('allauth.urls')),

    # Custom urls includes go here
    path('CAL/', include('hicalweb.CAL.urls', namespace='CAL')),
    path('iterative/', include('hicalweb.iterative.urls', namespace='iterative')),
    path('search/', include('hicalweb.search.urls', namespace='search')),
    path('topic/', include('hicalweb.topic.urls', namespace='topic')),
    path('judgment/', include('hicalweb.judgment.urls', namespace='judgment')),

] + static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)

if settings.DEBUG:
    # This allows the error pages to be debugged during development, just visit
    # these url in browser to see how these error pages look like.
    urlpatterns += [
        url(r'^400/$', default_views.bad_request, kwargs={'exception': Exception('Bad Request!')}),
        url(r'^403/$', default_views.permission_denied, kwargs={'exception': Exception('Permission Denied')}),
        url(r'^404/$', default_views.page_not_found, kwargs={'exception': Exception('Page not Found')}),
        url(r'^500/$', default_views.server_error),
    ]
    if 'debug_toolbar' in settings.INSTALLED_APPS:
        import debug_toolbar
        urlpatterns = [
            url(r'^__debug__/', include(debug_toolbar.urls)),
        ] + urlpatterns
