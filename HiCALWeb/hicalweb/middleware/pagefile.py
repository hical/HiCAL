from django.utils.deprecation import MiddlewareMixin


class PageHTMLMiddleware(MiddlewareMixin):
    """
    Adds file name of the rendered template to context of the response for certain views.
    """

    def process_request(self, request):
        # Add page_file field to request
        request.page_file = None

    def process_view(self, request, view, *args, **kwargs):
        try:
            request.page_file = view.view_class.template_name
        except AttributeError:
            pass

