/*
  Copyright (c) 2010-2011 Gluster, Inc. <http://www.gluster.com>
  This file is part of GlusterFS.

  GlusterFS is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License,
  or (at your option) any later version.

  GlusterFS is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

#include "statedump.h"
#include "stack.h"

static inline
int call_frames_count (call_frame_t *call_frame)
{
        call_frame_t *pos;
        int32_t count = 0;

        if (!call_frame)
                return count;

        for (pos = call_frame; pos != NULL; pos = pos->next)
                count++;

        return count;
}

void
gf_proc_dump_call_frame (call_frame_t *call_frame, const char *key_buf,...)
{

        char prefix[GF_DUMP_MAX_BUF_LEN];
        va_list ap;
        char key[GF_DUMP_MAX_BUF_LEN];
        call_frame_t my_frame;
        int  ret = -1;

        if (!call_frame)
                return;

        GF_ASSERT (key_buf);

        memset(prefix, 0, sizeof(prefix));
        memset(&my_frame, 0, sizeof(my_frame));
        va_start(ap, key_buf);
        vsnprintf(prefix, GF_DUMP_MAX_BUF_LEN, key_buf, ap);
        va_end(ap);

        ret = TRY_LOCK(&call_frame->lock);
        if (ret) {
                gf_log("", GF_LOG_WARNING, "Unable to dump call frame"
                       " errno: %s", strerror (errno));
                return;
        }

        memcpy(&my_frame, call_frame, sizeof(my_frame));
        UNLOCK(&call_frame->lock);

        gf_proc_dump_build_key(key, prefix,"ref_count");
        gf_proc_dump_write(key, "%d", my_frame.ref_count);
        gf_proc_dump_build_key(key, prefix,"translator");
        gf_proc_dump_write(key, "%s", my_frame.this->name);
        gf_proc_dump_build_key(key, prefix,"complete");
        gf_proc_dump_write(key, "%d", my_frame.complete);
        if (my_frame.parent) {
                gf_proc_dump_build_key(key, prefix,"parent");
                gf_proc_dump_write(key, "%s", my_frame.parent->this->name);
        }
        if (my_frame.wind_from) {
                gf_proc_dump_build_key(key, prefix, "wind_from");
                gf_proc_dump_write(key, "%s", my_frame.wind_from);
        }
        if (my_frame.wind_to) {
                gf_proc_dump_build_key(key, prefix, "wind_to");
                gf_proc_dump_write(key, "%s", my_frame.wind_to);
        }
        if (my_frame.unwind_from) {
                gf_proc_dump_build_key(key, prefix, "unwind_from");
                gf_proc_dump_write(key, "%s", my_frame.unwind_from);
        }
        if (my_frame.unwind_to) {
                gf_proc_dump_build_key(key, prefix, "unwind_to");
                gf_proc_dump_write(key, "%s", my_frame.unwind_to);
        }
}


void
gf_proc_dump_call_stack (call_stack_t *call_stack, const char *key_buf,...)
{
        char prefix[GF_DUMP_MAX_BUF_LEN];
        va_list ap;
        call_frame_t *trav;
        int32_t cnt, i;
        char key[GF_DUMP_MAX_BUF_LEN];

        if (!call_stack)
                return;

        GF_ASSERT (key_buf);

        cnt = call_frames_count(&call_stack->frames);

        memset(prefix, 0, sizeof(prefix));
        va_start(ap, key_buf);
        vsnprintf(prefix, GF_DUMP_MAX_BUF_LEN, key_buf, ap);
        va_end(ap);

        gf_proc_dump_build_key(key, prefix,"uid");
        gf_proc_dump_write(key, "%d", call_stack->uid);
        gf_proc_dump_build_key(key, prefix,"gid");
        gf_proc_dump_write(key, "%d", call_stack->gid);
        gf_proc_dump_build_key(key, prefix,"pid");
        gf_proc_dump_write(key, "%d", call_stack->pid);
        gf_proc_dump_build_key(key, prefix,"unique");
        gf_proc_dump_write(key, "%Ld", call_stack->unique);

        gf_proc_dump_build_key(key, prefix,"op");
        if (call_stack->type == GF_OP_TYPE_FOP)
                gf_proc_dump_write(key, "%s", gf_fop_list[call_stack->op]);
        else if (call_stack->type == GF_OP_TYPE_MGMT)
                gf_proc_dump_write(key, "%s", gf_mgmt_list[call_stack->op]);

        gf_proc_dump_build_key(key, prefix,"type");
        gf_proc_dump_write(key, "%d", call_stack->type);
        gf_proc_dump_build_key(key, prefix,"cnt");
        gf_proc_dump_write(key, "%d", cnt);

        trav = &call_stack->frames;

        for (i = 1; i <= cnt; i++) {
                if (trav) {
                        gf_proc_dump_add_section("%s.frame.%d", prefix, i);
                        gf_proc_dump_call_frame(trav, "%s.frame.%d", prefix, i);
                        trav = trav->next;
                }
        }
}

void
gf_proc_dump_pending_frames (call_pool_t *call_pool)
{

        call_stack_t     *trav = NULL;
        int              i = 1;
        int              ret = -1;

        if (!call_pool)
                return;

        ret = TRY_LOCK (&(call_pool->lock));
        if (ret) {
                gf_log("", GF_LOG_WARNING, "Unable to dump call pool"
                       " errno: %d", errno);
                return;
        }


        gf_proc_dump_add_section("global.callpool");
        gf_proc_dump_write("global.callpool","%p", call_pool);
        gf_proc_dump_write("global.callpool.cnt","%d", call_pool->cnt);


        list_for_each_entry (trav, &call_pool->all_frames, all_frames) {
                gf_proc_dump_add_section("global.callpool.stack.%d",i);
                gf_proc_dump_call_stack(trav, "global.callpool.stack.%d", i);
                i++;
        }
        UNLOCK (&(call_pool->lock));
}

gf_boolean_t
__is_fuse_call (call_frame_t *frame)
{
        gf_boolean_t    is_fuse_call = _gf_false;
        GF_ASSERT (frame);
        GF_ASSERT (frame->root);

        if (NFS_PID != frame->root->pid)
                is_fuse_call = _gf_true;
        return is_fuse_call;
}
