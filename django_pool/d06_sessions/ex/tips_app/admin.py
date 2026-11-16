from django.contrib import admin
from .models import Tip


@admin.register(Tip)
class TipAdmin(admin.ModelAdmin):
    list_display = ('content', 'author', 'date', 'upvote_count', 'downvote_count')
    list_filter = ('date',)
    search_fields = ('content', 'author__username')
