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

#ifndef USERCAST_H
#define USERCAST_H

#define PURPLE_PLUGINS

#define USERCAST_PLUGIN_ID              "core-usercast"
#define USERCAST_PLUGIN_NAME            "Usercast"
#define USERCAST_PLUGIN_VERSION         "0.2.1"

#define USERCAST_CONF_ROOT              "/plugins/core/usercast"
#define USERCAST_CONF_PREFIX            USERCAST_CONF_ROOT"/prefix"
#define USERCAST_CONF_POSTFIX           USERCAST_CONF_ROOT"/postfix"
#define USERCAST_CONF_PREFIX_POLICY     USERCAST_CONF_ROOT"/prefix_policy"
#define USERCAST_CONF_POSTFIX_POLICY    USERCAST_CONF_ROOT"/postfix_policy"

typedef enum {
    POLICY_ALWAYS,
    POLICY_FIRST_WORD,
    POLICY_LAST_WORD,
    POLICY_NEVER
} Policy;

#endif /* USERCAST_H */
