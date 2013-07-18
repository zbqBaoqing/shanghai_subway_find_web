#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include<signal.h>
#include<time.h>


#include "station_insert.h"


#define	SERV_PORT			5510
#define LISTENQ				12
#define STATION_NUM 			286
#define USERNAME			0
#define PASSWORD			1

#define MAX_VERTEX_NUM   		40
#define INFINITY   			10000
//递归调用求各站点间是否有直达路线
int same_station(int same_stat[][5], int train[][5], int q, int t,int a[],int m)
{
	int 	i, j, n, r, k;
	int	z;


	for(r = t ; r >= q; r--)
	{ 
		n = 0;
		for(i = 0; i < 5; i++)   //判断是否有直达的路线,如果有多路直达则都保存下来
		{
			for(j = 0; j < 5; j++)
			{
        

				if((train[q][i] == train[r][j]) && train[r][j] != 0)
				{
					same_stat[r][n] = train[r][j];  //记录起始站至终点中第r站与起点直达的路线号，如果多条直达都记录
                    n++;
					break;
				}
				
			}
            
			if(train[q][i] == 0)//如果没有到达q站点的路
				break;
		}

		for(z = n ; z < 5; z++)   //数组不够5位0补齐
			same_stat[r][z] = 0;

		if(same_stat[r][0] != 0)
		{
			k = r;   //记录起始站至终点站能直达的站点的位置
			a[m++] = k;
			break;
		}
	}
	if(k != t && k != q - 1 )
	{
		
		same_station(same_stat, train, k, t, a,m);
	}
	else
	{
		
		a[m]=99;
		return 0;
	}
        
    return 0;
}




//根据由最短路径函数传来的路线矩阵和终点名解析出路线
char *road(struct subway_function *st, int D[], int road_sort[],SQL *sql)
{
	char	send_buf[1024],*buf = send_buf;
	int	    i, j, t;
	int	    k, n, r;
	char	ch[5];
	char	stat_name[40][30]; //存放站名
	int	    num[40]={0};	//存放站点编号
	int	    inod[40][5]; //存放站点经过的线号
	int	     a[40] = {0};
    char    *query=NULL;
    

   query="select * from sub_info";

    //设置所读字符的编码格式
    mysql_query(sql->mysql,"SET NAMES 'UTF8'");

    //执行由query所指向的SQL查询，返回0为执行成功，否则，失败
    t = mysql_real_query(sql->mysql,query,(unsigned int)strlen(query));

    if (0 != t){
        printf("执行查询时出现异常:%s.<br />",mysql_error(sql->mysql));
        return NULL;
    } else {

      //  printf("[%s]构建成功!\n",query);

        // 返回查询的结果集
        sql->res = mysql_store_result(sql->mysql);

        if ( NULL  == sql->res){
            printf("Sorry!mysql_store_result failure!<br />");
            return NULL;
        }
        n = 0;
        while((sql->row=mysql_fetch_row(sql->res))){

            num[n]=atoi(sql->row[0]);// 保存站点顺序编号
            strncpy(stat_name[n],sql->row[1],(unsigned int)strlen(sql->row[1])+1);
           // printf("%d %s ",num[n],stat_name[n]);
            inod[n][0]=atoi(sql->row[2]);
            inod[n][1]=atoi(sql->row[3]);
            inod[n][2]=atoi(sql->row[4]);
            inod[n][3]=atoi(sql->row[5]);
            inod[n][4]=atoi(sql->row[6]);
         //   printf("%d %d %d %d %d\n",inod[n][0],inod[n][1],inod[n][2],inod[n][3],inod[n][4]);
            n++;
        }
        //释放结果集资源
        mysql_free_result(sql->res);
    } 


    
//    printf("进入路进解析\n");
	if(st->ori_index == 39)
	{
		num[38] = st->ori_index;
		strcpy(stat_name[38], st->ori_name);
		inod[38][0] = st->ori_route;
	//	printf("ori_routet =%d\n", st->ori_route);
		for(i = 1; i < 5; i++)
		{
			inod[38][i] = 0;
		}
		if(st->ter_index == 40)
		{
			num[39] = st->ter_index;
			strcpy(stat_name[39], st->ter_name);
			inod[39][0] = st->ter_route;		
			for(i = 1; i < 5; i++)
			{
				inod[39][i] = 0;
			}
		}
	}
	else if(st->ter_index == 39)
	{
		num[38] = st->ter_index;
		strcpy(stat_name[38], st->ter_name);
		inod[38][0] = st->ter_route;
		for(i = 1; i < 5; i++)
		{
			inod[38][i] = 0;
		}
	}

	//顺序保存站点名及其线号数
	n = 0;
	char 	station[st->vexnum][30];
	int	train[st->vexnum][5];

	for(i = 0; road_sort[i] != 100; i++ )
	{
		n =  road_sort[i];
	//	printf("n:%d\n",n);
		strcpy(station[i], stat_name[n - 1]);
		for( j = 0; j < 5; j++)
		{
			train[i][j] = inod[n - 1][j];  //记录第n站点的所有经过它的不同线路号
		}

	}
	k = i - 1 ;

	int	same_stat[40][5]; //用来保存站点间有相同线路的号数


	 same_station(same_stat, train, 0, k, a,0);  //函数调用

             

	strcpy(buf,"*******您要经过的各换乘站点如下*******<br /><br />");
	
	//输出要经过的各站点

	for(i = 0; road_sort[i] != 100 ; i++)
	{
		strncat(buf, station[i],strlen(station[i]));
		if(i < k)
		strcat(buf, " --> ");
	}

	strcat(buf,"<br /><br />全程共经过 ");

	sprintf(ch, "%d", D[st->ter_index - 1]);
	strcat(buf, ch);
	strcat(buf, "个站点<br /><br  />");
	strcat(buf, "<br\t*******您的换乘方式如下*******<br /><br /> ");

  	strcat(buf, station[0]);
	strcat(buf, "站");
	//输出换乘方式
	for(r = 0; a[r] != 99 ; r++)
	{
		i = a[r];
		strcat(buf, " ***乘坐 ");
		for(j = 0; j < 5; j++)
		{
			if(same_stat[i][j] != 0)
			{
				sprintf(ch, "%d", same_stat[i][j]);
				strcat(buf, ch);
			}
			if(j < 4 && same_stat[i][j+1] != 0)
			{
				strcat(buf," , ");
			}

		}
		strcat(buf," 路车*** ");
		strcat(buf, station[i]);
		strcat(buf, "站");
	}
	strcat(buf, "<br /><br />到达目的地，谢谢您的使用！<br /><br />如有不满意的地方，请拨打: 120110 进行投诉！<br /><br />*******再见！******<br />");
//	printf("%s\n", buf);
	return buf;

}

// 迪杰特斯拉求最短路径，并根据P解析路线
// 用Dijkstra算法求有向网G的v0顶点到其余顶点v的最短路径P[v]及带权长度 
// D[v]。若P[v][w]为1,则w是从v0到v当前求得最短路径上的顶点。 
// final[v]为1当且仅当v∈S,即已经求得从v0到v的最短路径
char * ShortestPath_DIJ(struct subway_function *A, SQL *sql)
{ 

	char *buf;
	int v,w,i,j,min,s ,k=1, t;
	int P[MAX_VERTEX_NUM][MAX_VERTEX_NUM];
	int D[MAX_VERTEX_NUM];
	int final[MAX_VERTEX_NUM];
 	int path[MAX_VERTEX_NUM +1][1], a[MAX_VERTEX_NUM];
	
	for(v=0;v<A->vexnum;++v)
	{
		final[v]=0;
		D[v]=A->array[A->ori_index-1][v];
		for(w=0;w < A->vexnum;++w)
		   	P[v][w]=0; // 设空路径 
		if(D[v]<INFINITY)
		{
			P[v][A->ori_index - 1]=1;
			P[v][v]=1;
		}
	
	}
	//printf("vex = %d\t",A.vexnum);
	D[A->ori_index  -1]=0;
	final[A->ori_index - 1]=1; // 初始化,v0顶点属于S集 
	for(i=0;i<A->vexnum - 1;i++) // 其余G.vexnum-1个顶点 
	{ // 开始主循环,每次求得v0到某个v顶点的最短路径,并加v到S集 
		s = 0;
		min=INFINITY; // 当前所知离v0顶点的最近距离 
		for(w=0;w<A->vexnum;++w)
			if(!final[w]) // w顶点在V-S中 
			if(D[w]<min)
			{
				v=w;
				min=D[w];
			} // w顶点离v0顶点更近 
		final[v]=1; // 离v0顶点最近的v加入S集 		
		path[v+1][0]=k;
		k++;

		for(w=0;w<A->vexnum;++w) // 更新当前最短路径及距离 
		{
			if(!final[w] && (min+A->array[v][w]<D[w]))
			{
				// 修改D[w]和P[w],w∈V-S 
				D[w]=min+A->array[v][w];
				for(j=0;j<A->vexnum;++j)
				{
					P[w][j]=P[v][j];
				}
					P[w][w]=1;	
						}//if
		}//for
	
	}
	path[A->ori_index][0] = 0;
	s = A->ter_index -1;
	t = 0;
	  for(i = 0; i < A->vexnum; i++)
	  {
		  if(P[s][i] == 1)
		  {
			a[t] =  path[i+1][0];
			t++;
		  }
	  }

	  int  road_sort[t]; //保存站点顺序编号

	  //对编号由小到大排序
	  for(i = 0; i < t; i++)
	  {
		 s= i;
		 for(j = i+1; j < t; j++)
		 {
			 if(a[s] > a[j])
			 {
				 s = j;
			 }
		 }

		 if( s != i)
		 {
			 v = a[s];
			 a[s] = a[i];
			 a[i] = v;

		 }
	  }
	  for(i = 0; i < t; i++)
	  {
		for( w = 1; w <= A->vexnum; w++)
		{
			if(path[w][0] == a[i])
			{
				road_sort[i] = w;  
				break;
			}
		}
	  }
	  
	  road_sort[t] = 100;
/*	  printf("this is road_sort[]:\n");
	printf("\nthis is P:\n   1234567890123456789012345678901234567890\n");
	for(i = 0; i < A->vexnum; i++)
	{	printf("%2d:",i+1);
		for(j = 0; j < A->vexnum; j++)
			printf("%d",P[i][j]);
		printf("\n");
	}
		printf("###########     迪杰特斯拉    [OK]\n");*/
	
	    buf = road(A, D, road_sort,sql); //函数调用
	    return (buf);
}



int find_name(struct subway_function *st, SQL *sql)
{
	int    	n = 0 ,t = 0;
    char    *query =NULL;
    
    query = "select * from all_station";

    //设置所读字符的编码格式
    mysql_query(sql->mysql, "SET NAMES 'UTF8'");

    //执行由query指向的SQL查询，返回0为执行成功，否则，失败
    t = mysql_real_query(sql->mysql,query,(unsigned long)strlen(query));
if (0 != t){

        printf("<p>执行查询时出现异常:%s.</p>",mysql_error(sql->mysql));

        return 1;
    }else {
       // printf("[%s]构建成功！\n",query);
        
        //返回查询的结果集
        sql->res = mysql_store_result(sql->mysql);
    
        if (NULL == sql -> res){
            printf("<p>sorry! mysql_store_result failure!</p>");
            return 1;
        }
    
        //获取结果集中的列数
    //    num_fields = mysql_num_fields(sql->res);
        n = 0;
        //重复读取行，并获取每一字段的值，直到row 为NULL
        while ((sql->row = mysql_fetch_row(sql->res))){
			    
                strncpy(st->stat_name[n],sql->row[0], (unsigned int)strlen(sql->row[0])+1); //保存所有站点的名称

                strncpy(st->flag[n],sql->row[1],(unsigned int)strlen(sql->row[1])+1); //保存所有站点的信息

              //  printf("name= %s\n",st->stat_name[n]);
            n++;
        }

        // 释放结果集资源
        mysql_free_result(sql->res);
    }


	return 0;
}

//对从web表单上接收到的汉字进行解码 ,因为所传来的汉字是以%AE%B4%C3十六进制的形式
//所以要把这十六进制的数转成十进制的ascii码

int to_char(char ch)
{
    switch (ch) {
    case 'A': return 10;
    case 'B': return 11;
    case 'C': return 12;
    case 'D': return 13;
    case 'E': return 14;
    case 'F': return 15;
    default : return ch-'0';
    }
}

//web_name1,web_name2指的是web网页上input< name="name1"> input< name="name2">, name1,2是标签
int to_string(char *src_str,char *web_name1 ,char *web_name2,char name[][26])
{
    char *to_str = (char *)malloc(sizeof(char)*(strlen(src_str)+1));
    char *str = to_str;
    char *str_name1, *str_name2;
     
	
    while (*src_str) {
        if (*src_str == '%') {
            src_str++;
            *str = to_char(*src_str++)*16;
            *str += to_char(*src_str++);
            str++;
        } else {
            *str++ = *src_str++;
        }
    }
    *str = '\0';
  /* 解析字符串 */
    str_name1 = to_str;
    str_name1 += strlen(web_name1) + 1;
    str_name2 = strstr(to_str, "&");
    *str_name2 = '\0';
    str_name2 += strlen(web_name2) + 2;

  //去掉age后面没用的字符信息
    str=str_name2;

    while('&' != *str){
        str++;
        continue;
    }
    *str='\0';


	strncpy(name[0],str_name1,(int)strlen(str_name1)+1);
	strncpy(name[1],str_name2,(int)strlen(str_name2)+1);	

//	printf("%s=%s<br />%s=%s<br />",web_name1, name[0],web_name2,name[1]);
    free(to_str);
    return 0;
}





void dispose_function(struct subway_function *info,SQL *sql,char *data,char *web_name1 ,char *web_name2)
{

	int n;
	char *send_buf;
	char name[2][26], tex_flag[2][10];
    FILE *fp;
    char buf[1000];
	
	if (NULL == data){
		printf("<p>亲！你还没告诉我你的站点信息!</p>");
		return;
	}else {
		to_string(data,web_name1,web_name2,name); //解析web传来的中文站点名
	}
    if (NULL == name[0]){
        printf("<p> 亲!你不想让我知道你的出发点吗?</p>");
        return;
    }
    if (NULL == name[1]){
        printf("<p>亲!你还没想好要去哪吗?</p>");
        return;
    }
	
	

	for(n = 0; n < STATION_NUM; n++)	//查找 站点
		if(strcmp(info->stat_name[n],name[0]) == 0)
			break;
	if(n == STATION_NUM)
	{
        printf("<p style='color:red'>无%s站点信息！\n",name[0]);
	}
	else
	{
		strcpy(info->ori_name, name[0]);
		strcpy(tex_flag[0],info->flag[n]);
	}

	for(n = 0; n < STATION_NUM; n++)	//查找 站点
		if(strcmp(info->stat_name[n],name[1]) == 0)
			break;
	if(n == STATION_NUM)
	{
        printf("<p style='color:red'>无%s站点信息！<p>\n",name[1]);

	}
	else
	{
		strcpy(info->ter_name, name[1]);
		strcpy(tex_flag[1],info->flag[n]);
	}

	//printf("station: %s   %s\n",name[n],flag[n]);
//	printf("%s %s\n",info->ori_name,tex_flag[0]);
//	printf("%s %s\n", info->ter_name,tex_flag[1]);

       station_insert(info, tex_flag,sql); //函数调用

//	printf("route: %d  %d \nvexnum: %d\n",info->ori_route,info->ter_route,info->vexnum);
	//printf("###########     构建图    [OK]\n");

	send_buf = ShortestPath_DIJ(info,sql);//函数调用

     //显示cgi跳转后页面设置
     
        fp = fopen("beijing.html","r");
        
        fgets(buf,1000,fp);        
    
        while(!feof(fp)){
            printf("%s",buf);
            fgets(buf,1000,fp);
        }
    
       printf("<div class=\"print\">%s<br /></div>",send_buf);
       printf("<div class=\"print\"> <a href=\"../index.html\">上一页</a></div></body></html>");

    return;
}



	//mysql数据库的连接

int connect_mysql(SQL *sql){

    sql -> user = "root";
    sql -> passwd = "zbqacer";
    sql -> db = "subway";
    sql -> ip = "127.0.0.1";
    sql -> port = 0;
    sql -> unix_socket = NULL;
    sql -> clientflag = 0;

    //初始化mysql变量
    sql->mysql = mysql_init(NULL);

    //连接mysql数据库
    if (! mysql_real_connect(sql->mysql, sql->ip, sql->user,sql->passwd,sql->db,sql->port,sql->unix_socket,sql->clientflag)){
          fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(sql->mysql));
    }else{
     //   printf("数据库连接成功!\n");
    }


    return 0;
}



int main(void)
{
	
	struct subway_function st,*info = &st;
	char *data;
  	SQL sql ; //定义一个mysql结构体的变量
	char *web_name1,*web_name2;

	//web_name1,web_name2指的是web网页上input< name="name1"> input< name="name2">, name1,2是标签
	web_name1="org_name"; //起始站点标签	
	web_name2="ter_name";//终点站标签

	
	printf("Content-type:text/html; charset=utf-8\n\n");

	data = getenv("QUERY_STRING");//获得web表单传来的参数
    
	
       	connect_mysql(&sql); //mysql数据库的连接

    	find_name(info,&sql); //函数调用，保存所有站点的信息
	
	dispose_function(info, &sql, data,web_name1,web_name2);	
	
   	 //关闭mysql数据库
   	mysql_close(sql.mysql);

	return 0;
}

