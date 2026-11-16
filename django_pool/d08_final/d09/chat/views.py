from django.contrib.auth.decorators import login_required
from django.shortcuts import render, get_object_or_404, redirect
from .models import ChatRoom


def chat_list(request):
    """Page affichant les 3 chatrooms."""
    rooms = ChatRoom.objects.all()[:3]
    return render(request, 'chat/chat_list.html', {'rooms': rooms})


@login_required
def chat_room(request, room_id):
    """Page d'une chatroom."""
    room = get_object_or_404(ChatRoom, pk=room_id)
    return render(request, 'chat/chat_room.html', {'room': room})
