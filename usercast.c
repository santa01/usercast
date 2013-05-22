/*
 * Copyright (c) 2013 Pavlo Lavrenenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define PURPLE_PLUGINS

#define USERCAST_PLUGIN_ID      "core-usercast"
#define USERCAST_PLUGIN_NAME    "usercast"
#define USERCAST_PLUGIN_VERSION "0.0.1"

#include <conversation.h>
#include <signals.h>
#include <gtkconv.h>
#include <version.h>
#include <debug.h>

static gint double_click_time = 400;
static GTimeVal click_time = { 0, 0 };

static gboolean
conversation_nick_clicked(PurpleConversation* conv, gchar* nick, guint button)
{
    GdkEvent* next_event = NULL;
    gchar* user_cast = NULL;
    glong first_click = 0;
    glong second_click = 0;

    if (purple_conversation_get_type(conv) != PURPLE_CONV_TYPE_CHAT)
    {
        purple_debug_warning(USERCAST_PLUGIN_NAME, "Conversation `%s' is not a chat\n", conv->name);
        return FALSE;
    }

    if (!PIDGIN_IS_PIDGIN_CONVERSATION(conv))
    {
        purple_debug_warning(USERCAST_PLUGIN_NAME, "Conversation `%s' is not a pidgin conversation\n",
                             conv->name);
        return FALSE;
    }

    if (button != 1)
        return FALSE;

    if (click_time.tv_sec != 0 && click_time.tv_usec != 0)
    {
        first_click = click_time.tv_sec * 1000 + click_time.tv_usec / 1000;
        g_get_current_time(&click_time);
        second_click = click_time.tv_sec * 1000 + click_time.tv_usec / 1000;

        if (second_click - first_click <= double_click_time)
        {
            user_cast = g_strdup_printf("%s: ", nick);
            gtk_text_buffer_insert_at_cursor(PIDGIN_CONVERSATION(conv)->entry_buffer, user_cast, -1);
            g_free(user_cast);

            purple_debug_info(USERCAST_PLUGIN_NAME, "Casted user `%s' to `%s'\n", nick, conv->name);

            while (!(next_event = gdk_event_peek()))
                g_usleep(1000);

            /* Pingin handles GDK_2BUTTON_PRESS separately, make it stop */
            if (next_event->type == GDK_2BUTTON_PRESS)
            {
                gdk_event_free(next_event);
                gdk_event_free(gdk_event_get());
            }
        }

        click_time.tv_sec = 0;
        click_time.tv_usec = 0;
    }
    else
        g_get_current_time(&click_time);

    return TRUE;
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
    GdkScreen* default_screen = NULL;
    GValue gtk_double_click_time;

    if (!(default_screen = gdk_screen_get_default()))
    {
        purple_debug_error(USERCAST_PLUGIN_NAME, "Cannot obtain default screen\n");
        return FALSE;
    }

    /* FIXME: Never TRUE in KDE4 */
    if (gdk_screen_get_setting(default_screen, "gtk-double-click-time", &gtk_double_click_time))
        double_click_time = g_value_get_int(&gtk_double_click_time);

    purple_debug_info(USERCAST_PLUGIN_NAME, "Double click time is %d ms\n", double_click_time);

    purple_signal_connect(pidgin_conversations_get_handle(), "chat-nick-clicked",
                          plugin, PURPLE_CALLBACK(conversation_nick_clicked), NULL);
    return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
    purple_signals_disconnect_by_handle(plugin);
    return TRUE;
}

static PurplePluginInfo info =
{
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,
    USERCAST_PLUGIN_ID,
    USERCAST_PLUGIN_NAME,
    USERCAST_PLUGIN_VERSION,
    "Paste username into chat conversation",
    "Double-click username to paste it into chat input area",
    "Pavlo Lavrenenko <santa.ssh@gmail.com>",
    "https://github.com/santa01/usercast",
    plugin_load,
    plugin_unload,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(usercast, init_plugin, info)
