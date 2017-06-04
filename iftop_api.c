
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "iftop_api.h"
#include "options.h"



extern options_t options ;

//pthread_mutex_t     info_mutex;
//extern int write_interval; 	//����д�ļ�������info��ʱ����
time_t write_first = 0;
time_t write_last = 0;


/**********************************************************************************************************************
 *Function
 *********************************************************************************************************************/

/* 功能：写通信信息到文件
 * 参数：
 * 返回：
 */
FILE *check_write(void)
{
    time_t cur_time = 0;
    FILE *fp = NULL;

	cur_time = time(NULL);
    if (cur_time - write_last >= options.write_interval)
    {
        options.write_interval_timeout = 1;
		if (options.write_communication_info != 0)
		{
        	fp = fopen(CI_FILEPATH, FOPEN_OPTION);
        	//check error
        	if (fp == NULL)
        	{
            	//options.write_interval_timeout = 0;       //重来
        	}
		}
    }

    return fp;
}

/* 名称: write_info
 * 功能: 写连接信息到文件
 * 参数: fd: 文件流
 * 			info: 需要的信息的结构体
 * 返回: 成功0，失败...
 */
int write_info(FILE *fp, communication_info_s *info)
{
    char info_str[300] = {0};

    snprintf(info_str, sizeof(info_str), "%s:%d <==> %s:%d TX:%s RX:%s\n",
        info->src_addr, info->src_port, info->dst_addr, info->dst_port, info->tx_total, info->rx_total);

    fwrite(info_str, strlen(info_str), 1, fp);
    //出错检测

    return 0;
}

/* 名称: close_write
 * 功能: 设置超时标志，关闭文件流
 * 参数: fd: 文件流
 * 返回: 无
 */
void close_write(FILE *fp)
{
    if (options.write_interval_timeout != 0)
    {
        options.write_interval_timeout = 0;
        write_last = time(NULL);

    }
	if (options.write_communication_info != 0 && fp != NULL)
	{
		fclose(fp);
	}

}

/* 名称：in_port_list
 * 功能：判断端口是否在列表中
 * 参数：
 *      list: 列表
 *      list_len: 列表长度
 *      port: 端口号
 * 返回： 在：1， 不在：0
 */
int in_port_list(int *list, int list_len, int port)
{
    //-----------------
    if (port == 0) {
        return 0;
    }
    //-----------------
	int i = 0;
	for (i = 0; i < list_len; i++)
	{
		//printf("list[%d] = %d, port = %d\n", i, list[i], port);
		if (list[i] == port)
		{
			return 1;
		}
		else if (list[i] == -1)
		{
			break;
		}
	}

	return 0;
}
