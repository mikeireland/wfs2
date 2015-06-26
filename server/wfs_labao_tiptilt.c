/************************************************************************/
/* wfs_labao_tiptilt.c							*/
/*                                                                      */
/* Receives data from the labao System for tiptilt offset.		*/
/************************************************************************/
/*                                                                      */
/*                    CHARA ARRAY USER INTERFACE			*/
/*                 Based on the SUSI User Interface			*/
/*		In turn based on the CHIP User interface		*/
/*                                                                      */
/*            Center for High Angular Resolution Astronomy              */
/*              Mount Wilson Observatory, CA 91001, USA			*/
/*                                                                      */
/* Telephone: 1-626-796-5405                                            */
/* Fax      : 1-626-796-6717                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu			                */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar         		                */
/* Date   : Nov 2009   							*/
/************************************************************************/

#include "wfs_server.h"
#include <netdb.h>
#include <resolv.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <errno.h>

/*
 * globals
 */

static pthread_t labao_tiptilt_thread;
static pthread_mutex_t labao_tiptilt_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool labao_tiptilt_running = FALSE;
static bool flushed = FALSE;
static int labao_tiptilt_socket_fd = -1;
static int labao_tiptilt_socket = -1;
static float labao_receive_rate;
static float labao_tiptilt_Az = 0.0;
static float labao_tiptilt_El = 0.0;

#define TIPTILT_MAX_VALUE (16382.0) /* Must be the same as TIPTILT version */

/************************************************************************/
/* initialize_labao_tiptilt()						*/
/*									*/
/* Set initial values for the tiptilt global structure.			*/
/************************************************************************/

void initialize_labao_tiptilt(void)
{
	/* Open tiptilt socket() */

	open_labao_tiptilt_socket();

	/* Empty the buffer */

	flush_labao_tiptilt(0, NULL);

	/* Start the thread */

	if (pthread_mutex_init(&labao_tiptilt_mutex, NULL) != 0)
	{
		error(FATAL, "Unable to create LABAO tiptilt mutex.");
		return;
	}

	if (pthread_create(&labao_tiptilt_thread, NULL, 
			do_labao_tiptilt, NULL) != 0)
	{
		error(FATAL, "Error creating LABAO Tiptilt thread.");
		return;
	}

	labao_tiptilt_running = TRUE;

} /* initialize_labao_tiptilt() */

/************************************************************************/
/* reopen_labao_tiptilt_socket()					*/
/*									*/
/* User callable way of forcing a renwal of the tiptilt socket.		*/
/************************************************************************/

int reopen_labao_tiptilt_socket(int argc, char **argv)
{
	open_labao_tiptilt_socket();

	return NOERROR;

} /* reopen_labao_tiptilt_socket() */

/************************************************************************/
/* open_labao_tiptilt_socket()						*/
/*									*/
/* Opens tiptilt socket. Returns error level.				*/
/************************************************************************/

void open_labao_tiptilt_socket(void)
{
	struct sockaddr_in server; /* Server address */
        int     nagle;
	int	reuse;
        struct timeval timeout;

	/* Close it if it's open */

	close(labao_tiptilt_socket_fd);
	labao_tiptilt_socket_fd = -1;
	close(labao_tiptilt_socket);


	/* Create the socket. */

	if ((labao_tiptilt_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
                labao_tiptilt_socket = -1;
		return;
        }

        /* Force things to time out after 5 seconds */

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        if (setsockopt(labao_tiptilt_socket, SOL_SOCKET, SO_RCVTIMEO,
                        (char *)&timeout, sizeof(timeout)) < 0)
        {
                close(labao_tiptilt_socket);
                labao_tiptilt_socket = -1;
		return;
        }

        if (setsockopt(labao_tiptilt_socket, SOL_SOCKET, SO_SNDTIMEO,
                        (char *)&timeout, sizeof(timeout)) < 0)
        {
		close(labao_tiptilt_socket);
		labao_tiptilt_socket = -1;
		return;
        }

        /* We make this a non-blocking port */

        if (fcntl(labao_tiptilt_socket, F_SETFL, O_NONBLOCK) < 0)
        {
                close(labao_tiptilt_socket);
                labao_tiptilt_socket = -1;
		return;
	}

	/* Turn off Nangle */

        nagle = 1;
        if (setsockopt(labao_tiptilt_socket,IPPROTO_TCP,TCP_NODELAY,
                        (char *)&nagle, sizeof(nagle) ))
        {
                close(labao_tiptilt_socket);
                labao_tiptilt_socket = -1;
		return;
	}

	/* Bypass time out */

	reuse = 1;
        if (setsockopt(labao_tiptilt_socket,SOL_SOCKET, SO_REUSEADDR,
                        (char *)&reuse, sizeof(reuse) ))
        {
                close(labao_tiptilt_socket);
                labao_tiptilt_socket = -1;
		return;
	}

        /* Set up server address */

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(TIPTILT_DATA_PORT);

        /* Now bind the socket to this address */

        if (bind(labao_tiptilt_socket,
                        (struct sockaddr *)&server,sizeof(server)) < 0)
        {
                close(labao_tiptilt_socket);
                labao_tiptilt_socket = -1;
		return;
        }

        /* Create a queue for client connection requests */

        if (listen(labao_tiptilt_socket, 0) < 0)
        {
                close(labao_tiptilt_socket);
                labao_tiptilt_socket = -1;
                return;
	}

} /* open_labao_tiptilt_socket() */

/************************************************************************/
/* flush_labao_tiptilt()						*/
/************************************************************************/

int flush_labao_tiptilt(int argc, char **argv)
{
	flushed = TRUE;

	return NOERROR;

} /* flush_labao_tiptilt() */

/************************************************************************/
/* stop_labao_tiptilt()							*/
/*									*/
/* Closes port and tries to stop the tiptilt thread.			*/
/************************************************************************/

void stop_labao_tiptilt(void)
{
	/* Let the thread know to stop */

	labao_tiptilt_running = FALSE;

	/* Wait for the thread to stop */

	error(MESSAGE,"Waiting for LABAO tiptilt thread to terminate.");
	if (pthread_join(labao_tiptilt_thread,NULL) != 0)
	{
		error(ERROR, "Error waiting for tiptilt thread to stop.");
	}

	/* Close the serial port and socket */

	close(labao_tiptilt_socket_fd);
	labao_tiptilt_socket_fd = -1;
	close(labao_tiptilt_socket);

} /* stop_labao_tiptilt() */

/************************************************************************/
/* do_labao_tiptilt()							*/
/*									*/
/* Tries to catch tiptilt data from LABAO system.			*/
/************************************************************************/

void *do_labao_tiptilt(void *arg)
{
	int	i;
	short int data[2];
	unsigned int    len;                    /* Length in bytes */
        struct sockaddr_in connector;   /* Connector's address */
	char c, *p;
	int	num_bytes;
	time_t last_time = 0;
	float	dt;
	static float	delta_x = 0.0, delta_y = 0.0;
	int data_receive_count = 0;

	/* Initialize */

	p = (char *)data;
	num_bytes = 0;

	/* Run until we're told to stop */

	while(labao_tiptilt_running)
	{
		/* Have we tried to flush the line? */

		if (flushed)
		{
		    while(labao_tiptilt_socket_fd != -1 && 
			read(labao_tiptilt_socket_fd, &c, 1) == 1);
		    flushed = FALSE;
		    p = (char *)data;
		    num_bytes = 0;
		}

		/* Do the receive rate calculation */

		if (time(NULL) - last_time > 0)
		{
			dt = time(NULL) - last_time;
			last_time = time(NULL);
			labao_receive_rate = (float)data_receive_count/
					((float)dt);
			data_receive_count = 0;
		}

		/* Is someone trying to send us data? */

		if (labao_tiptilt_socket != -1)
		{
			if (labao_tiptilt_socket_fd != -1 && 
			  labao_tiptilt_waiting(labao_tiptilt_socket_fd))
		        {
			    while ((i = read(labao_tiptilt_socket_fd, p, 
				sizeof(char))) > 0)
		            {
			        p += i ;
			        if ((num_bytes += i) == 
					2*sizeof(short int)) break;

			        /* Not too fast please */

			        usleep(5);
			    }

			    /* Did we fail? */

			    if (i <= 0) labao_tiptilt_socket_fd = -1;
		        }

		        /* Is someone trying to connect? */

		        if (labao_tiptilt_socket_fd == -1)
		    	{
			    /* There may be someone waiting to connect */

		 	    if (labao_tiptilt_waiting(labao_tiptilt_socket))
			    {
			        len = sizeof(connector);
			        if ((labao_tiptilt_socket_fd = 
			  	    accept(labao_tiptilt_socket,
				    (struct sockaddr *)&connector,&len)) < 0)
				    labao_tiptilt_socket_fd = -1;
		 	    }
		        }
		}

		if (num_bytes < 2*sizeof(short int))
		{
			continue;
		}

		/* If we reach here there must be a signal */

		num_bytes = 0;
		p = (char *)data;

		/* OK, we got one... */

		data_receive_count++;

		/* Convert the requested position to a +-1 floating point */

		pthread_mutex_lock(&labao_tiptilt_mutex);
		labao_tiptilt_Az = (float)data[0]/(float)TIPTILT_MAX_VALUE;
		labao_tiptilt_El = (float)data[1]/(float)TIPTILT_MAX_VALUE;
		pthread_mutex_unlock(&labao_tiptilt_mutex);

		/* Is the servo ON? */

		if (wfs_tiptilt_servo.on)
		{
		    delta_x =  wfs_tiptilt_servo.labao_x * labao_tiptilt_Az -
                        wfs_tiptilt_servo.damp_x * delta_x;
		    delta_y =  wfs_tiptilt_servo.labao_y * labao_tiptilt_El -
                        wfs_tiptilt_servo.damp_y * delta_y;

		    /* Don't move too far */

		    if (delta_x > 1.0)
			delta_x = 1.0;
		    else if (delta_x < -1.0)
			delta_x = -1.0;

		    if (delta_y > 1.0)
			delta_y = 1.0;
		    else if (delta_y < -1.0)
			delta_y = -1.0;

		    /* Impliment this */

		    for(i = 0; i < WFS_DFT_SUBAP_NUMBER; i++)
        	    {
		    	subap_centroids_offset.x[i] += delta_x;
		    	subap_centroids_offset.y[i] += delta_y;
			subap_centroids_offset.xp[i]= 
				(int)(subap_centroids_offset.x[i] + 0.5);
			subap_centroids_offset.yp[i] = 
				(int)(subap_centroids_offset.y[i] + 0.5);
		    }
		}
		else
		{
		    delta_x = 0.0;
		    delta_y = 0.0;
		    for(i = 0; i < WFS_DFT_SUBAP_NUMBER; i++)
        	    {
		    	subap_centroids_offset.x[i] = 0.0;
		    	subap_centroids_offset.y[i] = 0.0;
		    	subap_centroids_offset.xp[i] = 0;
		    	subap_centroids_offset.yp[i] = 0;
		    }
		}
	}

	return NULL;

} /* do_labao_tiptilt() */

/************************************************************************/
/* current_labao_tiptilt()						*/
/*									*/
/* Return current labao tiptilt offset.					*/
/************************************************************************/

void current_labao_tiptilt(float *az, float *el)
{
	static float last_Az = 0.0, last_El = 0.0;

	pthread_mutex_lock(&labao_tiptilt_mutex);

	/* We dont' want to use the same one twice */

	if (last_Az == labao_tiptilt_Az && last_El == labao_tiptilt_El)
	{
		*az = 0.0;
		*el = 0.0;
	}
	else
	{
		*az = labao_tiptilt_Az;
		*el = labao_tiptilt_El;
	}
	last_Az = labao_tiptilt_Az;
	last_El = labao_tiptilt_El;
	
	pthread_mutex_unlock(&labao_tiptilt_mutex);

} /* current_labao_tiptilt() */

/************************************************************************/
/* current_labao_receive_rate()						*/
/************************************************************************/

float current_labao_receive_rate(void)
{ 
	float	rate;

	pthread_mutex_lock(&labao_tiptilt_mutex);
	rate = labao_receive_rate;
	pthread_mutex_unlock(&labao_tiptilt_mutex);

	return rate;

} /* current_labao_receive_rate() */

/************************************************************************/
/* labao_tiptilt_waiting()                                              */
/*                                                                      */
/* This is a local version of this function to ensure that it is thread */
/* safe.								*/
/* Unix version of kbhit like function. Returns TRUE if a character     */
/* waiting at the port, FALSE otherwise.                                */
/* Note that this function can cause problems, so for real keyboard     */
/* use I go back to a non-block keyboard I/O.                           */
/************************************************************************/

int     labao_tiptilt_waiting(int fd)
{
        /* Locals */

        fd_set rfds;            /* File discriptor set */
        struct timeval tv;      /* Time to wait */

        if (fd <0) return FALSE;

        /* Set tings up */

        FD_ZERO(&rfds);          /* Zero the file discriptor set */
        FD_SET(fd, &rfds);       /* Add stdin to the set */
        tv.tv_sec = 0;           /* Wait no more than 1 usec */
        tv.tv_usec = 1;

        if (select(FD_SETSIZE, &rfds, NULL, NULL, &tv) < 0)
        {
                /* Something is wrong. */

                if (errno != EINTR) return FALSE;
        }

        if (FD_ISSET(fd, &rfds)) /* Was there something? */
        {
                return TRUE;
        }

        /* Must have been nothing there */

        return FALSE;

} /* labao_tiptilt_waiting() */
