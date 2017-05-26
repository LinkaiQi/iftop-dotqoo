#ifndef COMMUNICATION_INFO_SERVER_H_
#define COMMUNICATION_INFO_SERVER_H_ 

#include "iftop_api.h"

#define COMMUNICATION_INFO_PORT         22222

/* 名称：create_communication_info_server
 * 功能：创建一个tcp服务器，供客户端连接
 * 参数：无
 * 返回：sockfd,或-1
 */
int create_communication_info_server(void);

/* 名称: check_send
 * 功能: 检查是否需要发送info给客户端(标志是否打开)
 * 参数: 无
 * 返回: 线程创建结果
 */
int check_send(void);

/* 名称: close_send()
 * 功能: 关闭打开的socket fd
 * 参数: 无
 * 返回: 无
 */
void close_send(void);

/* 名称：send_info
 * 功能：发送自定义的结构
 * 参数：无
 * 返回：发送的字节数
 */
int send_info(communication_info_s info);

/* 名称：block_sigpipe
 * 功能：阻塞sigpipe信号, 客户端Ctrl+c时，会发送这个信号
 * 参数：无
 * 返回：无
 */
void block_sigpipe(void);

#endif 
