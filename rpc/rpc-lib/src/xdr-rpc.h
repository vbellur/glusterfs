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

#ifndef _XDR_RPC_H
#define _XDR_RPC_H_

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#ifndef GF_SOLARIS_HOST_OS
#include <rpc/rpc.h>
#endif

#ifdef GF_SOLARIS_HOST_OS
#include <rpc/auth.h>
#include <rpc/auth_sys.h>
#endif

//#include <rpc/pmap_clnt.h>
#include <arpa/inet.h>
#include <rpc/xdr.h>
#include <sys/uio.h>

/* Converts a given network buffer from its XDR format to a structure
 * that contains everything an RPC call needs to work.
 */
extern int
xdr_to_rpc_call (char *msgbuf, size_t len, struct rpc_msg *call,
                 struct iovec *payload, char *credbytes, char *verfbytes);

extern int
rpc_fill_empty_reply (struct rpc_msg *reply, uint32_t xid);

extern int
rpc_fill_denied_reply (struct rpc_msg *reply, int rjstat, int auth_err);

extern int
rpc_fill_accepted_reply (struct rpc_msg *reply, int arstat, int proglow,
                         int proghigh, int verf, int len, char *vdata);
extern int
rpc_reply_to_xdr (struct rpc_msg *reply, char *dest, size_t len,
                  struct iovec *dst);

extern int
xdr_to_auth_unix_cred (char *msgbuf, int msglen, struct authunix_parms *au,
                       char *machname, gid_t *gids);
/* Macros that simplify accesing the members of an RPC call structure. */
#define rpc_call_xid(call)              ((call)->rm_xid)
#define rpc_call_direction(call)        ((call)->rm_direction)
#define rpc_call_rpcvers(call)          ((call)->ru.RM_cmb.cb_rpcvers)
#define rpc_call_program(call)          ((call)->ru.RM_cmb.cb_prog)
#define rpc_call_progver(call)          ((call)->ru.RM_cmb.cb_vers)
#define rpc_call_progproc(call)         ((call)->ru.RM_cmb.cb_proc)
#define rpc_opaque_auth_flavour(oa)     ((oa)->oa_flavor)
#define rpc_opaque_auth_len(oa)         ((oa)->oa_length)

#define rpc_call_cred_flavour(call)     (rpc_opaque_auth_flavour ((&(call)->ru.RM_cmb.cb_cred)))
#define rpc_call_cred_len(call)         (rpc_opaque_auth_len ((&(call)->ru.RM_cmb.cb_cred)))


#define rpc_call_verf_flavour(call)     (rpc_opaque_auth_flavour ((&(call)->ru.RM_cmb.cb_verf)))
#define rpc_call_verf_len(call)         (rpc_opaque_auth_len ((&(call)->ru.RM_cmb.cb_verf)))

#endif
