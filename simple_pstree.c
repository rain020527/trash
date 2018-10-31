#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_PAYLOAD 12000

int main(int argc, char *argv[])
{
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    int sock_fd;
    struct msghdr msg;
    char string[100];
    char s[100];
    if(argv[1] == NULL) {
        string[0]='-';
        string[1]='c';
        string[2]='1';
    } else if(argv[1][0]!='-') {
        string[0]='-';
        string[1]='c';
        strcat(string,argv[1]);
    } else if(argv[1][2]=='\0') {
        string[0] = argv[1][0];
        string[1] = argv[1][1];
        if(argv[1][1] == 'c') {
            sprintf(s, "%d", 1);
        } else {
            sprintf(s, "%d", getpid());
        }
        strcat(string,s);
    } else {
        strcpy(string,argv[1]);
    }
    //************************************

    //************************************
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);

    if(sock_fd < 0) {
        puts("Socket initialization failed");
        return -1;
    }
    memset(&src_addr, 0, sizeof(src_addr));

    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();  /* self pid */

    /* interested in group 1<<0 */
    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;   /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    strcpy(NLMSG_DATA(nlh), string);// ****************************
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    //printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);
    //printf("Waiting for message from kernel\n");

    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    //printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));
    printf("%s", (char *)NLMSG_DATA(nlh));
    close(sock_fd);
}
