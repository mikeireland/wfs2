/************************************************************************/
/* wfs.c 		                                                */
/*                                                                      */
/* ROutines for handling the wfs setup and status display.		*/
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
/* Author : Theo ten Brummelaar                                         */
/* Date   : April 2014                                                  */
/************************************************************************/

#include "wfsgtk.h"

static int wfs_page_created = FALSE;

static GtkWidget *num_dark_button = NULL;
static GtkWidget *threshold_button = NULL;
static GtkWidget *num_frames_button = NULL;

/************************************************************************/
/* void fill_wfs_page(GtkWidget *vbox);					*/
/*									*/
/* Fill out the wfs page.						*/
/************************************************************************/

void fill_wfs_page(GtkWidget *vbox)
{
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkObject *adjustment;
	int	  i;

	/* First row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(100, 1, 1000, 1, 1, 0);
			/* Climb Rate, Digits */
        num_dark_button =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), num_dark_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (num_dark_button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(num_dark_button);

	button = gtk_button_new_with_label ("MAKE DARK");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_make_dark_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("RESET DARK");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_RESET_BACKGROUND) );
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, -32767, 32767, 1, 1, 0);
			/* Climb Rate, Digits */
        threshold_button =
			gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), threshold_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (threshold_button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(threshold_button);

	button = gtk_button_new_with_label ("THRESHOLD");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_threshold_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, 32767, 1, 1, 0);
			/* Climb Rate, Digits */
        num_frames_button =
			gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), num_frames_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (num_frames_button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(num_frames_button);

	button = gtk_button_new_with_label ("#FRAMES");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_num_frames_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	for(i=0; i<5; i++)
	{
		label = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
		gtk_widget_set_usize (label, WFS_WIDTH, WFS_HEIGHT);
		gtk_widget_show(label);
	}

	gtk_widget_show(vbox);

	/* It is now OK to update things */

	wfs_page_created = TRUE;

} /* fill_wfs_page() */

/************************************************************************/
/* wfs_make_dark_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_make_dark_callback(GtkButton *button, gpointer user_data)
{
        struct smessage mess;
	int	num;

        /* The temperature is separate. */

        num =gtk_spin_button_get_value_as_int((GtkSpinButton *)num_dark_button);
        mess.type = WFS_TAKE_BACKGROUND;
        mess.data = (unsigned char *)&num;
        mess.length = sizeof(int);

        send_message(server, &mess);

} /* wfs_make_dark_callback() */

/************************************************************************/
/* wfs_threshold_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_threshold_callback(GtkButton *button, gpointer user_data)
{
        struct smessage mess;
	float	thresh;

        /* The temperature is separate. */

        thresh = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)threshold_button);
        mess.type = WFS_SET_THRESHOLD;
        mess.data = (unsigned char *)&thresh;
        mess.length = sizeof(thresh);

        send_message(server, &mess);

} /* wfs_threshold_callback() */

/************************************************************************/
/* message_wfs_set_threshold()						*/
/*									*/
/************************************************************************/

int message_wfs_set_threshold(int server, struct smessage *mess)
{
	if (mess->length != sizeof(float))
	{
		print_status(ERROR,"Got WFS_SET_THRESHOLD with wrong data.");
		return NOERROR;
	}

	if (!wfs_page_created) return NOERROR;

	gtk_spin_button_set_value((GtkSpinButton *)threshold_button, 
                        *((float *)mess->data));

	return NOERROR;

} /* message_wfs_set_threshold() */

/************************************************************************/
/* wfs_num_frames_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_num_frames_callback(GtkButton *button, gpointer user_data)
{
        struct smessage mess;
	int	num_frames;

        /* The temperature is separate. */

        num_frames = gtk_spin_button_get_value_as_int(
		(GtkSpinButton *)num_frames_button);
        mess.type = WFS_SET_NUM_FRAMES;
        mess.data = (unsigned char *)&num_frames;
        mess.length = sizeof(num_frames);

        send_message(server, &mess);

} /* wfs_num_frames_callback() */

/************************************************************************/
/* message_wfs_set_num_frames()						*/
/*									*/
/************************************************************************/

int message_wfs_set_num_frames(int server, struct smessage *mess)
{
	if (mess->length != sizeof(int))
	{
		print_status(ERROR,"Got WFS_SET_NUM_FRAMES with wrong data.");
		return NOERROR;
	}

	if (!wfs_page_created) return NOERROR;

	gtk_spin_button_set_value((GtkSpinButton *)num_frames_button, 
                        *((int *)mess->data));

	return NOERROR;

} /* message_wfs_set_num_frames() */
