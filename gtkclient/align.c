/************************************************************************/
/* align.c 		                                                */
/*                                                                      */
/* ROutines for handling the align setup and status display.		*/
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

static int align_page_created = FALSE;

static GtkWidget *tries_entry = NULL;

/************************************************************************/
/* void fill_align_page(GtkWidget *vbox);				*/
/*									*/
/* Fill out the align page.						*/
/************************************************************************/

void fill_align_page(GtkWidget *vbox)
{
        GtkWidget *label;
        GtkWidget *button;
        GtkWidget *hbox;
	int	i;

	/* First row */

	hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	label = gtk_label_new("Number of Trys: ");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/5, WFS_HEIGHT);
        gtk_widget_show(label);

	tries_entry = gtk_entry_new ();
        gtk_entry_set_text (GTK_ENTRY (tries_entry),"50");
        gtk_box_pack_start(GTK_BOX(hbox), tries_entry, TRUE, TRUE, 0);
        gtk_widget_set_usize (tries_entry, WFS_WIDTH/5, WFS_HEIGHT);
        gtk_widget_show(tries_entry);

	button = gtk_button_new_with_label ("FOCUS WFS");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_autoalign_focus_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER(button),1);
        gtk_widget_set_usize (button, WFS_WIDTH/5, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("ALIGN BEACON");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_autoalign_beacon_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER(button),1);
        gtk_widget_set_usize (button, WFS_WIDTH/5, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("STOP ALIGN");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_STOP_AUTOALIGN));
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER(button),1);
        gtk_widget_set_usize (button, WFS_WIDTH/5, WFS_HEIGHT);
        gtk_widget_show(button);

	/* Blank space */

	for(i=0; i<6; i++)
	{
		hbox = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
		gtk_widget_show(hbox);

		label = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
		gtk_widget_set_usize (label, WFS_WIDTH, WFS_HEIGHT);
		gtk_widget_show(label);
	}

	/* OK, show this */

	gtk_widget_show(vbox);

	/* It is now OK to update things */

	align_page_created = TRUE;

} /* fill_align_page() */

/************************************************************************/
/* wfs_autoalign_focus_callback()                                       */
/*                                                                      */
/* Start auto alignment.                                                */
/************************************************************************/

void wfs_autoalign_focus_callback(GtkButton *button, gpointer data)
{
        struct smessage mess;
        int     tries;
        char    *entry;

        entry = (char *)gtk_entry_get_text(GTK_ENTRY(tries_entry));
        sscanf(entry,"%d", &tries);

        mess.type = WFS_START_FOCUS_PARABOLA;
        mess.length = sizeof(int);
        mess.data = (unsigned char *)&tries;

        if (!send_message(server, &mess))
        {
          print_status(ERROR,
                "Failed to send WFS_START_FOCUS_PARABOLA message.\n");
        }

} /* wfs_autoalign_focus_callback() */

/************************************************************************/
/* wfs_autoalign_beacon_callback()                                       */
/*                                                                      */
/* Start auto alignment.                                                */
/************************************************************************/

void wfs_autoalign_beacon_callback(GtkButton *button, gpointer data)
{
        struct smessage mess;
        int     tries;
        char    *entry;

        entry = (char *)gtk_entry_get_text(GTK_ENTRY(tries_entry));
        sscanf(entry,"%d", &tries);

        mess.type = WFS_START_ALIGN_BEACON;
        mess.length = sizeof(int);
        mess.data = (unsigned char *)&tries;

        if (!send_message(server, &mess))
        {
          print_status(ERROR,
                "Failed to send WFS_START_ALIGN_BEACON message.\n");
        }

} /* wfs_autoalign_beacon_callback() */
