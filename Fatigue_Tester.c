#include <gtk/gtk.h>


struct EntryStruct  
{  
    	GtkEntry * IP;  
	GtkEntry * Port;   
};  


/* Stop the GTK+ main loop function. */
static void destroy (GtkWidget *window,gpointer data)
{
	gtk_main_quit ();
}

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
}

int main (int argc,char *argv[])
{
	GtkWidget *window;
	GtkWidget *label1;
	GtkWidget *label2;	
	GtkWidget *button1;
	GtkWidget *button2;
	GtkWidget* vbox;
	GtkWidget* hbox1;
	GtkWidget* hbox2;
  	GtkWidget* menubar;
  	GtkWidget* menu;
  	GtkWidget* editmenu;
  	GtkWidget* helpmenu;
  	GtkWidget* rootmenu;
  	GtkWidget* menuitem;
  	GtkAccelGroup* accel_group;

	gtk_init (&argc, &argv);
	struct EntryStruct entries;


	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "MainWindow");
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);
	gtk_widget_set_size_request (window, 1000, 75);	
	g_signal_connect (G_OBJECT (window), "destroy",G_CALLBACK (destroy), NULL);

    	vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,5); 
	hbox1=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
	hbox2=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5); 

	label1 = gtk_label_new ("IP:");
	label2 = gtk_label_new ("Port:");
	entries.IP = (GtkEntry*)gtk_entry_new ();
	entries.Port = (GtkEntry*)gtk_entry_new ();
	
	button1 = gtk_button_new_with_label ("Connect");
	gtk_button_set_relief (GTK_BUTTON (button1), GTK_RELIEF_NONE);
        g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(on_button1_clicked),(gpointer) &entries); 
	/* Create a new button that has a mnemonic key of Alt+C. */
	button2 = gtk_button_new_with_mnemonic ("_Close");
	gtk_button_set_relief (GTK_BUTTON (button2), GTK_RELIEF_NONE);
	g_signal_connect_swapped (G_OBJECT (button2), "clicked",G_CALLBACK (gtk_widget_destroy),(gpointer) window);

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
  	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_box_pack_start(GTK_BOX(vbox),menubar,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox1),label1,FALSE,FALSE,5);
	gtk_box_pack_start(GTK_BOX(hbox1),GTK_WIDGET(entries.IP),FALSE,FALSE,5);
	gtk_box_pack_start(GTK_BOX(hbox1),label2,FALSE,FALSE,5);
	gtk_box_pack_start(GTK_BOX(hbox1),GTK_WIDGET(entries.Port),FALSE,FALSE,5);
	gtk_box_pack_start(GTK_BOX(hbox2),button1,FALSE,FALSE,5);
	gtk_box_pack_start(GTK_BOX(hbox2),button2,FALSE,FALSE,5);


	gtk_widget_show_all (window);
	gtk_main ();
	return 0;
}
