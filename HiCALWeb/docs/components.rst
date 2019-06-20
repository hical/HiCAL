Components
==========

Our system currently supports two retrieval components, CAL and Search.
All components in the architecture are stand-alone and interact with each other via HTTP API.
You can also add your own component and run it locally if you wish.

Let's go through an example below.


Adding a component
^^^^^^^^^^^^^^^^^^

Let's go through an example of adding a new component.
For simplicity, the component will simply return a list of predefine documents that need to be judged.
The component will allow user to access and judge these documents in the order they are define.
We'll call this component `Iterative`.

Let's add a new app in our project by running this command::

    $ python manage.py createapp iterative

This will create a folder with all the files we need.
Make sure the newly created app is in the right directory. It should be in ``/hicalweb``.

Let's add the new app to our settings file so that our server is aware of it.
You can do this by going to ``config/settings/base.py`` file and adding ``hicalweb.iterative`` to `LOCAL_APPS`::

    # Apps specific for this project go here.
    LOCAL_APPS = [
        # custom users app
        ...
        'hicalweb.iterative',
    ]


Now add a specific url for the app in ``config/urls.py``::


    urlpatterns = [
        ...
        # add this
        url(r'^iterative/', include('hicalweb.iterative.urls', namespace='iterative')),

    ]

and create a new ``urls.py`` file under the ``iterative`` app directory::

    from django.conf.urls import url

    from hicalweb.iterative import views

    urlpatterns = [
        url(r'^$', views.HomePageView.as_view(), name='main'),

        # Ajax views
        url(r'^post_log/$', views.MessageAJAXView.as_view(), name='post_log_msg'),
        url(r'^get_docs/$', views.DocAJAXView.as_view(), name='get_docs'),
    ]

This will allow access to the component from the url ``/iterative/``

We will use ``get_docs/`` url pattern to link to our view ``DocAJAXView``, which will be responsible for getting the documents to judge.
Please look at ``iterative/views.py`` for more details on what these views do.

Let's look at ``DocAJAXView`` view as an example::


    from interfaces.DocumentSnippetEngine import functions as DocEngine
    from interfaces.Iterative import functions as IterativeEngine

    class DocAJAXView(views.CsrfExemptMixin, views.LoginRequiredMixin,
                      views.JsonRequestResponseMixin,
                      views.AjaxResponseMixin, generic.View):
        """
        View to get a list of documents to judge
        """
        require_json = False

        def render_timeout_request_response(self, error_dict=None):
            if error_dict is None:
                error_dict = self.error_response_dict
            json_context = json.dumps(
                error_dict,
                cls=self.json_encoder_class,
                **self.get_json_dumps_kwargs()
            ).encode('utf-8')
            return HttpResponse(
                json_context, content_type=self.get_content_type(), status=502)

        def get_ajax(self, request, *args, **kwargs):
            try:
                current_task = self.request.user.current_task
                docs_ids = IterativeEngine.get_documents(current_task.topic.number)
                docs_ids = helpers.remove_judged_docs(docs_ids,
                                                      self.request.user,
                                                      current_task)
                documents = DocEngine.get_documents(docs_ids, query=None)
                return self.render_json_response(documents)
            except TimeoutError:
                error_dict = {u"message": u"Timeout error. Please check status of servers."}
                return self.render_timeout_request_response(error_dict)


The method ``get_ajax`` will be called from our template to retrieve a list of documents.
The variable ``docs_ids`` contains the list of documents ids that we need to retrieve their content.
We put all functions associated with the component retrieval in ``/interfaces`` directory.

The variable ``documents`` will contain a list of documents with their content, in the following format::

    [
        {
            'doc_id': "012345",
            'title': "Document Title",
            'content': "Body of document",
            'date': "Date of document"
        },
        ...
    ]

`documents` will be passed as context to our template, and loaded in the browser.

The HTML associated is under ``iterative/templates/iterative/iterative.html``.
The ``HomePageView`` view, which the url pattern ``/iterative/^`` is pointing to, will render this page.
If you look at the ``HomePageView`` view, you will see that ``template_name = 'iterative/iterative.html'``.

You can modify the ``iterative.html`` as you like.
The ``iterative.html`` file will call `iterative:get_docs` once the page is loaded.
You can customize the behaviour by modifying the javascript code in `iterative.html` to meet your needs.

The judgment buttons in the interface will call the ``send_judgment()`` function, which will send a call to the url ``judgment:post_judgment`` in ``judgments/url.py`` and run the view ``JudgmentAJAXView``, which will save the document judgment to the database.
If you would like to include more information to be saved with each judgment, you can modify the ``send_judgment()`` in ``iterative.html``.
You will also need to modify the database model, `Judgment`, associated with each judgment instance. The `Judgment` class is located in ``judgment/models.py``.




CAL and Search
^^^^^^^^^^^^^^
Both components follow a similar pattern as the `Iterative` component described above.
The difference is that both are running somewhere else, and we need to make an http calls to get the documents we need to judge.
The IPs for each component is set in ``config/settings/base.py``.
The methods used to retrieve documents are found in ``/interfaces/``.

The HTML associted with each component is found under the ``templates`` folder under the component directory.


