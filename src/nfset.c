/* ipset.c is Copyright (c) 2013 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

#if defined(HAVE_NFSET) && defined(HAVE_LINUX_NETWORK)

#include <nftables/libnftables.h>

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <linux/version.h>

struct ntf_ctx *ctx = NULL;

const char nft_cmd_add_ipv4[] = "add element %s { %d.%d.%d.%d }";

#define CMD_BUFFER_SIZE 4096
char cmd_buffer[CMD_BUFFER_SIZE];

static int start_with(const char *s, const char *prefix) {
  return strncmp(prefix, s, strlen(prefix)) == 0;
}

void nfset_init() {
  if (daemon->nfsets != NULL)
    nfset_display(&daemon->nfsets->root, 0);
  ctx = nft_ctx_new(NFT_CTX_DEFAULT);
  if (ctx == NULL) exit(EXIT_FAILURE);
}

int add_to_nfset(const char *setname, const struct all_addr *ipaddr, int flags) {
  if (flags & F_IPV4) {
    if (start_with(setname, "ipv4 ")) {
      uint8_t *ip = &ipaddr->addr.addr4.s_addr;
      const char *real_setname = setname + strlen("ipv4 ");
      snprintf(cmd_buffer, CMD_BUFFER_SIZE, nft_cmd_add_ipv4, real_setname, ip[0], ip[1], ip[2], ip[3]);
      nft_run_cmd_from_buffer(ctx, cmd_buffer);
    }
  }
  return 0;
}

#endif
