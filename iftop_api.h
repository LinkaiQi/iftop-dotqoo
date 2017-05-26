#ifndef _IFTOP_API_H_
#define _IFTOP_API_H_

#include <stdio.h>

#include "iftop_api.h"

/**********************************************************************************************************************
 * Define
 *********************************************************************************************************************/
#define CI_FILEPATH     "/tmp/communication_info"
#define FOPEN_OPTION    "w+"


/**********************************************************************************************************************
 * Struct
 *********************************************************************************************************************/
typedef struct COMMUNICATION_INFO_S
{
    char                    src_addr[128];
    char                    dst_addr[128];
    unsigned short int      src_port;
    unsigned short int      dst_port;
    char                    tx_total[10];
    char                    rx_total[10];
}communication_info_s;


/* 功能：写通信信息到文件
 * 参数：
 * 返回：
 */
FILE *check_write(void);

/* 名称: write_info
 * 功能: 写连接信息到文件
 * 参数: fd: 文件流
 * 			info: 需要的信息的结构体
 * 返回: 成功0，失败...
 */
int write_info(FILE *fp, communication_info_s *info);

/* 名称: close_write
 * 功能: 设置超时标志，关闭文件流
 * 参数: fd: 文件流
 * 返回: 无
 */
void close_write(FILE *fp);

/* 名称：in_port_list
 * 功能：判断端口是否在列表中
 * 参数：
 *      list: 列表
 *      list_len: 列表长度
 *      port: 端口号
 * 返回： 在：1， 不在：0
 */
int in_port_list(int *list, int list_len, int port);


#endif
