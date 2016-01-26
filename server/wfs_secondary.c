/************************************************************************/
/* wfs_secondary.c                                                      */
/*                                                                      */
/* Routines to talk to MERCURY controller.				*/
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
/* Date   : Oct 2016 	  		                                */
/************************************************************************/

#include "./wfs_server.h"

/* Globals */

static int mercury_server = -1;
static struct s_secondary_position mercury_pos = {0 , 0, 0};	
static bool mercury_server_servo = FALSE;

/************************************************************************/
/* open_mercury_server()                                                */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/* Returns error level.							*/
/* If the server paramter is 0 or larger we will assume that the 	*/
/* socket is already open. This might be true if the WFS socket is	*/
/* the server and we have already opened the connection.		*/
/************************************************************************/

int open_mercury_server(void)
{
        struct smessage mess;
	char	server_name[2334];

	close_server_socket(mercury_server);

	sprintf(server_name,"%s_SECONDARY", scope_types[scope_number]);

	if ((mercury_server = open_server_by_name(server_name)) == -1)
	{
		{
                        return error(ERROR,"Failed to open socket %s.\n",
                                server_name);
		}
        }

	/* Add the appropriate messages jobs */

	add_message_job(mercury_server, SECONDARY_SERVO_ON, 
		message_secondary_servo_on);
        add_message_job(mercury_server,SECONDARY_SERVO_OFF,
		message_secondary_servo_off);
        add_message_job(mercury_server,SECONDARY_POSITION,
		message_secondary_position);

	/* Find out where things are now */

	mess.type = SECONDARY_POSITION;
	mess.data = NULL;
	mess.length = 0;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_POSITION.");

	error(MESSAGE,"Reopened socket %d to %s.\n", 
		mercury_server, server_name);

	return NOERROR;

} /* open_mercury_server() */

/************************************************************************/
/* close_mercury_server()                                               */
/*                                                                      */
/************************************************************************/

int close_mercury_server(void)
{
	if (mercury_server < 0) return NOERROR;

	if (mercury_servo_off() != NOERROR)
		error(ERROR,"Failed to turn off mercury servo.");

	sleep(1);

	close_server_socket(mercury_server);

	mercury_server = -1;

	return NOERROR;

} /* close_mercury_server() */

/************************************************************************/
/* message_secondary_servo_on()						*/
/************************************************************************/

int message_secondary_servo_on(int server, struct smessage *mess)
{
	mercury_server_servo = TRUE;

	return NOERROR;

} /* message_secondary_servo_on() */

/************************************************************************/
/* message_secondary_servo_off()					*/
/************************************************************************/

int message_secondary_servo_off(int server, struct smessage *mess)
{
	mercury_server_servo = FALSE;

	return NOERROR;

} /* message_secondary_servo_off() */

/************************************************************************/
/* mercury_servo_is_on()						*/
/************************************************************************/

bool mercury_servo_is_on(void) { return mercury_server_servo; }

/************************************************************************/
/* message_secondary_position()						*/
/************************************************************************/

int message_secondary_position(int server, struct smessage *mess)
{

	if (mess->length != sizeof(mercury_pos))
	{
	    return error(ERROR,"Bad data for SECONDARY_POSITION message.\n");
	}

	mercury_pos = *((struct s_secondary_position *)mess->data);

	return NOERROR;

} /* message_secondary_position() */

/************************************************************************/
/* mercury_stop()			 				*/
/*									*/
/************************************************************************/

int mercury_stop(void)
{
        struct smessage mess;

	mess.type = SECONDARY_STOP;
	mess.length = 0;
	mess.data = NULL;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_STOP.\n");

	return NOERROR;

} /* mercury_stop() */

/************************************************************************/
/* mercury_abort()			 				*/
/*									*/
/************************************************************************/

int mercury_abort(void)
{
        struct smessage mess;

	mess.type = SECONDARY_ABORT;
	mess.length = 0;
	mess.data = NULL;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_ABORT.\n");

	return NOERROR;

} /* mercury_abort() */

/************************************************************************/
/* mercury_servo_on()			 				*/
/*									*/
/************************************************************************/

int mercury_servo_on(void)
{
        struct smessage mess;

	mess.type = SECONDARY_SERVO_ON;
	mess.length = 0;
	mess.data = NULL;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_SERVO_ON.\n");

	return NOERROR;

} /* mercury_servo_on() */

/************************************************************************/
/* mercury_servo_off()			 				*/
/*									*/
/************************************************************************/

int mercury_servo_off(void)
{
        struct smessage mess;

	mess.type = SECONDARY_SERVO_OFF;
	mess.length = 0;
	mess.data = NULL;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_SERVO_OFF.\n");

	return NOERROR;

} /* mercury_servo_off() */

/************************************************************************/
/* mercury_absolute_x()			 				*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_absolute_x(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_X_ABSOLUTE;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_X_ABSOLUTE.\n");

	return NOERROR;

} /* mercury_absolute_x() */

/************************************************************************/
/* mercury_absolute_y()							*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_absolute_y(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_Y_ABSOLUTE;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_Y_ABSOLUTE.\n");

	return NOERROR;

} /* mercury_absolute_y() */

/************************************************************************/
/* mercury_absolute_z()							*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_absolute_z(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_Z_ABSOLUTE;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_Z_ABSOLUTE.\n");

	return NOERROR;

} /* mercury_absolute_z() */

/************************************************************************/
/* mercury_relative_x()							*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_relative_x(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_X_RELATIVE;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_X_RELATIVE.\n");

	return NOERROR;

} /* mercury_relative_x() */

/************************************************************************/
/* mercury_relative_y()							*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_relative_y(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_Y_RELATIVE;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_Y_RELATIVE.\n");

	return NOERROR;

} /* mercury_relative_y() */

/************************************************************************/
/* mercury_relative_z()							*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_relative_z(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_Z_RELATIVE;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_Z_RELATIVE.\n");

	return NOERROR;

} /* mercury_relative_z() */

/************************************************************************/
/* mercury_focus()							*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_focus(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_FOCUS;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_FOCUS.\n");

	return NOERROR;

} /* mercury_focus() */

/************************************************************************/
/* mercury_tilt_az()							*/
/*									*/
/* Value is in arcseconds.						*/
/************************************************************************/

int mercury_tilt_az(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_TIPTILT_AZ;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_TIPTILT_AZ.\n");

	return NOERROR;

} /* mercury_tilt_az() */

/************************************************************************/
/* mercury_tilt_el()							*/
/*									*/
/* Value is in arcseconds.						*/
/************************************************************************/

int mercury_tilt_el(float pos)
{
        struct smessage mess;

	mess.type = SECONDARY_TIPTILT_EL;
	mess.length = sizeof(pos);
	mess.data = (unsigned char *)&pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
                                "Error sending SECONDARY_TIPTILT_EL.\n");

	return NOERROR;

} /* mercury_tilt_el() */

/************************************************************************/
/* mercury_set_position()						*/
/*									*/
/* Value is in microns.							*/
/************************************************************************/

int mercury_set_position(float x, float y, float z)
{
        struct smessage mess;

        mercury_pos.x = x;
        mercury_pos.y = y;
        mercury_pos.z = z;

	mess.type = SECONDARY_SET_POSITION;
	mess.length = sizeof(mercury_pos);
	mess.data = (unsigned char *)&mercury_pos;

	if (!send_message(mercury_server, &mess)) return error(ERROR,
		"Error sending SECONDARY_SET_POSITION.\n");

	return NOERROR;

} /* mercury_set_position() */
