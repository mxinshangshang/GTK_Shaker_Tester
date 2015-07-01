#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define _LINUX_ 1

#ifdef _WIN32_
#include <windows.h>
#include <winsock2.h>
#include "mysql.h"
#endif

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <mysql/mysql.h>
#endif

#define SERVER_HOST "localhost"
#define SERVER_USER "root"
#define SERVER_PWD  "12345"

#define DB_NAME     "fatigue_test_db"
#define TABLE_NAME  "mytables"

int check_tbl(MYSQL* mysql,char *name);
int check_db(MYSQL *mysql,char *db_name);

gint **datas;
gint num=0;

GSocket *sock;
gint issucceed=-1;
#define MAXSIZE 2048
GtkTextBuffer *show_buffer,*input_buffer;
gboolean timer = TRUE;
static cairo_surface_t *surface = NULL;

gdouble width, height;
gint top_y=50,top_x=50;
gdouble big_sp,small_sp;
gint biggest=0;
gdouble Blank=25;
gint last_point[8];
gint next=25;

struct EntryStruct
{
    GtkEntry *IP;
	GtkEntry *Port;
};

struct EntryStruct1
{
    GtkEntry *DA1;
	GtkEntry *DA2;
	GtkEntry *D0;
	GtkEntry *PWM;
	GtkEntry *PMW_Duty;
	GtkEntry *PWM_DIR;
};

int init_db()
{
    int err=0;
    MYSQL mysql;

    if(!mysql_init(&mysql))
    {
        g_print("mysql_init:");
        exit(1);
    }

    if(!mysql_real_connect(&mysql,SERVER_HOST,SERVER_USER,SERVER_PWD,NULL,0,NULL,0))
    {
        g_print("mysql_real_connect");
        exit(1);
    }

    err = check_db(&mysql,DB_NAME);//use check_db()
    if(err != 0)
    {
        g_print("create db is err!\n");
        mysql_close(&mysql);
        exit(1);
    }
    //select which db
    if(mysql_select_db(&mysql,DB_NAME)) //return 0 is success ,!0 is err
    {
        g_print("mysql_select_db:");
        mysql_close(&mysql);
        exit(1);
    }
    if((err=check_tbl(&mysql,TABLE_NAME))!=0)//use check_tbl()
    {
        g_print("check_tbl is err!\n");
        mysql_close(&mysql);
        exit(1);
    }
    mysql_close(&mysql);
    return 0;
}

int check_db(MYSQL *mysql,char *db_name)
{
    MYSQL_ROW row = NULL;
    MYSQL_RES *res = NULL;

    res = mysql_list_dbs(mysql,NULL);
    if(res)
    {
        while((row = mysql_fetch_row(res))!=NULL)
        {
            g_print("db is %s\n",row[0]);
            if(strcmp(row[0],db_name)==0)
            {
                g_print("find db %s\n",db_name);
                break;
            }
        }
        //mysql_list_dbs会分配内存，需要使用mysql_free_result释放
        mysql_free_result(res);
    }
    if(!row)  //没有这个数据库，则建立
    {
        char buf[128]={0};
        strcpy(buf,"CREATE DATABASE ");
        strcat(buf,db_name);
        if(mysql_query(mysql,buf)){
        	g_print("Query failed (%s)\n",mysql_error(mysql));
            exit(1);
        }
    }
    return 0;
}

int check_tbl(MYSQL* mysql,char *name)
{
    if(name == NULL)
        return 0;
    MYSQL_ROW row=NULL;
    MYSQL_RES *res = NULL;
    res = mysql_list_tables(mysql,NULL);
    if(res)
    {
        while((row = mysql_fetch_row(res))!=NULL)
        {
            g_print("tables is %s\n",row[0]);
            if(strcmp(row[0],name) == 0)
            {
                g_print("find the table !\n");
                break;
            }
        }
        mysql_free_result(res);
    }
    if(!row) //create table
    {
        char buf[1024]={0};
        char qbuf[1024]={0};
        snprintf(buf,sizeof(buf),"%s (SN INT(10) AUTO_INCREMENT NOT NULL,pulse1 INT(10),pulse2 INT(10),pulse3 INT(10),AD1 INT(10),AD2 INT(10),AD3 INT(10),AD4 INT(10),DI INT(10),PRIMARY KEY (SN));",TABLE_NAME);
        strcpy(qbuf,"CREATE TABLE ");
        strcat(qbuf,buf);
        if(mysql_query(mysql,qbuf)){
        	g_print("Query failed (%s)\n",mysql_error(mysql));
            exit(1);
        }
    }
    return 0;
}

void send_to_mysql(gint rcvd_mess[])
{
	gchar sql_insert[200];
    MYSQL my_connection;
    gint res;

    mysql_init(&my_connection);
    if (mysql_real_connect(&my_connection,SERVER_HOST,SERVER_USER,SERVER_PWD,DB_NAME,0,NULL,0))
    {
    	sprintf(sql_insert, "INSERT INTO mytables(pulse1,pulse2,pulse3,AD1,AD2,AD3,AD4,DI) VALUES('%d','%d','%d','%d','%d','%d','%d','%d')",rcvd_mess[0],rcvd_mess[1],rcvd_mess[2],rcvd_mess[3],rcvd_mess[4],rcvd_mess[5],rcvd_mess[6],rcvd_mess[7]);
        res = mysql_query(&my_connection, sql_insert);

        if (!res)
        {
        	g_print("Inserted %lu rows\n", (unsigned long)mysql_affected_rows(&my_connection));
        }
        else
        {
            fprintf(stderr, "Insert error %d: %s\n", mysql_errno(&my_connection),
            mysql_error(&my_connection));
        }

        mysql_close(&my_connection);
    }
    else
    {
        if (mysql_errno(&my_connection))
        {
            fprintf(stderr, "Connection error %d: %s\n",
            mysql_errno(&my_connection), mysql_error(&my_connection));
        }
    }
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
draw_configure_event (GtkWidget         *widget,
                      GdkEventConfigure *event,
                      gpointer           data)
{
  GtkAllocation allocation;
  cairo_t *cr;

  if (surface)
    cairo_surface_destroy (surface);

  gtk_widget_get_allocation (widget, &allocation);
  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               allocation.width,
                                               allocation.height);

  /* Initialize the surface to white */
  cr = cairo_create (surface);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

/* Redraw the screen from the surface */
static gboolean
draw_callback (GtkWidget *widget,
               cairo_t   *cr,
               gpointer   data)
{
	gdouble i=0,x=0,y=0;
	gint j=0,x_o;
	gchar c[1];
	gint recv[8];
	for(j=0;j<8;j++)
	{
		recv[j]=0;
	}

 	width = gtk_widget_get_allocated_width (widget);
  	height = gtk_widget_get_allocated_height (widget);

  	cairo_set_source_surface (cr, surface, 0, 0);
   	cairo_paint (cr);

	big_sp=(height-2*Blank)/10;
	small_sp=(height-2*Blank)/top_y;

	if(num>=1)
	{
		recv[0]=datas[num-1][0];
		recv[1]=datas[num-1][1];
		recv[2]=datas[num-1][2];
		if(recv[0]>recv[1])
		{
			if(recv[2]>recv[0]) biggest=recv[2];
			else biggest=recv[0];
		}
		else
		{
			if(recv[2]>recv[1]) biggest=recv[2];
			else biggest=recv[1];
		}
	}
	else biggest=50;
    if(biggest>=top_y)
	{
		top_y=biggest/50*50+50;
		big_sp=(height-2*Blank)/10;
		small_sp=(height-2*Blank)/top_y;
	}

   	cairo_set_source_rgb(cr,0,0,0);
	cairo_set_line_width(cr,0.5);
	cairo_rectangle (cr,Blank, Blank, width-2*Blank, height-2*Blank);

	for(i=height-Blank;i>=Blank;i=i-big_sp)//Y
	{
		cairo_move_to(cr,Blank-6,i);
		cairo_line_to(cr,width-Blank,i);
		cairo_move_to(cr,Blank-16,i);
		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, 12.0);
		gcvt(y, 4, c);
		y=y+top_y/10;
		cairo_show_text(cr,c);
	}
	for(i=height-Blank;i>=Blank;i=i-small_sp)
	{
		cairo_move_to(cr,Blank-3,i);
		cairo_line_to(cr,Blank,i);
	}
	if(num>700)
	{
		x=((num-700)/100+1)*100;
	}
	for(i=Blank;i<(width-Blank);i=i+100)//X
	{
		cairo_move_to(cr,i,Blank);
		cairo_line_to(cr,i,height-Blank+6);
		cairo_move_to(cr,i,height-Blank+16);
		cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, 12.0);
		gcvt(x, 4, c);
		x=x+100;
		cairo_show_text(cr,c);
	}
	for(i=Blank;i<(width-Blank);i=i+10)
	{
		cairo_move_to(cr,i,height-Blank);
		cairo_line_to(cr,i,height-Blank+3);
	}
    cairo_stroke(cr);

    if(datas[0]!=NULL)
    {
    	for(j=0;j<8;j++)
    	{
    		last_point[j]=0;
    	}

		if(num>700)
		{
			x_o=((num-700)/100+1)*100;
		}
		else x_o=0;

		next=24;
		for(j=x_o;j<num;j++)
		{
	       	cairo_set_source_rgb(cr,0,1,0);
	    	cairo_set_line_width(cr,1.5);
			recv[0]=datas[j][0];
			cairo_move_to(cr,next,height-Blank-last_point[0]*small_sp);
			next++;
			cairo_line_to(cr,next,height-Blank-recv[0]*small_sp);
			last_point[0]=recv[0];
		    cairo_stroke(cr);

		    next--;
	       	cairo_set_source_rgb(cr,1,0,0);
	    	cairo_set_line_width(cr,1.5);
			recv[1]=datas[j][1];
			cairo_move_to(cr,next,height-Blank-last_point[1]*small_sp);
			next++;
			cairo_line_to(cr,next,height-Blank-recv[1]*small_sp);
			last_point[1]=recv[1];
		    cairo_stroke(cr);

		    next--;
	       	cairo_set_source_rgb(cr,0,0,1);
	    	cairo_set_line_width(cr,1.5);
			recv[2]=datas[j][2];
			cairo_move_to(cr,next,height-Blank-last_point[2]*small_sp);
			next++;
			cairo_line_to(cr,next,height-Blank-recv[2]*small_sp);
			last_point[2]=recv[2];
		    cairo_stroke(cr);
		}
		next--;
    }
  	return FALSE;
}

gboolean time_handler (GtkWidget *widget)
{
  if (surface == NULL) return FALSE;

  if (!timer) return FALSE;

  gtk_widget_queue_draw_area(widget,0,0,800,500);
  return TRUE;
}


void show_err(char *err)
{
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer),&start,&end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,err,strlen(err));
}

/* show the received message */
void show_remote_text(char rcvd_mess[])
{
    GtkTextIter start,end;
    gchar * escape, * text;
    escape = g_strescape (rcvd_mess, NULL);
    text = g_strconcat (escape, "\n", NULL);
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,text,strlen(text));/*插入文本到缓冲区*/
    g_free (escape);
    g_free (text);
}

/* show the input text */
void show_local_text(const gchar* text)
{
	GtkTextIter start,end;
    gchar * escape, *text1;
    escape = g_strescape (text, NULL);
    text1 = g_strconcat (escape, "\n", NULL);
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,text1,strlen(text1));/*插入文本到缓冲区*/
	g_free (escape);
	g_free (text1);
}

/* clean the input text */
void on_cls_button_clicked()
{
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(input_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(input_buffer),&start,&end);/*插入到缓冲区*/
}

/* a new thread,to receive message */
gpointer recv_func(gpointer arg)
{
	 gchar rcvd_mess[MAXSIZE];
	 gint bufferIn[8];
	 gint i;
	 GInputVector vector;
	 GError *error = NULL;
	 vector.buffer = bufferIn;
	 vector.size = 32;
	 while(1)
	 {
		memset(rcvd_mess,0,MAXSIZE);
		if(g_socket_receive(sock,vector.buffer, vector.size,NULL, &error)<0)
		{
			perror("server recv error\n");
			exit(1);
		}
	    send_to_mysql(bufferIn);              //记录到mysql
	    g_print("header is  %d %d %d %d %d %d %d %d:\n",bufferIn[0],bufferIn[1],bufferIn[2],bufferIn[3],bufferIn[4],bufferIn[5],bufferIn[6],bufferIn[7]);
	    for(i=0;i<8;i++)
	    {
	    	datas[num][i]=bufferIn[i];
	    }
		num++;
	 }
}

/* build socket connection */
int build_socket(const char *serv_ip,const char *serv_port)
{
	gboolean res;
    g_type_init();
    GInetAddress *iface_address = g_inet_address_new_from_string (serv_ip);
    GSocketAddress *connect_address = g_inet_socket_address_new (iface_address, atoi(serv_port));
    GError *err = NULL;
    sock = g_socket_new(G_SOCKET_FAMILY_IPV4,
    					G_SOCKET_TYPE_STREAM,
						G_SOCKET_PROTOCOL_TCP,
                        &err);
    g_assert(err == NULL);
    res=g_socket_connect (sock,
    				      connect_address,
                          NULL,
                          &err);
    if(res==TRUE)
    {
    	g_thread_new(NULL,recv_func, sock);
    	g_print("recv_func start...\n");
    	return 0;
    }
    else
    {
    	g_print("g_socket_connect error\n");
    	return 1;
    }
}
/* send function */
void send_func(const char *text)
{
	int n;
	GError *err = NULL;
	n=g_socket_send(sock,
	               text,
			MAXSIZE,
	               NULL,
	               &err);
	if(n<0)
	{
		perror("S send error\n");
		exit(1);
	}
}

/* get the input text,and send it */
void on_send_button_clicked()
{
	GtkTextIter start,end;
	gchar *text;
 	if(issucceed==-1){ /* Haven't create a socket */
 		show_err("Not connected...\n");
	}
	else
	{ /* Socket creating has succeed ,so send message */
		text=(gchar *)malloc(MAXSIZE);
		if(text==NULL)
		{
			printf("Malloc error!\n");
			exit(1);
		}
		/* get text */
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(input_buffer),&start,&end);
		text=gtk_text_buffer_get_text(GTK_TEXT_BUFFER(input_buffer),&start,&end,FALSE);
		/* If there is no input,do nothing but return */
		if(strcmp(text,"")!=0)
		{
			send_func(text);
			on_cls_button_clicked();
			show_local_text(text);
		}
		else
			show_err("The message can not be empty...\n");
		free(text);
	}
}

/* Stop the GTK+ main loop function. */
static void destroy (GtkWidget *window,gpointer data)
{
	int i;
	for(i=0;i<360000;i++)
	{
		g_free(datas[i]);
	}
	g_free(datas);

	gtk_main_quit ();
}

/* Menu key test */
void on_menu_activate(GtkMenuItem* item,gpointer data)
{
   	g_print("menuitem %s is pressed.\n",(gchar*)data);
}

void on_button1_clicked(GtkButton *button,gpointer user_data)
{
	int res;
	struct EntryStruct *entry = (struct EntryStruct *)user_data;
	const gchar *serv_ip = gtk_entry_get_text(GTK_ENTRY(entry->IP));
	const gchar *serv_port= gtk_entry_get_text(GTK_ENTRY(entry->Port));
	g_print ("IP: %s\n", serv_ip);
	g_print ("Port: %s\n", serv_port);
	res=build_socket(serv_ip,serv_port);
	if(res==1)
		g_print("IP Address is  Invalid...\n");
	else if(res==-1)
		g_print("Connect Failure... \n");
	else
	{
		init_db();
		g_print("Connect Successful... \n");
		issucceed=0;
	}
}

char *_(char *c)
{
    return(g_locale_to_utf8(c,-1,0,0,0));
}


int main (int argc,char *argv[])
{
	int i=0;
	GtkWidget *window;
	GtkWidget *label1,*label2,*label3,*label4,*label5,*label6,*label7,*label8;
	GtkWidget *conn_button,*close_button,*send_button;
	GtkWidget *rece_view;
	GtkWidget *send_view;
	GtkWidget *da;

	GtkWidget *menubar;
  	GtkWidget *menu;
  	GtkWidget *editmenu;
  	GtkWidget *helpmenu;
  	GtkWidget *rootmenu;
  	GtkWidget *menuitem;
  	GtkAccelGroup *accel_group;

	GtkWidget *grid;
	GtkWidget *scrolled1,*scrolled2;

	gtk_init (&argc, &argv);
	struct EntryStruct entries;
	struct EntryStruct1 entries1;

	datas= (gint **)g_malloc(sizeof(gint *) * 360000);
	for(i=0;i<360000;i++)
	{
		datas[i]=(gint *)g_malloc(sizeof(gint) * 8);
	}


	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Window For Fatigue-Test");
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);
	gtk_widget_set_size_request (window, 800, 600);
	g_signal_connect (G_OBJECT (window), "destroy",G_CALLBACK (destroy), NULL);

	grid=gtk_grid_new ();

	label1 = gtk_label_new ("IP:");
	label2 = gtk_label_new ("Port:");
	label3 = gtk_label_new ("DA1:");
	label4 = gtk_label_new ("DA2:");
	label5 = gtk_label_new ("D0:");
	label6 = gtk_label_new ("PWM:");
	label7 = gtk_label_new ("Duty Cycle:");
	label8 = gtk_label_new ("PWM-DIR:");
	entries.IP = (GtkEntry*)gtk_entry_new ();
	entries.Port = (GtkEntry*)gtk_entry_new ();
	entries1.DA1 = (GtkEntry*)gtk_entry_new ();
	entries1.DA2 = (GtkEntry*)gtk_entry_new ();
	entries1.D0 = (GtkEntry*)gtk_entry_new ();
	entries1.PWM = (GtkEntry*)gtk_entry_new ();
	entries1.PMW_Duty = (GtkEntry*)gtk_entry_new ();
	entries1.PWM_DIR = (GtkEntry*)gtk_entry_new ();
	rece_view = gtk_text_view_new ();
	send_view = gtk_text_view_new ();
	send_button= gtk_button_new_with_label ("Send");

	rece_view=gtk_text_view_new();
    send_view=gtk_text_view_new();

	da = gtk_drawing_area_new();

 	width = gtk_widget_get_allocated_width (da);
  	height = gtk_widget_get_allocated_height (da);

	g_signal_connect (G_OBJECT(da), "draw",G_CALLBACK (draw_callback), NULL);
    g_signal_connect (G_OBJECT(da),"configure-event",G_CALLBACK (draw_configure_event), NULL);
    g_timeout_add(10, (GSourceFunc) time_handler, (gpointer) da);

	/* get the buffer of textbox */
	show_buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(rece_view));
	input_buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(send_view));
	/* set textbox to diseditable */
	gtk_text_view_set_editable(GTK_TEXT_VIEW(rece_view),FALSE);
	/* scroll window */
	scrolled1=gtk_scrolled_window_new(NULL,NULL);
	scrolled2=gtk_scrolled_window_new(NULL,NULL);
	/* create a textbox */
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled1),rece_view);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled2),send_view);
	/* setting of window */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled1),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);



	g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(on_send_button_clicked),NULL);


	conn_button = gtk_button_new_with_label ("Connect");
	gtk_button_set_relief (GTK_BUTTON (conn_button), GTK_RELIEF_NONE);
    g_signal_connect(G_OBJECT(conn_button), "clicked", G_CALLBACK(on_button1_clicked),(gpointer) &entries);
	/* Create a new button that has a mnemonic key of Alt+C. */
	close_button = gtk_button_new_with_mnemonic ("Close");
	gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
	g_signal_connect_swapped (G_OBJECT (close_button), "clicked",G_CALLBACK (gtk_widget_destroy),(gpointer) window);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_widget_destroy), NULL);

	accel_group=gtk_accel_group_new();

	menu=gtk_menu_new();
	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 新建")));
	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 打开")));
	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 保存")));
	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 另存为")));
	menuitem=gtk_separator_menu_item_new();
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	menuitem=gtk_image_menu_item_new_from_stock( GTK_STOCK_QUIT,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 退出")));
  	rootmenu=gtk_menu_item_new_with_label(_(" 文件 "));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootmenu),menu);
	menubar=gtk_menu_bar_new();
  	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),rootmenu);
 	rootmenu=gtk_menu_item_new_with_label(_(" 编辑 "));
 	editmenu=gtk_menu_new();
  	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 剪切 ")));
 	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY,accel_group);
   	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem);
   	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_("复制 ")));
   	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 粘贴 ")));
	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 查找 ")));
  	gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootmenu),editmenu);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),rootmenu);
	rootmenu=gtk_menu_item_new_with_label(_(" 帮助 "));
 	helpmenu=gtk_menu_new();
 	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_( " 帮助 ")));
	menuitem=gtk_menu_item_new_with_label(_(" 关于..."));
 	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(_(" 关于 ")));
  	gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootmenu),helpmenu);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),rootmenu);

	gtk_window_add_accel_group(GTK_WINDOW(window),accel_group);
	//void gtk_grid_attach (GtkGrid  *grid,GtkWidget *child,gint left,gint top,gint width,gint height);
	gtk_grid_attach (GTK_GRID (grid), menubar, 0, 0,850, 30);
	gtk_grid_attach (GTK_GRID (grid),  label1, 0, 50, 50, 30);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries.IP), 50, 50, 100, 30);
	gtk_grid_attach (GTK_GRID (grid),  label2, 150, 50, 50, 40);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET(entries.Port), 200, 50, 100, 30);
	gtk_grid_attach (GTK_GRID (grid),  conn_button, 0, 100, 80, 30);
	gtk_grid_attach (GTK_GRID (grid),  close_button, 100, 100, 80, 30);
	gtk_grid_attach (GTK_GRID (grid),  da, 5, 150, 687, 495);
	gtk_grid_attach (GTK_GRID (grid),  scrolled1, 700, 150, 115, 100);
	gtk_grid_attach (GTK_GRID (grid),  label3, 690, 250, 80, 50);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries1.DA1), 765, 250, 50, 50);
	gtk_grid_attach (GTK_GRID (grid),  label4, 690, 280, 80, 50);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries1.DA2), 765, 280, 50, 50);
	gtk_grid_attach (GTK_GRID (grid),  label5, 690, 310, 80, 50);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries1.D0), 765, 310, 50, 50);
	gtk_grid_attach (GTK_GRID (grid),  label6, 690, 340, 80, 50);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries1.PWM), 765, 340, 50, 50);
	gtk_grid_attach (GTK_GRID (grid),  label7, 690, 370, 80, 50);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries1.PMW_Duty), 765, 370, 50, 50);
	gtk_grid_attach (GTK_GRID (grid),  label8, 690, 400, 80, 50);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries1.PWM_DIR), 765, 400, 50, 50);

	//gtk_grid_attach (GTK_GRID (grid),  scrolled2, 700, 255, 180, 100);
	gtk_grid_attach (GTK_GRID (grid),  send_button, 765, 450, 50, 20);

	gtk_grid_set_row_spacing(GTK_GRID(grid),1);
	gtk_grid_set_column_spacing (GTK_GRID(grid),1);
	gtk_container_add (GTK_CONTAINER (window), grid);

	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}

