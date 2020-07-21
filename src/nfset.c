/* ipset.c is Copyright (c) 2013 Jason A. Donenfeld <Jason@zx2c4.com>. All
   Rights Reserved.

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

#include <arpa/inet.h>
#include <errno.h>
#include <linux/version.h>
#include <nftables/libnftables.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/utsname.h>

struct ntf_ctx *ctx = NULL;

const char nft_cmd_add[] = "add element %s { %s }";

#define CMD_BUFFER_SIZE 4096
char cmd_buffer[CMD_BUFFER_SIZE];

char addr_buffer[ADDRSTRLEN + 1];

static int start_with(const char *s, const char *prefix) {
  return strncmp(prefix, s, strlen(prefix)) == 0;
}

void nfset_init() {
  ctx = nft_ctx_new(NFT_CTX_DEFAULT);
  if (ctx == NULL) exit(EXIT_FAILURE);
}

int add_to_nfset(const char *setname, const struct all_addr *ipaddr,
                 int flags) {
  if (flags & F_IPV4) {
    if (start_with(setname, "ipv4 ")) {
      const char *real_setname = setname + strlen("ipv4 ");
      inet_ntop(AF_INET, ipaddr, addr_buffer, ADDRSTRLEN);
      snprintf(cmd_buffer, CMD_BUFFER_SIZE, nft_cmd_add, real_setname, addr_buffer);
      nft_run_cmd_from_buffer(ctx, cmd_buffer);
    }
  } else if (flags & F_IPV6) {
    if (start_with(setname, "ipv6 ")) {
      const char *real_setname = setname + strlen("ipv6 ");
      inet_ntop(AF_INET6, ipaddr, addr_buffer, ADDRSTRLEN);
      snprintf(cmd_buffer, CMD_BUFFER_SIZE, nft_cmd_add, real_setname, addr_buffer);
      nft_run_cmd_from_buffer(ctx, cmd_buffer);
    }
  }
  return 0;
}

#endif
