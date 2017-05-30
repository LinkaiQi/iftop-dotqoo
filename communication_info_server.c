/*********************************************************************************************************************
 * Include
 ********************************************************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "communication_info_server.h"
#include "iftop_api.h"
#include "ui.h"
#include "options.h"
#include "iftop.h"
//#include "threadprof.h"



/*********************************************************************************************************************
 * 全局变量
 ********************************************************************************************************************/
extern options_t options ;

int sockfd;
int cliefd;
pthread_t info_server_tid;


/* 名称：create_communication_info_server()
 * 功能：开服务器，发送信息给客户端
 * 参数：
 * 返回：一个sockfd,或-1
 */
int create_communication_info_server(void)
{
    struct  sockaddr_in   servaddr;
    struct  sockaddr_in   clieaddr;
    int     ret = 0;
    int     clieaddr_len = 0;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	//printf("socket error:sockfd = %d\n", sockfd);
        return -1;
    }

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(COMMUNICATION_INFO_PORT);

    if ((ret = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(struct sockaddr_in))) == -1)
    {
    	//printf("bind error:%s\n", strerror(errno));
        return -1;
    }

    if ((ret = listen(sockfd, 10)) == -1)
    {
    	//printf("listen error: %s\n", strerror(errno));
        return -1;
    }

    clieaddr_len = sizeof(struct sockaddr_in);
    while(1)
    {
        cliefd = accept(sockfd, (struct sockaddr *)&clieaddr, (socklen_t *)&clieaddr_len);
        if (cliefd == -1)
        {
        	//
        }
    }

    return sockfd;
}

/* block_sigpipe
 * 功能：阻塞sigpipe信号, 客户端Ctrl+c时，会发送这个信号
 * 参数：无
 * 返回：无
 */
void block_sigpipe(void)
{
    sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	sigprocmask(SIG_BLOCK, &set, NULL);
}

/* 名称: check_send
 * 功能: 检查是否需要发送info给客户端(标志是否打开)
 * 参数: 无
 * 返回: 线程创建结果
 */
int check_send(void)
{
	int ret = 0;

	if (options.send_communication_info != 0)
	{
		if (options.create_send_thread == 0)
		{
			block_sigpipe();
			ret = pthread_create(&info_server_tid, NULL, (void*)&create_communication_info_server, NULL);
			if (ret == 0) //success
			{
				options.create_send_thread = 1;
			}
			else
			{
				//
			}
		}
	}
	return ret;
}

/* 名称: close_send()
 * 功能: 关闭打开的socket fd
 * 参数: 无
 * 返回: 无
 */
void close_send(void)
{

	if (options.send_communication_info == 0 && options.create_send_thread != 0)
	{
		if (sockfd > 0)
		{
			close(sockfd);
		}
		options.create_send_thread = 0;
	}

}

/* 名称：send_info
 * 功能：发送自定义的结构
 * 参数：无
 * 返回：发送的字节数
 */
int send_info(communication_info_s info)
{
    int ret = 0;

    if((ret = send(cliefd, &info, sizeof(communication_info_s), 0)) == -1)
    {
    	//
    }
	//printf("cliefd = %d, send info: %d:%s\n", cliefd, ret, strerror(errno));

    return ret;
}
