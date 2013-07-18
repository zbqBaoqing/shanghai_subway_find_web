#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<math.h>
#include<string.h>
#include<unistd.h>

#include"station_insert.h"


#define STATION_NUM 	286
#define FLAG_INFO	82

struct f_route route_info[10];

struct node_info
{
	int route;
	int route_index;
	int matrix_index;
}node;

void my_err(char *string)
{
	perror(string);
	exit (1);
}


void sort_route_info(struct f_route *route, int num)
{
	int i, j, k;
	for(i = 0; i < num-1; i++)
	{
		k = i;
		for(j = i+1; j < num; j++)
			if(route[k].route_value > route[j].route_value)
				k = j;
		if(k != i)
		{
			struct f_route temp;
			temp = route[k];
			route[k] = route[i];
			route[i] = temp;
		}
	}
}

struct f_route *get_route_info(int route, int route_index,SQL *sql)
{
	int 	t = 0;
//	char    read_buf[1000];
	char 	flag[FLAG_INFO][8];
	int 	f_route;
	int 	n, j = 0;
    char    *query=NULL;


    query = "select * from flag_info";

    //设置所读字符的编码格式
    mysql_query(sql->mysql,"SET NAMES 'UTF8'");
    
    //执行由query指向的SQL查询，返回0为执行成功，否则，失败
    t = mysql_real_query(sql->mysql,query,(unsigned int)strlen(query));

    if ( 0 != t ){
        printf("执行查询时出现异常:%s.<br />",mysql_error(sql->mysql));
        return NULL;
    }else {
  //      printf("[%s]构建成功!\n",query);

        //返回查询的结果集
        sql->res=mysql_store_result(sql->mysql);

        if (NULL == sql->res){
            printf("Sorry! mysql_store_result failture!<br />");
            return NULL;
        }
        n = 0;
        j = 0;
        //重复读取每行，并获取每一字段的值，直到row为NULL
        while((sql->row=mysql_fetch_row(sql->res))){
            
            strncpy(flag[n],sql->row[0],(unsigned int)strlen(sql->row[0])+1);
            f_route = (flag[n][3]-48)*10 + flag[n][4]-48;
		
            if(f_route == route)
		    {
			route_info[j].matrix_index = (flag[n][0]-48)*10 + flag[n][1]-48;
			route_info[j].route = f_route;
			route_info[j].route_index = (flag[n][5]-48)*10 + flag[n][6] -48;
			route_info[j].route_value = (int)fabs(route_info[j].route_index -route_index);
			j++;
		    }
            n++;
        }

        //释放结果集资源
        mysql_free_result(sql->res);
    }



	if (node.route == route)
	{
		route_info[j].matrix_index = node.matrix_index;
		route_info[j].route = node.route;
		route_info[j].route_index = node.route_index;
		route_info[j].route_value = (int)fabs(route_info[j].route_index - route_index);
		j++;
	}
	sort_route_info(route_info,j);
	
	return route_info;
}

int add_matrix(struct subway_function *sub, char *flag,SQL *sql)
{
	int 	i;
	int 	matrix_index;
	int 	insert_flag;
	int	route;
	int 	route_index;		//线路下标
	struct 	f_route *route_info; //线路信息
	
	
	matrix_index = (flag[0]-48)*10 + flag[1]-48;
	insert_flag = flag[2]-48;
	route = (flag[3]-48)*10 + flag[4]-48;
	route_index = (flag[5]-48)*10 + flag[6]-48;

	if(matrix_index != 0)
		return matrix_index;
	else
	{	
		route_info = get_route_info(route,route_index,sql);
		if ((route == sub->ori_route) && (( fabs(node.route_index-route_index) + fabs(route_info[0].route_index-route_index)) == fabs(node.route_index-route_info[0].route_index)))
		{
			insert_flag = 2;
			route_info[1].matrix_index = node.matrix_index;
		}
	//	printf("insert flag : %d\n",insert_flag);
	/*	for(i = 0; i < 10; i++)
			printf("%3d",route_info[i].matrix_index);
		printf("\n");
		printf("#%d\n",sub->vexnum);*/
		if(insert_flag == 1)
		{
			
			for(i = 0; i < sub->vexnum+1; i++)
			{
				sub->array[sub->vexnum][i] = 65535; 
				sub->array[i][sub->vexnum] = 65535;
			}
			sub->array[sub->vexnum][route_info[0].matrix_index-1] = route_info[0].route_value;
			sub->array[route_info[0].matrix_index-1][sub->vexnum] = route_info[0].route_value;
		}
		else if(insert_flag == 2)
		{
			for(i = 0; i < sub->vexnum+1; i++)
			{
				sub->array[sub->vexnum][i] = 65535; 
				sub->array[i][sub->vexnum] = 65535;
			}
			sub->array[sub->vexnum][route_info[0].matrix_index-1] = route_info[0].route_value;
			sub->array[route_info[0].matrix_index-1][sub->vexnum] = route_info[0].route_value;

			sub->array[sub->vexnum][route_info[1].matrix_index-1] = route_info[1].route_value;
			sub->array[route_info[1].matrix_index-1][sub->vexnum] = route_info[1].route_value;
		}
		sub->vexnum++;
		
	
	}
//	printf("vexnnm : %d\n", sub->vexnum);
	return sub->vexnum;
}

int station_insert(struct subway_function *sub, char (*flag)[10], SQL *sql)
{
	int 	t= 0, num_fields = 0;
	int 	i,  n;
//	char    read_buf[10];
//	char 	num[7];
    char    *query=NULL;

    	sub->vexnum = 38;

        query = "select * from 38_station";
        
        //设置所读字符的编码格式
        mysql_query(sql->mysql,"set names 'utf8'");

    //执行由query指向的SQL查询，返回0为执行成功，否则，失败 
    t = mysql_real_query(sql->mysql,query,(unsigned long)strlen(query));

    if (0 != t){
        printf("执行查询时出现异常:%s.<br />",mysql_error(sql->mysql));
        return 1;
    }else {
     //   printf("[%s]构建成功!\n",query);

        //返回查询的结果集
        sql->res = mysql_store_result(sql->mysql);

        if (NULL == sql->res){
            printf("Sorry! mysql_store_result failture!<br />");
            return 1;
        }
        
            //获取结果集中的列数
            num_fields = mysql_num_fields(sql->res);

         //   printf("num_fields:%d\n",num_fields);    

        n = 0;
        while( (sql->row = mysql_fetch_row(sql->res))){
            
            for (i = 0; i < num_fields; i++){
                
			    sub->array[n][i] = atoi(sql->row[i]);
            }

            n++;
        }

        //释放结果集资源
        mysql_free_result(sql->res);
    }
    




	node.route = 0;
	sub->ori_index = add_matrix(sub,flag[0],sql);
	node.route = (flag[0][3]-48)*10 + flag[0][4]-48;
	node.route_index = (flag[0][5]-48)*10 + flag[0][6]-48;
	node.matrix_index = sub->ori_index;
	sub->ori_route = (flag[0][7]-48)*10 + flag[0][8]-48;
	sub->ter_index = add_matrix(sub, flag[1],sql);
	sub->ter_route = (flag[1][7]-48)*10 + flag[1][8]-48;


	return 0;
}
