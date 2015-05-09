#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <semaphore.h>

struct EntryStruct  
{  
    	GtkEntry * IP;  
	GtkEntry * Port;   
};  

//int sockfd;
GSocket *sock;
int issucceed=-1;
//struct sockaddr_in saddr;
#define MAXSIZE 1024 
GtkTextBuffer *show_buffer,*input_buffer;  
gboolean timer = TRUE;
/* Pixmap for scribble area, to store current scribbles */
static cairo_surface_t *surface = NULL;
//static cairo_surface_t *surfaceL = NULL;
//static cairo_surface_t *surfaceB = NULL;

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
  	gint width, height;
 	width = gtk_widget_get_allocated_width (widget);
  	height = gtk_widget_get_allocated_height (widget);
   	cairo_set_source_surface (cr, surface, 0, 0);
   	cairo_paint (cr);
   	cairo_set_source_rgb(cr,0,0,0); 
	cairo_set_line_width(cr,2);
	//cairo_move_to(cr, 25, height-25);
	//cairo_line_to(cr, width-25, height-25);
	//cairo_move_to(cr, 25, 25);
	//cairo_line_to(cr, 25, height-25);

	gdouble Blank=25;
	gchar c[1];
	gdouble i=0,x=0,y=0,x1=Blank,y1=Blank,y0=height-2*Blank,x2=width-2*Blank; 
	cairo_rectangle (cr,x1, y1, x2, y0);/*axis-y top (x1,y1);axis-zero (x1,y0);axis-x right (x2,y0)*/
   	//for(i=y1;i<=y0+25;i=i+10)
  	//{	
	//	cairo_move_to(cr,x1,i);
	//	cairo_line_to(cr,x1+3,i);
 	//} 
	//for(i=x1;i<=x2+25;i=i+10)
	//{
	//	cairo_move_to(cr,i,y0+25);
	//	cairo_line_to(cr,i,y0+25-3);
	//}
	if((width-2*Blank)/100>5)//Y
	{
		for(i=Blank;i<(width-Blank);i=i+50)
		{
			cairo_move_to(cr,i,Blank);
			cairo_line_to(cr,i,height-Blank+6);
			cairo_move_to(cr,i,height-Blank+16);
			cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size (cr, 15.0);
			gcvt(x++, 4, c);
			cairo_show_text(cr,c);
		}
		for(i=Blank;i<(width-Blank);i=i+5)
		{
			cairo_move_to(cr,i,height-Blank);
			cairo_line_to(cr,i,height-Blank+3);
		}
	}
	if((height-2*Blank)/100>3)//X
	{
		for(i=height-Blank;i>Blank;i=i-50)
		{
			cairo_move_to(cr,Blank-6,i);
			cairo_line_to(cr,width-Blank,i);
			cairo_move_to(cr,Blank-16,i);
			cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size (cr, 15.0);
			gcvt(y++, 4, c);
			cairo_show_text(cr,c);
		}
		for(i=height-Blank;i>Blank;i=i-5)
		{
			cairo_move_to(cr,Blank-3,i);
			cairo_line_to(cr,Blank,i);
		}
	}

        cairo_stroke(cr);
        
  	return FALSE;
}


gboolean time_handler (GtkWidget *widget)
{
  if (surface == NULL) return FALSE;
 
  if (!timer) return FALSE;
 
  gtk_widget_queue_draw_area(widget,0,0,600,480);
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
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/  
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,"Server:\n",8);/*插入文本到缓冲区*/  
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,rcvd_mess,strlen(rcvd_mess));/*插入文本到缓冲区*/  
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,"\n",1);/*插入换行到缓冲区*/  
} 

/* show the input text */
void show_local_text(const gchar* text)
{
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(show_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,"Me:\n",4);/*插入文本到缓冲区*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,text,strlen(text));/*插入文本到缓冲区*/
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(show_buffer),&end,"\n",1);/*插入文本到缓冲区*/
}

/* clean the input text */
void on_cls_button_clicked()
{
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(input_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
	gtk_text_buffer_delete(GTK_TEXT_BUFFER(input_buffer),&start,&end);/*插入到缓冲区*/
}

/* a new thread,to receive message */
gpointer recv_func(gpointer arg)/*recv_func(void *arg)*/
{     

//      char rcvd_mess[MAXSIZE];	
//      while(1)
//	{
//		bzero(rcvd_mess,MAXSIZE);
//		if(recv(sockfd,rcvd_mess,MAXSIZE,0)<0)  /*阻塞直到收到客户端发的消息*/
//		{
//			perror("server recv error\n");
//			exit(1);
//		}
//		show_remote_text(rcvd_mess);  
//		//g_print ("Port: %s\n", rcvd_mess);
//	}

	 char rcvd_mess[MAXSIZE];
	 GInputVector vector;
	 GError *error = NULL;
	 vector.buffer = rcvd_mess;
	 vector.size = MAXSIZE;
	 while(1)
	 {
		memset(rcvd_mess,0,MAXSIZE);
		if(g_socket_receive(sock,vector.buffer, vector.size,NULL, &error)<0)
		{
			perror("server recv error\n");
			exit(1);
		}
	    g_print("Messages = %s\n", rcvd_mess);
	    show_remote_text(rcvd_mess);
	    g_print("Waiting……");
	 }
}
/* build socket connection */
int build_socket(const char *serv_ip,const char *serv_port)
{
//	int res;
//	pthread_t recv_thread;
//	pthread_attr_t thread_attr;
//	/* set status of thread */
//	res=pthread_attr_init(&thread_attr);
//	if(res!=0)
//	{
//		perror("Setting detached attribute failed");
//		exit(EXIT_FAILURE);
//	}
//	sockfd=socket(AF_INET,SOCK_STREAM,0); /* create a socket */
//	if(sockfd==-1)
//	{
//		perror("Socket Error\n");
//		exit(1);
//	}
//	bzero(&saddr,sizeof(saddr));
//	saddr.sin_family=AF_INET;
//	saddr.sin_port=htons(atoi(serv_port));
//	res=inet_pton(AF_INET,serv_ip,&saddr.sin_addr);
//	if(res==0){ /* the serv_ip is invalid */
//		return 1;
//	}
//	else if(res==-1){
//		return -1;
//	}
//	/* set the stats of thread:means do not wait for the return value of subthread */
//	res=pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);
//	if(res!=0)
//	{
//		perror("Setting detached attribute failed");
//		exit(EXIT_FAILURE);
//	}
//        res=connect(sockfd,(struct sockaddr *)&saddr,sizeof(saddr));
//	/* Create a thread,to process the receive function. */
//	if(res==0)
//        {
//       	res=pthread_create(&recv_thread,&thread_attr,&recv_func,NULL);
//	   if(res!=0)
//	     {
//		perror("Thread create error\n");
//		exit(EXIT_FAILURE);
//	     }
//	/* callback the attribute */
//	     (void)pthread_attr_destroy(&thread_attr);
//        }
//        else
//        {
//		perror("Oops:connected failed\n");
//		exit(EXIT_FAILURE);
//        }
//	return 0;



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
//        GSource *source = g_socket_create_source (sock, G_IO_IN,NULL);
//        g_source_set_callback (source, (GSourceFunc)recv_func, NULL, NULL);
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
//	//socklen_t len=sizeof(saddr);
//	n=send(sockfd,text,MAXSIZE,0);
//	if(n<0)
//	{
//		perror("S send error\n");
//		exit(1);
//	}


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
	else{
		g_print("Connect Successful... \n");
		issucceed=0;
	}
}


int main (int argc,char *argv[])
{
	GtkWidget *window;
	GtkWidget *label1;
	GtkWidget *label2;	
	GtkWidget *conn_button;
	GtkWidget *close_button;
	GtkWidget *rece_view;
	GtkWidget *send_view;
	GtkWidget *send_button;
	GtkWidget* da;
	//GtkWidget* daLeft;
	//GtkWidget* daBottom;
  	
	GtkWidget* menubar;
  	GtkWidget* menu;
  	GtkWidget* editmenu;
  	GtkWidget* helpmenu;
  	GtkWidget* rootmenu;
  	GtkWidget* menuitem;
  	GtkAccelGroup* accel_group;
	
	GtkWidget* grid;
	GtkWidget *scrolled1,*scrolled2;


	gtk_init (&argc, &argv);
	struct EntryStruct entries;


	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "MainWindow");
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);
	gtk_widget_set_size_request (window, 800, 600);	
	g_signal_connect (G_OBJECT (window), "destroy",G_CALLBACK (destroy), NULL);

	grid=gtk_grid_new ();

	label1 = gtk_label_new ("IP:");
	label2 = gtk_label_new ("Port:");
	entries.IP = (GtkEntry*)gtk_entry_new ();
	entries.Port = (GtkEntry*)gtk_entry_new ();
	rece_view = gtk_text_view_new ();
	send_view = gtk_text_view_new ();
	send_button= gtk_button_new_with_label ("Send");

	rece_view=gtk_text_view_new();  
    	send_view=gtk_text_view_new();  

	//frame = gtk_frame_new (NULL);
	//gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	da = gtk_drawing_area_new();
	//daLeft = gtk_drawing_area_new();
	//daBottom = gtk_drawing_area_new();
      	/* set a minimum size */
      	//gtk_widget_set_size_request (drawarea, 690, 600);
	//gtk_widget_set_size_request (daLeft, 690, 600);
	//gtk_widget_set_size_request (daBottom, 690, 600);
      	//gtk_container_add (GTK_CONTAINER (frame), drawarea);
  	//gtk_widget_set_size_request (daBottom, 100, 50);
 	//gtk_widget_set_events (G_OBJECT(daBottom), GDK_EXPOSURE_MASK);
	g_signal_connect (G_OBJECT(da), "draw",G_CALLBACK (draw_callback), NULL);
      	g_signal_connect (G_OBJECT(da),"configure-event",G_CALLBACK (draw_configure_event), NULL);
        g_timeout_add(100, (GSourceFunc) time_handler, (gpointer) da);



 
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

	accel_group=gtk_accel_group_new();

    	menu=gtk_menu_new();
    	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 新建")); 
    	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 打开"));
    	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 保存"));
    	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 另存为"));
    	menuitem=gtk_separator_menu_item_new();
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
    	menuitem=gtk_image_menu_item_new_from_stock( GTK_STOCK_QUIT,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)("exit"));
  	rootmenu=gtk_menu_item_new_with_label(" 文件 ");
    	gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootmenu),menu);
    	menubar=gtk_menu_bar_new();
  	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),rootmenu); 
     	rootmenu=gtk_menu_item_new_with_label(" 编辑 ");
     	editmenu=gtk_menu_new();
  	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 剪切 "));
     	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY,accel_group); 
   	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem);
   	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)("复制 "));
   	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 粘贴 ")); 
    	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),menuitem); 
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 查找 "));
  	gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootmenu),editmenu);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),rootmenu);
    	rootmenu=gtk_menu_item_new_with_label(" 帮助 ");
 	helpmenu=gtk_menu_new();
 	menuitem=gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP,accel_group);
  	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)( " 帮助 "));
    	menuitem=gtk_menu_item_new_with_label(" 关于...");
 	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu),menuitem);
  	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_menu_activate),(gpointer)(" 关于 "));
  	gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootmenu),helpmenu);
  	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),rootmenu);


	gtk_window_add_accel_group(GTK_WINDOW(window),accel_group);

	gtk_grid_attach (GTK_GRID (grid), menubar, 0, 0,800, 30);

	gtk_grid_attach (GTK_GRID (grid),  label1, 0, 50, 50, 30);
	gtk_grid_attach (GTK_GRID (grid),  GTK_WIDGET(entries.IP), 50, 50, 100, 30);
	gtk_grid_attach (GTK_GRID (grid),  label2, 150, 50, 50, 40);
	gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET(entries.Port), 200, 50, 100, 30);

	gtk_grid_attach (GTK_GRID (grid),  conn_button, 0, 100, 80, 25);
	gtk_grid_attach (GTK_GRID (grid),  close_button, 100, 100, 80, 25);

	//gtk_grid_attach (GTK_GRID (grid),  daLeft, 5, 150, 50, 550);
	//gtk_grid_attach (GTK_GRID (grid),  daBottom, 5, 700, 690, 50);
	gtk_grid_attach (GTK_GRID (grid),  da, 5, 150, 600, 480);
	gtk_grid_attach (GTK_GRID (grid),  scrolled1, 610, 150, 180, 100);
	gtk_grid_attach (GTK_GRID (grid),  scrolled2, 610, 255, 180, 100);
	gtk_grid_attach (GTK_GRID (grid),  send_button, 610, 360, 80, 25);

	gtk_grid_set_row_spacing(GTK_GRID(grid),1);
	gtk_grid_set_column_spacing (GTK_GRID(grid),1);
	gtk_container_add (GTK_CONTAINER (window), grid);


	gtk_widget_show_all (window);
	gtk_main ();
	return 0;
}
