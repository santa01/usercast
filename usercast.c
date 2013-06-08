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

#include "usercast.h"

#include <conversation.h>
#include <signals.h>
#include <gtkconv.h>
#include <pluginpref.h>
#include <prefs.h>
#include <version.h>
#include <debug.h>
#include <string.h>

static void
cast_nick(PidginConversation* conv, const gchar* nick)
{
    Policy prefix_policy = purple_prefs_get_int(USERCAST_CONF_PREFIX_POLICY);
    Policy postfix_policy = purple_prefs_get_int(USERCAST_CONF_POSTFIX_POLICY);

    const gchar* user_prefix = purple_prefs_get_string(USERCAST_CONF_PREFIX);
    const gchar* user_postfix = purple_prefs_get_string(USERCAST_CONF_POSTFIX);
    gchar* user_cast = NULL;
    gchar* user_part = NULL;

    gint cursor_position = 0;
    gint text_length = 0;
    gchar* text = NULL;

    g_object_get(conv->entry_buffer, "cursor-position", &cursor_position, NULL);
    g_object_get(conv->entry_buffer, "text", &text, NULL);
    text_length = strlen(text);

    if (prefix_policy == POLICY_ALWAYS ||
            (prefix_policy == POLICY_FIRST_WORD && cursor_position == 0) ||
            (prefix_policy == POLICY_LAST_WORD && cursor_position == text_length))
        user_part = g_strdup_printf("%s%s", user_prefix, nick);
    else
        user_part = g_strdup_printf("%s", nick);

    if (postfix_policy == POLICY_ALWAYS ||
            (postfix_policy == POLICY_FIRST_WORD && cursor_position == 0) ||
            (postfix_policy == POLICY_LAST_WORD && cursor_position == text_length))
    {
        user_cast = g_strdup_printf("%s%s", user_part, user_postfix);
        g_free(user_part);
    }
    else
        user_cast = user_part;

    gtk_text_buffer_insert_at_cursor(conv->entry_buffer, user_cast, -1);
    g_free(user_cast);
}

static gboolean
conversation_nick_clicked(PurpleConversation* conv, gchar* nick, guint button)
{
    GdkEvent* next_event = NULL;
    PidginConversation* pidgin_conv = NULL;
    gint event_timeout = 100; /* milliseconds */

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

    /* Wait for either GDK_2BUTTON_EVENT or something else to arrive */
    for (; event_timeout > 0; event_timeout--)
    {
        if ((next_event = gdk_event_peek()))
            break;
        g_usleep(1000); /* 1 millisecond */
    }

    /* Pingin handles GDK_2BUTTON_PRESS separately */
    if (next_event && next_event->type == GDK_2BUTTON_PRESS)
    {
        /* Remove GDK_2BUTTON_PRESS from event queue */
        gdk_event_free(next_event);
        gdk_event_free(gdk_event_get());

        pidgin_conv = PIDGIN_CONVERSATION(conv);
        cast_nick(pidgin_conv, nick);
        gtk_widget_grab_focus(pidgin_conv->entry);

        purple_debug_info(USERCAST_PLUGIN_NAME, "Casted user `%s' to `%s'\n", nick, conv->name);
        return TRUE;
    }

    return FALSE;
}

static gboolean
plugin_load(PurplePlugin* plugin)
{
    purple_signal_connect(pidgin_conversations_get_handle(), "chat-nick-clicked",
                          plugin, PURPLE_CALLBACK(conversation_nick_clicked), NULL);
    return TRUE;
}

static gboolean
plugin_unload(PurplePlugin* plugin)
{
    purple_signals_disconnect_by_handle(plugin);
    return TRUE;
}

static PurplePluginPrefFrame*
get_plugin_pref_frame(PurplePlugin* plugin) {
    PurplePluginPrefFrame* frame = purple_plugin_pref_frame_new();
    PurplePluginPref* pref = NULL;

    pref = purple_plugin_pref_new_with_label("Prefix preferences");
    purple_plugin_pref_frame_add(frame, pref);

    pref = purple_plugin_pref_new_with_name_and_label(USERCAST_CONF_PREFIX, "Nick prefix");
    purple_plugin_pref_frame_add(frame, pref);

    pref = purple_plugin_pref_new_with_name_and_label(USERCAST_CONF_PREFIX_POLICY, "Prefix use policy");
    purple_plugin_pref_set_type(pref, PURPLE_PLUGIN_PREF_CHOICE);
    purple_plugin_pref_add_choice(pref, "Always", GINT_TO_POINTER(POLICY_ALWAYS));
    purple_plugin_pref_add_choice(pref, "First word", GINT_TO_POINTER(POLICY_FIRST_WORD));
    purple_plugin_pref_add_choice(pref, "Last word", GINT_TO_POINTER(POLICY_LAST_WORD));
    purple_plugin_pref_add_choice(pref, "Never", GINT_TO_POINTER(POLICY_NEVER));
    purple_plugin_pref_frame_add(frame, pref);

    pref = purple_plugin_pref_new_with_label("Postfix preferences");
    purple_plugin_pref_frame_add(frame, pref);

    pref = purple_plugin_pref_new_with_name_and_label(USERCAST_CONF_POSTFIX, "Nick postfix");
    purple_plugin_pref_frame_add(frame, pref);

    pref = purple_plugin_pref_new_with_name_and_label(USERCAST_CONF_POSTFIX_POLICY, "Postfix use policy");
    purple_plugin_pref_set_type(pref, PURPLE_PLUGIN_PREF_CHOICE);
    purple_plugin_pref_add_choice(pref, "Always", GINT_TO_POINTER(POLICY_ALWAYS));
    purple_plugin_pref_add_choice(pref, "First word", GINT_TO_POINTER(POLICY_FIRST_WORD));
    purple_plugin_pref_add_choice(pref, "Last word", GINT_TO_POINTER(POLICY_LAST_WORD));
    purple_plugin_pref_add_choice(pref, "Never", GINT_TO_POINTER(POLICY_NEVER));
    purple_plugin_pref_frame_add(frame, pref);

    return frame;
}


static PurplePluginUiInfo prefs_info = {
    get_plugin_pref_frame,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

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
    &prefs_info,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static void
init_plugin(PurplePlugin* plugin)
{
    purple_prefs_add_none(USERCAST_CONF_ROOT);
    purple_prefs_add_string(USERCAST_CONF_PREFIX, "Hey, ");
    purple_prefs_add_string(USERCAST_CONF_POSTFIX, ", ");
    purple_prefs_add_int(USERCAST_CONF_PREFIX_POLICY, POLICY_FIRST_WORD);
    purple_prefs_add_int(USERCAST_CONF_POSTFIX_POLICY, POLICY_ALWAYS);
}

PURPLE_INIT_PLUGIN(usercast, init_plugin, info)
