/************************************************************************/
/* wfsgtk.c                                                             */
/*                                                                      */
/* Main routine for WFS  GTK client. 		                        */
/************************************************************************/
/*                                                                      */
/*                    CHARA ARRAY USER INTERFACE                        */
/*                 Based on the SUSI User Interface                     */
/*              In turn based on the CHIP User interface                */
/*                                                                      */
/*            Center for High Angular Resolution Astronomy              */
/*              Mount Wilson Observatory, CA 91001, USA                 */
/*                                                                      */
/* Telephone: 1-626-796-5405                                            */
/* Fax      : 1-626-796-6717                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu                                  */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar                                         */
/* Date   : Aug 2012 	  		                                */
/************************************************************************/

#include "wfsgtk.h"

/* Globals */

static char myname[256];
static char	host[256];
static int	port = 0;
static char	skip_socket_manager = FALSE;
int server = -1;
char display[256];	/* Name of display to use */
int server_open = FALSE;
int display_delay_ms = 0;
char do_local_display = TRUE;
#ifdef GTK2
int main_page = -1;
int andor_setup_page = -1;
int wfs_page = -1;
int adjust_page = -1;
int rot_page = -1;
#endif
GtkWidget *notebook = NULL;
struct s_wfs_andor_setup andor_setup;
char engineering_mode = FALSE;
char got_andor_setup;
char server_name[256];
char scope_name[256];
GtkWidget *temp_label;
int send_ready_for_display = FALSE;
int movie_running = FALSE;
struct s_wfs_subap_centroids subap_centroids_ref;
struct s_wfs_subap_centroids subap_centroids;
bool show_boxes = TRUE;
GtkWidget *message_label;
static GtkWidget *tilt_x_label;
static GtkWidget *tilt_y_label;
static GtkWidget *focus_label;
static GtkWidget *a1_label;
static GtkWidget *a2_label;
static GtkWidget *r0_label;
static GtkWidget *aber_button[5];
struct s_wfs_clamp_fluxes clamp_fluxes;
bool wfs_show_tiptilt_info_flag = FALSE;
struct s_wfs_tiptilt wfs_tiptilt;
bool plot_aber = FALSE;
struct s_wfs_aberrations wfs_mean_aberrations;
struct s_wfs_tiptilt_servo wfs_tiptilt_servo;
scope aber_scope;

int main(int  argc, char *argv[] )
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *bigvbox;
	char	s[200];
	char	*p;
	char	*progname;
	GtkWidget *label;
	char	*c;
	GtkWidget *button;
	GtkWidget *hbox;

	/* Let GTK parse command line */

	gtk_init (&argc, &argv);

	/* See if we have any local arguments */

        progname = argv[0];
        p = s;
        while(--argc > 0 && (*++argv)[0] == '-')
        {
             for(p = argv[0]+1; *p != '\0'; p++)
             {
                switch(*p)
                {
                        case 'h': print_usage_message(progname);
                                  exit(0);

			case 'D':if (sscanf(p+1,"%d",&display_delay_ms) != 1)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
                                  if (display_delay_ms < 0)
                                  {
                                        fprintf(stderr,
                                                "Display delay must be > 0.\n");
                                        exit(-1);
                                  }
                                  while(*p != '\0') p++; p--;
                                  break;

			case 'd': do_local_display = FALSE;
                                 break;

			case 'E': engineering_mode = !engineering_mode;
                                 break;

			case 'p': strcpy(host, p+1);
				  for(c=host; *c!=0 && *c!=','; c++);
				  if (*c == 0 || sscanf(c+1,"%d", &port) != 1)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
				  *c = 0;
				  skip_socket_manager = TRUE;
                                  while(*p != '\0') p++; p--;
                                  break;

			default: fprintf(stderr,"Unknown flag %c.\n",*p);
                                 print_usage_message(progname);
                                 exit(-1);
                                 break;
                }
	    }
	}

        if (argc != 1)
        {
                print_usage_message(progname);
                exit(-1);
        }

	sprintf(server_name, "wfs_%s", argv[0]);
	sprintf(scope_name, "%s", argv[0]);

	/* Which machine are we using? */

	if (gethostname(myname, 256) < 0)
	{
		fprintf(stderr,"Failed to get host name.\n");
		exit(-2);
	}
	if (strlen(display)==0) strcpy(myname,"localhost");

	/* Which display should we use? */

	if ((p = getenv("DISPLAY")) == NULL || *p == ':')
	{
		sprintf(display,"%s:0.0",myname);
	}
	else
	{
		strcpy(display, p);
	}

	/* Initialise mesage jobs */

	init_jobs();
	set_client_name(server_name);

	/* Open the connection */

	reopen_socket_callback(NULL, NULL);

	/* Create a new window */

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	/* Set name of window */

	gtk_window_set_title (GTK_WINDOW (window), server_name);

	/* Set a handler for delete_event that immediately exits GTK. */

	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			GTK_SIGNAL_FUNC (delete_event), NULL);

	/* We will need a big vbox in which to place things */

	bigvbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add (GTK_CONTAINER (window), bigvbox);
        gtk_widget_show(bigvbox);

	/* Create a note book to use for this */

	notebook =  gtk_notebook_new();
        gtk_container_add (GTK_CONTAINER (bigvbox), notebook);
        gtk_widget_show(notebook);

	/* OK, let's make the main page */

	vbox = gtk_vbox_new(FALSE, 0);
	label = gtk_label_new("MAIN");
#ifdef GTK2
	main_page = 
#endif
		gtk_notebook_append_page((GtkNotebook *)notebook, 
		vbox, label);
        gtk_widget_show(vbox);

	/* A place for error messages */

	fill_status_view((GtkWidget *)vbox, 0, 100);

	/* This means we will see messages as they come in. */

#ifdef GTK2
        set_status_page(notebook, main_page);
#endif

	/* The Andor Setup Page */

        vbox = gtk_vbox_new(FALSE, 0);
        label = gtk_label_new("ANDOR SETUP");
#ifdef GTK2
        andor_setup_page =
#endif
                gtk_notebook_append_page((GtkNotebook *)notebook,
                vbox, label);

        /* And fill things out. */

        fill_andor_setup_page(vbox);

	/* The WFS Page */

        vbox = gtk_vbox_new(FALSE, 0);
        label = gtk_label_new("WFS");
#ifdef GTK2
        wfs_page =
#endif
                gtk_notebook_append_page((GtkNotebook *)notebook,
                vbox, label);

        /* And fill things out. */

        fill_wfs_page(vbox);

	/* The ADJUST Page */

	if (engineering_mode)
	{
		/* Servo adjustment control */

            	vbox = gtk_vbox_new(FALSE, 0);
            	label = gtk_label_new("ADJUST");
#ifdef GTK2
            	adjust_page =
#endif
                	gtk_notebook_append_page((GtkNotebook *)notebook,
                	vbox, label);

            	/* And fill things out. */
    
        	fill_adjust_page(vbox);

		/* Rotation stage control */

#ifdef ADD_ROT_PAGE
            	vbox = gtk_vbox_new(FALSE, 0);
            	label = gtk_label_new("ROT");
#ifdef GTK2
            	rot_page =
#endif
                	gtk_notebook_append_page((GtkNotebook *)notebook,
                	vbox, label);

            	/* And fill things out. */
    
        	fill_rotation_page(vbox);
#endif
	}

	/* Some buttons for the bottom of all pages */

	vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add (GTK_CONTAINER (bigvbox), vbox);
        gtk_widget_show(vbox);

	wfs_mean_aberrations.r0 = 0.0;
	wfs_mean_aberrations.seeing = 0.0;
	wfs_mean_aberrations.det_stddev = 0.0;
	wfs_mean_aberrations.mir_stddev = 0.0;
	wfs_mean_aberrations.xtilt = 0.0;
	wfs_mean_aberrations.ytilt = 0.0;
	wfs_mean_aberrations.focus = 0.0;
	wfs_mean_aberrations.a1 = 0.0;
	wfs_mean_aberrations.a2 = 0.0;

        message_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(vbox), message_label , TRUE, TRUE, 0);
        gtk_widget_set_usize (message_label, WFS_WIDTH, WFS_HEIGHT);
        gtk_widget_show(message_label);

	hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bigvbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	aber_button[0] = gtk_check_button_new_with_label("");
        gtk_box_pack_start(GTK_BOX(hbox), aber_button[0], TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (aber_button[0]), 1);
        gtk_widget_set_usize (aber_button[0],WFS_WIDTH/20, WFS_HEIGHT);
        gtk_toggle_button_set_active((GtkToggleButton *)aber_button[0], 1);
        gtk_widget_show (aber_button[0]);

        tilt_x_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox), tilt_x_label , TRUE, TRUE, 0);
        gtk_widget_set_usize (tilt_x_label, 3*WFS_WIDTH/20, WFS_HEIGHT);
        gtk_widget_show(tilt_x_label);

        aber_button[1] = gtk_check_button_new_with_label("");
        gtk_box_pack_start(GTK_BOX(hbox), aber_button[1], TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (aber_button[1]), 1);
        gtk_widget_set_usize (aber_button[1],WFS_WIDTH/20, WFS_HEIGHT);
        gtk_toggle_button_set_active((GtkToggleButton *)aber_button[1], 1);
        gtk_widget_show (aber_button[1]);

        tilt_y_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox), tilt_y_label , TRUE, TRUE, 0);
        gtk_widget_set_usize (tilt_y_label, 3*WFS_WIDTH/20, WFS_HEIGHT);
        gtk_widget_show(tilt_y_label);

	aber_button[2] = gtk_check_button_new_with_label("");
        gtk_box_pack_start(GTK_BOX(hbox), aber_button[2], TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (aber_button[2]), 1);
        gtk_widget_set_usize (aber_button[2],WFS_WIDTH/24, WFS_HEIGHT);
        gtk_toggle_button_set_active((GtkToggleButton *)aber_button[2], 1);
        gtk_widget_show (aber_button[2]);

        focus_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox), focus_label , TRUE, TRUE, 0);
        gtk_widget_set_usize (focus_label, 3*WFS_WIDTH/24, WFS_HEIGHT);
        gtk_widget_show(focus_label);

        aber_button[3] = gtk_check_button_new_with_label("");
        gtk_box_pack_start(GTK_BOX(hbox), aber_button[3], TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (aber_button[3]), 1);
        gtk_widget_set_usize (aber_button[3],WFS_WIDTH/24, WFS_HEIGHT);
        gtk_toggle_button_set_active((GtkToggleButton *)aber_button[3], 1);
        gtk_widget_show (aber_button[3]);

        a1_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox), a1_label , TRUE, TRUE, 0);
        gtk_widget_set_usize (a1_label, 3*WFS_WIDTH/24, WFS_HEIGHT);
        gtk_widget_show(a1_label);

        aber_button[4] = gtk_check_button_new_with_label("");
        gtk_box_pack_start(GTK_BOX(hbox), aber_button[4], TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (aber_button[4]), 1);
        gtk_widget_set_usize (aber_button[4],WFS_WIDTH/24, WFS_HEIGHT);
        gtk_toggle_button_set_active((GtkToggleButton *)aber_button[4], 1);
        gtk_widget_show (aber_button[4]);

        a2_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox), a2_label , TRUE, TRUE, 0);
        gtk_widget_set_usize (a2_label, 3*WFS_WIDTH/24, WFS_HEIGHT);
        gtk_widget_show(a2_label);

        r0_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox), r0_label , TRUE, TRUE, 0);
        gtk_widget_set_usize (r0_label, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(r0_label);

	hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bigvbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        /* Creates a CAM ON button */

        button = gtk_button_new_with_label ("USB ON");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_usb_onoff_callback), (gpointer) P_TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a CAM OFF button */

        button = gtk_button_new_with_label ("USB OFF");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_usb_onoff_callback),(gpointer) P_FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a CAM ON button */

        button = gtk_button_new_with_label ("CL ON");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_camlink_onoff_callback), (gpointer) P_TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a CAM OFF button */

        button = gtk_button_new_with_label ("CL OFF");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_camlink_onoff_callback),(gpointer) P_FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a COOLING ON button */

        button = gtk_button_new_with_label ("COOL ON");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_cooler_onoff_callback), (gpointer) P_TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	/* Label for temperature */

	temp_label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hbox), temp_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (temp_label, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(temp_label);

        /* Creates a COOLER OFF button */

        button = gtk_button_new_with_label ("COOL OFF");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC(wfs_cooler_onoff_callback),(gpointer) P_FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a ping button */

        button = gtk_button_new_with_label ("PING");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (ping_callback), (gpointer) &server);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	/* Some buttons for the bottom of all pages */

	hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bigvbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        /* Creates a CAM ON button */

/* This doesn't work very well
        button = gtk_button_new_with_label ("CAMCOMS");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
		(gpointer)(message_array+WFS_ANDOR_REOPEN) );
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);
*/

        /* Creates a MOVIE button */

        button = gtk_button_new_with_label ("MOVIE");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (toggle_movie_running_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a SAVE button */

        button = gtk_button_new_with_label ("SAVE");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
		(gpointer)(message_array+WFS_ANDOR_SAVE_FITS) );
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a SHUT AUTO button */

        button = gtk_button_new_with_label ("SAUTO");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_shutter_auto_callback),(gpointer)NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a SHUT OPEN button */

        button = gtk_button_new_with_label ("SOPEN");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_shutter_open_callback),(gpointer)NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a SHUT CLOSE button */

        button = gtk_button_new_with_label ("SCLOSE");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_shutter_close_callback),(gpointer)NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a reopen button */

        button = gtk_button_new_with_label ("REOPEN");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (reopen_socket_callback), (gpointer) &server);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize(button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Creates a ping button */

        button = gtk_button_new_with_label ("PING");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (ping_callback), (gpointer) &server);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

        /* Add a quit button */

        button = gtk_button_new_with_label ("QUIT");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (delete_event), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER(button),1);
        gtk_widget_set_usize(button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	/* and show the window */

	gtk_widget_show (window);
	
	/* Make sure the display is up to date. */

	andor_setup_display();

	/* Now start up the background process. */

        gtk_timeout_add(10, background_code, 0);

	//printf("%s\n", display);
	//if (initX(display) == 0)
	if (initX(NULL) == 0)
	{
		fprintf(stderr,"Failed to open X windows.\n");
		exit(-10);
	}

	/* Rest in gtk_main and wait for the fun to begin! */

	gtk_main ();

	return(0);
}

/************************************************************************/
/* delete_event()							*/
/*									*/
/* Callback function for program exit.					*/
/************************************************************************/

gint delete_event( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
	gtk_main_quit();
	close_server_socket(server);
	quitX();
	return(FALSE);

} /* delete_event() */

/************************************************************************/
/* background_code()							*/
/*									*/
/* Background code, deals with incomming messages.			*/
/************************************************************************/

gint background_code(gpointer data)
{
	struct timezone tz;
        struct timeval stime; /* System and RT time. */
        long  time_now_ms = 0;
        static long time_next_display = 0;
        static long start_time_sec = 0;
	struct smessage mess;

	/* Process the message queue */

	process_server_socket_all_messages(server);

        /* What time is it now? */

        gettimeofday(&stime,&tz);
        if (start_time_sec == 0) start_time_sec = stime.tv_sec;
        stime.tv_sec -= start_time_sec;

        time_now_ms = stime.tv_sec*1000 + stime.tv_usec/1000;

        /* Let the server know we are ready to display things */

        if (do_local_display && movie_running &&
                time_now_ms >= time_next_display && send_ready_for_display)
        {
		mess.type = WFS_ANDOR_CURRENT_FRAME;
		mess.length = 0;
		mess.data = NULL;
		send_message(server, &mess);
                time_next_display = time_now_ms + display_delay_ms;
                send_ready_for_display = FALSE;
        }

	return TRUE;
}

/************************************************************************/
/* reopen_socket_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void reopen_socket_callback(GtkButton *button, gpointer user_data)
{
	time_t start;
	struct smessage mess;

        /* Close the port */

        close_server_socket(server);

        /* Try and open the port */

	if (skip_socket_manager)
	{
            if ((server = open_server_socket(host, port, server_name)) == -1)
            {
                if (!server_open)
                {
                        fprintf(stderr,"Failed to open %s on %s,%d.\n",
				server_name, host, port);
                        exit(-2);
                }
                else
                        print_status(ERROR,"Failed to open socket %s.\n",
				server_name);
            }
	}
	else
	{
            if ((server = open_server_by_name(server_name)) == -1)
            {
                if (!server_open)
                {
                        fprintf(stderr,"Failed to open socket %s.\n",
				server_name);
                        exit(-2);
                }
                else
                        print_status(ERROR,"Failed to open socket %s.\n",
				server_name);
            }
	}

        /* Setup our other messages */

        set_wfs_messages();

	/* Make sure we know the current setup */

	got_andor_setup = FALSE;
	mess.type = WFS_ANDOR_GET_SETUP;
	mess.length = 0;
	mess.data = NULL;

	if (!send_message(server, &mess))
	{
                if (!server_open)
                {
			fprintf(stderr,"Failed to send WFS_DITHER_NAMES.\n");
                        exit(-7);
                }
                else
			print_status(ERROR,
			"Failed to send WFS_DITHER_NAMES.\n");
	}

	start = time(NULL);
	got_andor_setup = FALSE;
	while(time(NULL) < start+TIME_OUT && !got_andor_setup)
	{
		process_server_socket(server);
	}

	if (!got_andor_setup)
	{
                if (!server_open)
		{
			fprintf(stderr,"Failed to get andor setup.\n");
			exit(-1);
		}
                else
			print_status(FATAL,"Failed to get andor setup.\n");
	}

	/* Make sure we know the current clamp fluxes */

	mess.type = WFS_GET_CLAMP_FLUXES;
	mess.length = 0;
	mess.data = NULL;

	if (!send_message(server, &mess))
	{
                if (!server_open)
                {
			fprintf(stderr,
			"Failed to send WFS_GET_CLAMP_FLUXES.\n");
                        exit(-7);
                }
                else
			print_status(ERROR,
			"Failed to send WFS_GET_CLAMP_FLUXES.\n");
	}

	/* Make sure we know the current servo parameters */

	mess.type = WFS_GET_SERVO;
	mess.length = 0;
	mess.data = NULL;

	if (!send_message(server, &mess))
	{
                if (!server_open)
                {
			fprintf(stderr, "Failed to send WFS_GET_SERVO.\n");
                        exit(-7);
                }
                else
			print_status(ERROR, "Failed to send WFS_GET_SERVO.\n");
	}

        /* OK, if display is on tell server we are ready */

#ifdef DISPLAY_WORKING
        if (do_local_display) send_ready_for_display = TRUE;
#endif

        server_open = TRUE;

} /* reopen_socket_callback() */

/************************************************************************/
/* wfs_message_callback()                                               */
/*                                                                      */
/* A callback that sends a signal with no data. 			*/
/************************************************************************/

void wfs_message_callback(GtkButton *widget, gpointer type)
{
        struct smessage mess;

	mess.type = *((short int *)type);
	mess.length = 0;
	mess.data = NULL;
	if (!send_message(server, &mess))
		print_status(ERROR,"Failed to send message.\n");

} /* wfs_message_callback() */

/************************************************************************/
/* print_usage_message()                                                */
/*                                                                      */
/************************************************************************/

void print_usage_message(char *name)
{
        fprintf(stderr,"usage: %s [-flags] SCOPE\n",name);
        fprintf(stderr,"Flags:\n");
        fprintf(stderr,"-d\t\tTurn off local displays\n");
        fprintf(stderr,"-D[mS]\t\tSet delay in mS between displays (0)\n");
        fprintf(stderr,"-E\t\tToggle engineering mode (FALSE)\n");
        fprintf(stderr,"-h\t\tPrint this message\n");
        fprintf(stderr,"-p[host,port]\tSkip socket manager\n");

} /* print_usage_message() */

/************************************************************************/
/* wfs_usb_onoff_callback()                                             */
/*                                                                      */
/* Turns data collection on or off.					*/
/************************************************************************/

void wfs_usb_onoff_callback(GtkButton *widget, gpointer type)
{
        struct smessage mess;
	int	onoff;

	onoff = *((int *)type);
	mess.type = WFS_ANDOR_USB_ONOFF;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&onoff;
	if (!send_message(server, &mess))
		print_status(ERROR,"Failed to send WFS_ANDOR_USB_ONOFF.\n");

	if (!onoff && movie_running)
		toggle_movie_running_callback(widget, type);

} /* wfs_usb_onoff_callback() */

/************************************************************************/
/* wfs_cooler_onoff_callback()                                          */
/*                                                                      */
/* Turns cooler on or off.						*/
/************************************************************************/

void wfs_cooler_onoff_callback(GtkButton *widget, gpointer type)
{
        struct smessage mess;
	int	onoff;

	onoff = *((int *)type);
	mess.type = WFS_ANDOR_COOLER_ONOFF;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&onoff;
	if (!send_message(server, &mess))
		print_status(ERROR,"Failed to send WFS_ANDOR_COOLER_ONOFF.\n");

} /* wfs_cooler_onoff_callback() */

/************************************************************************/
/* wfs_shutter_auto_callback()                                          */
/*                                                                      */
/* Puts shutter into AUTO mode.						*/
/************************************************************************/

void wfs_shutter_auto_callback(GtkButton *widget, gpointer type)
{
        struct smessage mess;
	int	shutter;

	shutter = ANDOR_SHUTTER_AUTO;
	mess.type = WFS_ANDOR_SET_SHUTTER;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&shutter;
	if (!send_message(server, &mess))
		print_status(ERROR,"Failed to send WFS_ANDOR_SET_SHUTTER.\n");

} /* wfs_shutter_auto_callback() */

/************************************************************************/
/* wfs_shutter_open_callback()                                          */
/*                                                                      */
/* Puts shutter into open mode.						*/
/************************************************************************/

void wfs_shutter_open_callback(GtkButton *widget, gpointer type)
{
        struct smessage mess;
	int	shutter;

	shutter = ANDOR_SHUTTER_OPEN;
	mess.type = WFS_ANDOR_SET_SHUTTER;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&shutter;
	if (!send_message(server, &mess))
		print_status(ERROR,"Failed to send WFS_ANDOR_SET_SHUTTER.\n");

} /* wfs_shutter_open_callback() */

/************************************************************************/
/* wfs_shutter_close_callback()                                         */
/*                                                                      */
/* Puts shutter into close mode.					*/
/************************************************************************/

void wfs_shutter_close_callback(GtkButton *widget, gpointer type)
{
        struct smessage mess;
	int	shutter;

	shutter = ANDOR_SHUTTER_CLOSE;
	mess.type = WFS_ANDOR_SET_SHUTTER;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&shutter;
	if (!send_message(server, &mess))
		print_status(ERROR,"Failed to send WFS_ANDOR_SET_SHUTTER.\n");

} /* wfs_shutter_close_callback() */

/************************************************************************/
/* wfs_camlink_onoff_callback()                                         */
/*                                                                      */
/* Turns data collection on or off.					*/
/************************************************************************/

void wfs_camlink_onoff_callback(GtkButton *widget, gpointer type)
{
        struct smessage mess;
	int	onoff;

	onoff = *((int *)type);
	mess.type = WFS_ANDOR_CAMLINK_ONOFF;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&onoff;
	if (!send_message(server, &mess))
		print_status(ERROR,"Failed to send WFS_ANDOR_CAMLINK_ONOFF.\n");

	if (!onoff && movie_running)
		toggle_movie_running_callback(widget, type);

} /* wfs_camlink_onoff_callback() */

/************************************************************************/
/* update_mean_aberrations()                                            */
/*                                                                      */
/* Update the aberrations text display.					*/
/************************************************************************/

void update_mean_aberrations(void)
{
	char	s[345];
        GtkStyle* style;
        GdkColor color_green = {4L,0,65535,0};
        GdkColor color_red = {4L,65535,0,0};
        GdkColor color_dark_blue = {4L,0,0,65535};
        GdkColor color_purple = {4L,65535,0,65535};
        GdkColor color_light_blue = {4L,0,65535,65535};
        float   *values;

        style = gtk_style_copy (gtk_widget_get_style (tilt_x_label));
        style->fg[GTK_STATE_NORMAL] = color_red;
        style->fg[GTK_STATE_PRELIGHT] = color_red;
        gtk_widget_set_style (tilt_x_label, style);
        sprintf(s,"X: %+6.3f", wfs_mean_aberrations.xtilt);
        gtk_label_set_text((GtkLabel *) tilt_x_label, s);

        style = gtk_style_copy (gtk_widget_get_style (tilt_y_label));
        style->fg[GTK_STATE_NORMAL] = color_green;
        style->fg[GTK_STATE_PRELIGHT] = color_green;
        gtk_widget_set_style (tilt_y_label, style);
        sprintf(s,"Y: %+6.3f", wfs_mean_aberrations.ytilt);
        gtk_label_set_text((GtkLabel *) tilt_y_label, s);

        style = gtk_style_copy (gtk_widget_get_style (focus_label));
        style->fg[GTK_STATE_NORMAL] = color_purple;
        style->fg[GTK_STATE_PRELIGHT] = color_purple;
        gtk_widget_set_style (focus_label, style);
        sprintf(s,"Foc: %+6.3f", wfs_mean_aberrations.focus);
        gtk_label_set_text((GtkLabel *) focus_label, s);

        style = gtk_style_copy (gtk_widget_get_style (a1_label));
        style->fg[GTK_STATE_NORMAL] = color_light_blue;
        style->fg[GTK_STATE_PRELIGHT] = color_light_blue;
        gtk_widget_set_style (a1_label, style);
        sprintf(s,"A1: %+6.3f", wfs_mean_aberrations.a1);
        gtk_label_set_text((GtkLabel *) a1_label, s);

        style = gtk_style_copy (gtk_widget_get_style (a2_label));
        style->fg[GTK_STATE_NORMAL] = color_dark_blue;
        style->fg[GTK_STATE_PRELIGHT] = color_dark_blue;
        gtk_widget_set_style (a2_label, style);
        sprintf(s,"A2: %+6.3f", wfs_mean_aberrations.a2);
        gtk_label_set_text((GtkLabel *) a2_label, s);

	sprintf(s,"r0 = %5.1f", wfs_mean_aberrations.r0);
        gtk_label_set_text((GtkLabel *) r0_label, s);

        if (plot_aber)
        {
                values = vector(1, 6);

                values[6] = 0.0;

                if (gtk_toggle_button_get_active(
                   (GtkToggleButton *)aber_button[0]))
                {
                        values[1] =  wfs_mean_aberrations.xtilt;
                }
                else
                        values[1] = 0.0;

                if (gtk_toggle_button_get_active(
                   (GtkToggleButton *)aber_button[1]))
                        values[2] =  wfs_mean_aberrations.ytilt;
                else
                        values[2] = 0.0;

                if (gtk_toggle_button_get_active(
                   (GtkToggleButton *)aber_button[2]))
                        values[3] =  wfs_mean_aberrations.focus;
                else
                        values[3] = 0.0;

                if (gtk_toggle_button_get_active(
                   (GtkToggleButton *)aber_button[3]))
                        values[4] =  wfs_mean_aberrations.a1;
                else
                        values[4] = 0.0;

                if (gtk_toggle_button_get_active(
                   (GtkToggleButton *)aber_button[4]))
                        values[5] =  wfs_mean_aberrations.a2;
                else
                        values[5] = 0.0;

                update_scope(aber_scope, values);
                free_vector(values, 1, 6);
        }

} /* update_mean_aberrations() */
