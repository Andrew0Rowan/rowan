/*
 * Event loop based on select() loop
 * Copyright (c) 2002-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */


#ifndef __EDIT_H__
#define __EDIT_H__

int edit_init(void (*cmd_cb)(void *ctx, char *cmd),
            void (*eof_cb)(void *ctx),
            char ** (*completion_cb)(void *ctx, const char *cmd, int pos),
            void *ctx, const char *history_file);
void edit_deinit(const char *history_file,
        int (*filter_cb)(void *ctx, const char *cmd));
void edit_clear_line(void);
void edit_redraw(void);
void edit_set_finish(int finish);

#endif /* __EDIT_H__ */
