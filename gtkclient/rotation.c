/************************************************************************/
/* rotation.c                                                           */
/*                                                                      */
/* Routiens to control rotation stage.					*/
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
/* Author : Xiao Che and TtB						*/
/* Date   : April 2014                                                  */
/************************************************************************/

#include "wfsgtk.h"

static int sh_stage_page_created = FALSE;
static GtkWidget *sh_stage_status_label;
static GtkWidget *sh_led_status_label;
static GtkWidget *sh_abs_move_button;
static GtkWidget *sh_rel_move_button;
static GtkWidget *sh_set_pos_button;
static GtkWidget *sh_current_pos_label;
static struct s_wfs_tdc_status tdc_status;

#warning NEEDS A LOT OF CLEANING UP.

/************************************************************************/
/* void fill_sh_stage_page(GtkWidget *vbox);                            */
/*                                                                      */
/* Fill out the sh_stage  page.                                         */
/************************************************************************/

void fill_rotation_page(GtkWidget *vbox)
{
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkObject *adjustment;
  int 	    i;

  /*first row */

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE,TRUE, 0);
  gtk_widget_show(hbox);

  button = gtk_button_new_with_label ("Open");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_OPEN));
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("Initialize");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_INIT) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("Close");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_CLOSE) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("Home");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_HOME));
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("Stop");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_STOP_MOVE) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  sh_stage_status_label = gtk_label_new("SH stage: ???");
  gtk_box_pack_start(GTK_BOX(hbox), sh_stage_status_label, TRUE, TRUE, 0);
  gtk_widget_set_usize (sh_stage_status_label, WFS_WIDTH/6, WFS_HEIGHT/4);
  gtk_widget_show(sh_stage_status_label);

  /*second row */

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE,TRUE, 0);
  gtk_widget_show(hbox);

  label = gtk_label_new("Rot Vel:");
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
  gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(label);

  button = gtk_button_new_with_label ("Faster");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_VEL_FASTER) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("Slower");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_VEL_SLOWER) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  sh_led_status_label = gtk_label_new("LED: ???");
  gtk_box_pack_start(GTK_BOX(hbox), sh_led_status_label, TRUE, TRUE, 0);
  gtk_widget_set_usize (sh_led_status_label, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(sh_led_status_label);

  button = gtk_button_new_with_label ("LED On");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_ACT_AVMODES));
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("LED Off");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_DEACT_AVMODES) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);


  /*third row */

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE,TRUE, 0);
  gtk_widget_show(hbox);

  button = gtk_button_new_with_label ("Move to");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_ABS_MOVE) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  adjustment = gtk_adjustment_new(0, -1000,
				  1000, 1, 1, 0);
  sh_abs_move_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1,0);
  gtk_box_pack_start(GTK_BOX(hbox), sh_abs_move_button, TRUE, TRUE, 0);
  gtk_widget_set_usize (sh_abs_move_button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(sh_abs_move_button);


  button = gtk_button_new_with_label ("Move by");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_REL_MOVE));
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  adjustment = gtk_adjustment_new(0, -1000,
                                  1000, 1, 1, 0);
  sh_rel_move_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1,0);
  gtk_box_pack_start(GTK_BOX(hbox), sh_rel_move_button, TRUE, TRUE, 0);
  gtk_widget_set_usize (sh_rel_move_button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(sh_rel_move_button);

  button = gtk_button_new_with_label ("Identify");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
                      (gpointer)(message_array+WFS_TDC_IDENTIFY) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  label = gtk_label_new("Rot Vel:");
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
  gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(label);

  /*fourth row */

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE,TRUE, 0);
  gtk_widget_show(hbox);

  button = gtk_button_new_with_label ("Set Pos");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
                      (gpointer)(message_array+WFS_TDC_SET_POS) );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  adjustment = gtk_adjustment_new(0, -1000,
                                  1000, 1, 1, 0);
  sh_set_pos_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1,0);
  gtk_box_pack_start(GTK_BOX(hbox), sh_set_pos_button, TRUE, TRUE, 0);
  gtk_widget_set_usize (sh_set_pos_button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(sh_set_pos_button);


  button = gtk_button_new_with_label ("Get  Pos");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_message_callback),
		      (gpointer)(message_array+WFS_TDC_GET_POS)  );
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  sh_current_pos_label = gtk_label_new("Current position: ??? degree");
  gtk_box_pack_start(GTK_BOX(hbox), sh_current_pos_label, TRUE, TRUE, 0);
  gtk_widget_set_usize (sh_current_pos_label, WFS_WIDTH/3, WFS_HEIGHT);
  gtk_widget_show(sh_current_pos_label);

  /*fifth row */

  hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE,TRUE, 0);
  gtk_widget_show(hbox);

  button = gtk_button_new_with_label ("Update");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (tdc_update),NULL);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
  gtk_widget_show(button);

  /* Fill out rest with blanks */

  for (i=0; i<5; i++)
  {
    label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
    gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
    gtk_widget_show(label);
  }

  for (i=0; i<2; i++)
  {
    hbox = gtk_hbox_new(FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE,TRUE, 0);
    gtk_widget_show(hbox);

    label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
    gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
    gtk_widget_show(label);
  }

  /* OK, show the box */

  gtk_widget_show(vbox);

  /* It is now OK to update things */

  sh_stage_page_created = TRUE;

} /* fill_rotation_stage() */

/************************************************************************/
/* tdc_message_callback()                                               */
/*                                                                      */
/* A callback that sends a signal with no data.                         */
/************************************************************************/

void tdc_message_callback(GtkButton *widget, gpointer type)
{
  struct smessage mess;
  char s[234];
  float temp;

  mess.type = *((short int *)type);

  switch(mess.type)
    {
    case WFS_TDC_OPEN:
      mess.length = 0;
      mess.data = NULL;
      sprintf(s,"SH stage: Opened");
      gtk_label_set_text((GtkLabel *)sh_stage_status_label, s);
      break;

    case WFS_TDC_INIT:
      mess.length = 0;
      mess.data = NULL;
      sprintf(s,"SH stage: Initialized");
      gtk_label_set_text((GtkLabel *)sh_stage_status_label, s);
      break;

    case WFS_TDC_CLOSE:
      mess.length = 0;
      mess.data = NULL;
      sprintf(s,"SH stage: Closed");
      gtk_label_set_text((GtkLabel *)sh_stage_status_label, s);
      break;

    case WFS_TDC_IDENTIFY:
      mess.length = 0;
      mess.data = NULL;
      break;

    case WFS_TDC_HOME:
      mess.length = 0;
      mess.data = NULL;
      sprintf(s,"SH stage: Homed");
      gtk_label_set_text((GtkLabel *)sh_stage_status_label, s);
      break;

    case WFS_TDC_STOP_MOVE:
      mess.length = 0;
      mess.data = NULL;
      sprintf(s,"SH stage: Stopped");
      gtk_label_set_text((GtkLabel *)sh_stage_status_label, s);
      break;

    case WFS_TDC_VEL_FASTER:
      mess.length = 0;
      mess.data = NULL;
      break;

    case WFS_TDC_VEL_SLOWER:
      mess.length = 0;
      mess.data = NULL;
      break;

    case WFS_TDC_ACT_AVMODES:
      mess.length = 0;
      mess.data = NULL;
      break;

    case WFS_TDC_DEACT_AVMODES:
      mess.length = 0;
      mess.data = NULL;
      break;

    case WFS_TDC_ABS_MOVE:
      mess.length = sizeof(float);
      temp = gtk_spin_button_get_value_as_float((GtkSpinButton *)sh_abs_move_button);
      mess.data = (unsigned char *)&temp;
      break;

    case WFS_TDC_REL_MOVE:
      mess.length = sizeof(float);
      temp = gtk_spin_button_get_value_as_float((GtkSpinButton *)sh_rel_move_button);
      mess.data = (unsigned char *)&temp;
      break;

    case WFS_TDC_SET_POS:
      mess.length = sizeof(float);
      temp = gtk_spin_button_get_value_as_float((GtkSpinButton *)sh_set_pos_button);
      mess.data = (unsigned char *)&temp;
      break;

    case WFS_TDC_GET_POS:
      mess.length = 0;
      mess.data = NULL;
      break;
    }

  if (!send_message(server, &mess))
    print_status(ERROR,"Failed to send message.\n");

  tdc_update();

} /* tdc_message_callback() */

void tdc_update()
{
  struct smessage mess;
  mess.type =  WFS_TDC_UPDATE_STATUS;
  mess.length = 0;
  mess.data = NULL;

  if (!send_message(server, &mess))
    print_status(ERROR,"Failed to send message.\n");

}

int message_tdc_update_status(int server, struct smessage *mess)
{

  char s[234];

  if (mess->length != sizeof(struct s_wfs_tdc_status))
    {
      print_status(ERROR,
		   "Got WFS_TDC_UPDATE_STATUS message with bad data.\n");
      return NOERROR;
    }
  tdc_status = *((struct s_wfs_tdc_status *) mess->data);

  sprintf(s,"Current position: %f degree", tdc_status.position);
  gtk_label_set_text((GtkLabel *)sh_current_pos_label, s);
  
  if(tdc_status.led_act) sprintf(s,"LED: Active");
  else  sprintf(s,"LED: deactive");
  gtk_label_set_text((GtkLabel *)sh_led_status_label, s);

  return NOERROR;

}/*message_tdc_update_status*/
