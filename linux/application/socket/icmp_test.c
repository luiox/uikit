#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
//包的数量
#define MAX_NO_PACKETS 4
//延迟时间
#define MAX_WAIT_TIME 5
//包的大小
#define PACKET_SIZE 4096
// ICMP包中数据的长度
#define DATA_LEN 56
//当前已发送的包的数量
int nsend = 0;
//当前接收的包的数量
int nrecevd = 0;

//要发送的包
char sendPacket[PACKET_SIZE];
//接收的包
char recvPacket[PACKET_SIZE];

//接收延迟
struct timeval tvrecv;
//接收的协议地址簇
struct sockaddr_in fromAddr = {0};

//发送数据包
void send_packet(int sfd, struct sockaddr_in* pDstAddr, int len);
//打包
int pack(int nsend);
//计算校验和
u_short cal_cksum(u_short* icmp, int packsize);
//计算收发过程中的时间差
void tv_sub(struct timeval* out, struct timeval* in);
//收包
void recv_packet(int sfd);
//解包
int unpack(int len, int uid);
//定时器信号处理函数  用于迟迟接受不到返回包 结束程序
void sigalrm_hand(int n);

int  main(int argc, char* argv[])
{
    // 1. 创建socket     原始套接字
    struct protoent* protocal = getprotobyname("icmp");   //获取到icmp协议的   ip保护方式
    if (NULL == protocal) {
        printf("获取保护方式失败:%m\n"), exit(-1);
    }

    printf("获取保护方式成功:%d\n", protocal->p_proto);
    int sfd = socket(AF_INET, SOCK_RAW, protocal->p_proto);
    if (-1 == sfd) {
        printf("创建socket失败:%m\n"), exit(-1);
    }

    printf("创建socket成功!\n");
    // 2. 设置套接字选项
    int size = 50 * 1024;
    int r    = setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    if (-1 == r) {
        printf("设置接收buf失败:%m\n"), close(sfd), exit(-1);
    }
    printf("设置接收buf成功:%d\n", r);
    // 3. 获取 并 设置  接收机   ip地址
    struct sockaddr_in dst_addr = {0};
    dst_addr.sin_family         = AF_INET;

    //先当ip地址处理
    struct hostent* host  = NULL;
    in_addr_t       iAddr = inet_addr(argv[1]);
    if (INADDR_NONE == iAddr) {
        printf("%s不是个ip地址!\n", argv[1]);
        host = gethostbyname(argv[1]);
        if (NULL == host) {
            printf("输入错误!\n");
            close(sfd), exit(-1);
        }
        memcpy(&(dst_addr.sin_addr), host->h_addr, host->h_length);
        printf("ip地址为:%s\n", inet_ntoa(dst_addr.sin_addr));
    }
    else {
        printf("%s是个ip地址!\n", argv[1]);
        dst_addr.sin_addr.s_addr = iAddr;
    }

    // 4. 打包  报文   准备发送包
    // 5. 发包
    send_packet(sfd, &dst_addr, sizeof(dst_addr));

    // 6. 收包
    // 7. 解包
    recv_packet(sfd);
    // 8. 统计并显示结果
    sigalrm_hand(SIGALRM);

    return 0;
}

void send_packet(int sfd, struct sockaddr_in* pDstAddr, int len)
{
    int packSize;
    int r;
    while (nsend < MAX_NO_PACKETS) {
        nsend++;

        packSize = pack(nsend);   //打包
        //发送
        r = sendto(sfd, sendPacket, packSize, 0, (struct sockaddr*)pDstAddr, len);
        if (r < 0) {
            printf("第%d个包发送失败：%m\n", nsend);
            continue;
        }
        printf("第%d个包发送成功!\n", nsend);
        sleep(1);   //延迟
    }
}

//打包
int pack(int nsend)
{
    struct icmp* pIcmp = (struct icmp*)sendPacket;

    pIcmp->icmp_type  = ICMP_ECHO;
    pIcmp->icmp_code  = 0;
    pIcmp->icmp_cksum = 0;
    pIcmp->icmp_id    = getuid();
    pIcmp->icmp_seq   = nsend;

    int packSize = 8 + DATA_LEN;

    struct timeval* tval = (struct timeval*)pIcmp->icmp_data;

    gettimeofday(tval, NULL);

    pIcmp->icmp_cksum = cal_cksum((u_short*)pIcmp, packSize);
    return packSize;
}

//计算校验和   crc32的校验计算方法
u_short cal_cksum(u_short* icmp, int packsize)
{
    int      nleft  = packsize;
    int      sum    = 0;
    u_short* w      = icmp;
    u_short  answer = 0;


    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }


    if (1 == nleft) {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }


    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

//收包
void recv_packet(int sfd)
{
    int r;
    int fromLen = 0;
    //注册定时器信号处理函数
    signal(SIGALRM, sigalrm_hand);
    while (nrecevd < nsend) {
        alarm(MAX_WAIT_TIME);
        r = recvfrom(sfd, recvPacket, PACKET_SIZE - 1, 0, (struct sockaddr*)&fromAddr, &fromLen);
        if (r < 0) {   //没有接受到数据
            if (errno == EINTR)
                continue;
            printf("接受失败:%m\n");
            continue;
        }
        gettimeofday(&tvrecv, NULL);
        if (-1 == unpack(r, getuid()))
            continue;
        nrecevd++;
    }
}
//解包
int unpack(int len, int uid)
{
    //发送时候的时间
    struct timeval* tvsend;

    char* buf = recvPacket;
    // ip地址头的长度
    int iphdrlen;
    //最终要打印的延迟时间数
    double rtt;
    //获取ip地址头长度
    struct ip* ip;
    ip       = (struct ip*)recvPacket;
    iphdrlen = ip->ip_hl << 2;

    //往后偏移，得到SMTP地址头
    struct icmp* icmp = (struct icmp*)(buf + iphdrlen);
    len -= iphdrlen;

    if (len < 8) {
        printf("ICMP 包的大小小于8！\n");
        return -1;
    }

    if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == uid)) {
        tvsend = (struct timeval*)icmp->icmp_data;
        tv_sub(&tvrecv, tvsend);

        rtt = tvrecv.tv_sec * 1000 + tvrecv.tv_usec / 1000;
        printf("%d byte from %s： icmp_seq:%u ttl=%d rtt=%.3f ms\n",
               len,
               inet_ntoa(fromAddr.sin_addr),
               icmp->icmp_seq,
               ip->ip_ttl,
               rtt);
    }
    else {
        return -1;
    }
}

void sigalrm_hand(int n)
{
    printf("\n------------------------my ping-------------------\n");
    printf("%d个包发送成功 %d个包接收成功 %%%f 丢失!\n",
           nsend,
           nrecevd,
           ((nsend - nrecevd) * 1.0) / (nsend * 100));
    exit(1);
}

//计算收发过程中的时间差
void tv_sub(struct timeval* out, struct timeval* in)
{
    // in 58秒300微秒  out 59秒 100微秒
    if ((out->tv_usec -= in->tv_usec) < 0) {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}
