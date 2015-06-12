/************************************************************************/
/* sh_messages.c                                                        */
/*                                                                      */
/* Routines to respond to messages.                                     */
/************************************************************************/
/*                                                                      */
/*     Center for High Angular Resolution Astronomy                     */
/* Georgia State University, Atlanta GA 30303-3083, U.S.A.              */
/*                                                                      */
/*                                                                      */
/* Telephone: 1-404-651-1882                                            */
/* Fax      : 1-404-651-1389                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu/~theo/theo.html                  */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Xiao Che							*/
/* Date   : Aug 2012                                                    */
/************************************************************************/

#include "wfs_server.h"

/************************************************************************/
/* setup_tdc_messages()                                                 */
/*                                                                      */
/************************************************************************/

void setup_tdc_messages(void)
{
	server_add_message_job(WFS_TDC_OPEN, message_tdc_open);
	server_add_message_job(WFS_TDC_INIT, message_tdc_initialize);
	server_add_message_job(WFS_TDC_CLOSE, message_tdc_close);
	server_add_message_job(WFS_TDC_IDENTIFY, message_tdc_identify);
	server_add_message_job(WFS_TDC_HOME, message_tdc_home);
	server_add_message_job(WFS_TDC_SET_VELPARAMS, message_tdc_setvelparams);
	server_add_message_job(WFS_TDC_GET_VELPARAMS, message_tdc_getvelparams);
	server_add_message_job(WFS_TDC_VEL_SLOWER, message_tdc_vel_slower);
	server_add_message_job(WFS_TDC_VEL_FASTER, message_tdc_vel_faster);
	server_add_message_job(WFS_TDC_REL_MOVE, message_tdc_rel_move);
	server_add_message_job(WFS_TDC_ABS_MOVE, message_tdc_abs_move);
	server_add_message_job(WFS_TDC_STOP_MOVE, message_tdc_move_stop);
	server_add_message_job(WFS_TDC_ACT_AVMODES, message_tdc_act_avmodes);
	server_add_message_job(WFS_TDC_DEACT_AVMODES,
	                     message_tdc_deact_avmodes);
	server_add_message_job(WFS_TDC_SET_POS, message_tdc_set_poscounter);
	server_add_message_job(WFS_TDC_GET_POS, message_tdc_get_poscounter);
	server_add_message_job(WFS_TDC_UPDATE_STATUS,
	                     message_tdc_update_status);

}/* setup_tdc_messages */

/************************************************************************/
/* message_tdc_open()                                                   */
/*                                                                      */
/************************************************************************/

int message_tdc_open(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR, "Wrong number of data bytes in WFS_TDC_OPEN.");
	}

	return tdc_open();

} /* message_tdc_open() */

/************************************************************************/
/* message_tdc_initialize() 						*/
/*                                                                      */
/************************************************************************/

int message_tdc_initialize(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR, "Wrong number of data bytes in WFS_TDC_INIT.");
	}

	return tdc_initialize(); 

}/* message_tdc_initialize() */

/************************************************************************/
/* message_tdc_close()                                                  */
/*                                                                      */
/************************************************************************/

int message_tdc_close(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR, "Wrong number of data bytes in WFS_TDC_CLOSE.");
	}

	return tdc_close();

}/* message_tdc_close() */

/************************************************************************/
/* message_tdc_identify()                                               */
/*                                                                      */
/************************************************************************/

int message_tdc_identify(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_IDENTIFY.");
	}

	return tdc_identify();

}/* message_tdc_indentify() */

/************************************************************************/
/* message_tdc_home()                                                   */
/*                                                                      */
/************************************************************************/

int message_tdc_home(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_HOME.");
	}

	return tdc_home();

}/* message_tdc_home() */

/************************************************************************/
/* message_tdc_setvelparams()                                           */
/*                                                                      */
/************************************************************************/

int message_tdc_setvelparams(struct smessage *message)
{
	
	struct s_wfs_tdc_status *status;

	if (message->length != sizeof(struct s_wfs_tdc_status))
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_SET_VELPARAMS.");
	}

	status = (struct s_wfs_tdc_status *) message->data;
	
	return tdc_setvelparams(*status);

}/* message_tdc_setvelparams() */

/************************************************************************/
/* message_tdc_getvelparams()                                           */
/*									*/
/* server to client                                                     */
/************************************************************************/

int message_tdc_getvelparams(struct smessage *message)
{

	struct smessage mess;

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_GET_VELPARAMS.");
	}

	mess.type = WFS_TDC_SET_VELPARAMS_CLIENT; 
	mess.length = sizeof(struct s_wfs_tdc_status);
	mess.data = (unsigned char *)&tdc_status;

	if (server_send_message_all( &mess) != NOERROR)
	{
	  return error(ERROR,"Failed to send current TDC status.");
	}

	return NOERROR;

} /* message_tdc_getvelparams() */

/************************************************************************/
/* message_tdc_vel_slower()                                             */
/*                                                                      */
/************************************************************************/

int message_tdc_vel_slower(struct smessage *message)
{
	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_GET_VELPARAMS.");
	}

	tdc_status.max_vel /= 2.;
	tdc_status.accel /= 2.;

	return tdc_setvelparams(tdc_status);

} /* message_tdc_vel_slower() */

/************************************************************************/
/* message_tdc_vel_faster()                                             */
/*                                                                      */
/************************************************************************/

int message_tdc_vel_faster(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_GET_VELPARAMS.");
	}

	tdc_status.max_vel *= 2.;
	tdc_status.accel *= 2.;

	return tdc_setvelparams(tdc_status);

} /* message_tdc_vel_faster() */

/************************************************************************/
/* message_tdc_rel_move()                                               */
/*                                                                      */
/************************************************************************/

int message_tdc_rel_move(struct smessage *message)
{
	float * step;

	if (message->length != sizeof(float))
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_REL_MOVE.");
	}

	step = (float *)message->data;
	return tdc_rel_move(*step);

} /* message_tdc_rel_move() */

/************************************************************************/
/* message_tdc_abs_move()                                               */
/*                                                                      */
/************************************************************************/

int message_tdc_abs_move(struct smessage *message)
{

	float * step;

	if (message->length != sizeof(float))
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_ABS_MOVE.");
	}

	step = (float *)message->data;
	error(MESSAGE,"moving %f degree",*step);

	return tdc_abs_move(*step);

} /* message_tdc_abs_move() */


/************************************************************************/
/* message_tdc_stop_move()                                              */
/*                                                                      */
/************************************************************************/

int message_tdc_move_stop(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_STOP_MOVE.");
	}

	return tdc_move_stop();

} /* message_tdc_move_stop() */


/************************************************************************/
/* message_tdc_act_avmodes()                                            */
/*                                                                      */
/************************************************************************/

int message_tdc_act_avmodes(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_ACT_AVMODES.");
	}

	return tdc_act_avmodes();

} /* message_tdc_act_mdoes() */

/************************************************************************/
/* message_tdc_deact_avmodes()                                          */
/*                                                                      */
/************************************************************************/

int message_tdc_deact_avmodes(struct smessage *message)
{

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_DEACT_AVMODES.");
	}

	return tdc_deact_avmodes();

} /* message_tdc_deact_mdoes() */


/************************************************************************/
/* message_tdc_set_poscounter()                                         */
/*                                                                      */
/************************************************************************/

int message_tdc_set_poscounter(struct smessage *message)
{

	float *pos;

	if (message->length != sizeof(float))
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_SET_POS.");
	}

	pos = (float *)message->data;

	return tdc_set_poscounter(*pos);

} /* message_tdc_set_poscounter() */

/************************************************************************/
/* message_tdc_get_poscounter()                                         */
/*                                                                      */
/************************************************************************/

int message_tdc_get_poscounter(struct smessage *message)
{

	struct smessage mess;

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_GET_POS.");
	}

	mess.type = WFS_TDC_SET_POS_CLIENT;
	mess.length = sizeof(float);
	mess.data = (unsigned char *)&(tdc_status.position);

	if (server_send_message_all(&mess) != NOERROR)
	{
	  return error(ERROR,"Failed to send current position.");
	}

	return NOERROR;

} /* message_tdc_get_poscounter() */

/************************************************************************/
/* message_tdc_update_status()                                          */
/*                                                                      */
/************************************************************************/

int message_tdc_update_status(struct smessage *message)
{

	struct smessage mess;

	if (message->length != 0)
	{
	  return error(ERROR,
	               "Wrong number of data bytes in WFS_TDC_GET_POS.");
	}

	mess.type = WFS_TDC_UPDATE_STATUS;
	mess.length = sizeof(struct s_wfs_tdc_status);
	mess.data = (unsigned char *)&(tdc_status);

	if (server_send_message_all(&mess) != NOERROR)
	{
	  return error(ERROR,"Failed to send current TDC status.");
	}

	return NOERROR;

} /* message_tdc_update_status() */
