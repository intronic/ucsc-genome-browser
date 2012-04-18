from django.conf.urls.defaults import patterns, url
from django.views.generic import DetailView, ListView
from models import Sex

urlpatterns = patterns('',
    url(r'^$', 
        ListView.as_view(
        queryset=Sex.objects.order_by('term'),
        context_object_name='list',
        template_name='cvDb/index.html'),
        ),
    )

