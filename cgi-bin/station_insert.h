#ifndef _STATION_INSERT_H
#define _STATION_INSERT_H

#include"/usr/include/mysql/mysql.h"

	struct f_route
	{
		int  route;//  线路编号
		int  route_index;//在线路中的编号
		int  matrix_index;// 在38阶矩阵中的编号
		int  route_value;//站点距离绝对值
	};
	
    
    struct subway_function
	{
		char ori_name[30]; //起点名
		int ori_route; //经过起点的路线号
		char ter_name[30]; //终点名
		int ter_route; //经过终点的路线号
		int array[40][40];//带权的邻接矩阵
		char stat_name[287][27];//保存所有站点名
		char flag[287][11];//保存所有点的info.text文档的数字信息
		int vexnum;	//阶数
		int ori_index; //起点编号
		int ter_index; //终点编号
	};


    typedef struct mysql{
        
        MYSQL *mysql; //定义mysql的一个指向Mysql结构体的指针
        MYSQL_ROW row; //定义一个1行数据的“类型安全”表示
        MYSQL_RES *res; //定义一个指向返回行的查询结果的结构体指针 
        const char * ip; //定义主机IP地址
        const char * user; //定义用户
        const char * passwd; //定义用户密码
        const char * db;  //定义要链接的数据库
        unsigned int port; //定义Mysql服务器的TCP/IP端口
        const char * unix_socket;//定义连接类型
        unsigned int clientflag; //定义Mysql运行成ODBC数据库的标记
    }SQL; 



	void my_err(char *string);

	void sort_route_info(struct f_route *route, int num);

	struct f_route *get_route_info(int route, int route_index,SQL *sql);

	int add_matrix(struct subway_function *sub, char *flag,SQL *sql);

	int station_insert(struct subway_function *sub, char (*flag)[10],SQL *sql);

#endif
