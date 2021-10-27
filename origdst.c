/* code to show the original destination address if changed by linux/netfilter DNAT or REDIRECT
 * lacks error handling etc.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/netfilter_ipv4.h>
#include <netdb.h>

int main(void)
{
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = ntohs(8911);
    sa.sin_addr.s_addr = 0;
    
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int one = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    bind(s, (struct sockaddr *)&sa, sizeof sa);
    listen(s, 1);
    
    int t = accept(s, NULL, NULL);
    close(s);

    socklen_t len = sizeof sa;
    getsockopt(t, SOL_IP, SO_ORIGINAL_DST, &sa, &len);
    close(t);

    char pbuf[1024], hbuf[1024];
    getnameinfo( (struct sockaddr *)&sa, sizeof sa, hbuf, sizeof hbuf, pbuf, sizeof pbuf, NI_NUMERICHOST | NI_NUMERICSERV);
    printf("%s:%s\n", hbuf, pbuf);

    return 0;
}
