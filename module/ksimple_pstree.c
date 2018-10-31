#include<linux/pid.h>
#include<linux/module.h>
#include<linux/sched.h>
#include<net/netlink.h>
#include<linux/skbuff.h>
#include<net/sock.h>
#include<linux/printk.h>
#define MAX_LENGTH 12000

MODULE_LICENSE("GPL");

void DFS(struct task_struct *t,int i);
void SIB(struct task_struct *t,int i);
void PAR(struct task_struct *t,int i);

static char output_msg[MAX_LENGTH];
static struct sock *netlink_sock = NULL;
static unsigned lens = 0;
static int ww = -1;

static void reply_user_query(struct sk_buff *skb)
{
    ww = -1;
    struct nlmsghdr *nlh;
    struct nlmsghdr *nlh_out;
    struct sk_buff *skb_out;
    int pid;
    int err;
    char *ptr;
    lens = 0;
    long qaq=0;
    nlh = (struct nlmsghdr*)skb->data;
    pid = nlh->nlmsg_pid;
    ptr = NLMSG_DATA(nlh);
    memset(output_msg, 0, MAX_LENGTH);
    kstrtol(ptr+2,10,&qaq);
    if(find_get_pid(qaq)!=NULL) {
        switch(ptr[1]) {
        case 'c':
            DFS( pid_task(find_get_pid(qaq),PIDTYPE_PID),0);
            break;
        case 's':
            SIB( pid_task(find_get_pid(qaq),PIDTYPE_PID),0);
            break;
        case 'p':
            PAR( pid_task(find_get_pid(qaq),PIDTYPE_PID),0);
            break;
        }
    } else {
        output_msg[0]='\0';
    }
    skb_out = nlmsg_new(MAX_LENGTH, 0);
    nlh_out = nlmsg_put(skb_out, 0, 1, NLMSG_DONE, MAX_LENGTH, 0);
    memcpy(nlmsg_data(nlh_out), output_msg, MAX_LENGTH);
    err = nlmsg_unicast(netlink_sock, skb_out, pid);
    if(err < 0) {
        printk(KERN_ERR "Unicast failed\n");
    }
}
static int __init init(void)
{
    printk(KERN_INFO "ksimple_pstree: Module initializing\n");
    memset(output_msg, 0, MAX_LENGTH);
    struct netlink_kernel_cfg cfg = {
        .input = reply_user_query,
    };
    netlink_sock = netlink_kernel_create(&init_net, NETLINK_USERSOCK, &cfg);
    if(!netlink_sock) {
        printk(KERN_ERR "ksimple_pstree: Create netlink socket error.\n");
        return -1;
    }
    printk(KERN_ERR "ksimple_pstree: Create netlink socket ok.\n");
    printk(KERN_ERR "ksimple_pstree: Module initialization finished\n");
    return 0;

}


void DFS(struct task_struct *in,int i)
{
    struct list_head *l=NULL;
    struct task_struct *t;
    lens += snprintf(output_msg + lens, MAX_LENGTH - lens, "%*s%s(%d)\n", i*4, "", in->comm, in->pid);
    list_for_each(l,&in->children) {
        t = list_entry(l, struct task_struct,sibling);
        DFS(t,i+1);
    }

}

void SIB(struct task_struct *in,int i)
{
    struct list_head *l=NULL;
    struct task_struct *t;
    list_for_each(l,&in->parent->children) {
        t = list_entry(l, struct task_struct,sibling);
        if(in != t) {
            lens += snprintf(output_msg + lens, MAX_LENGTH - lens, "%s(%d)\n", t->comm, t->pid);
        }
    }
}

void PAR(struct task_struct *in,int i)
{
    ww += 1;
    if((in->parent)!=in) {
        PAR(in->parent,i+1);
        lens += snprintf(output_msg + lens, MAX_LENGTH - lens, "%*s%s(%d)\n", (ww-i-1)*4, "", in->comm, in->pid);
    } else {
        return;
    }

}

static void __exit end(void)
{
    sock_release(netlink_sock->sk_socket);
    return;
}

module_init(init);
module_exit(end);

