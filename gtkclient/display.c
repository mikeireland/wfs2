/************************************************************************/
/* display.c 		                                                */
/*                                                                      */
/* Hanldes messages for displaying pictures.				*/
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
/* Date   : April 2008                                                  */
/************************************************************************/

#include "wfsgtk.h"

static XImage *picture_image = NULL;
static char picture_window_up = FALSE;
static Window picture_window;
static float **values = NULL;
static int last_npixx = -1;
static int last_npixy = -1;

/************************************************************************/
/* message_wfs_andor_current_frame()                                    */
/*                                                                      */
/************************************************************************/

int message_wfs_andor_current_frame(int server, struct smessage *mess)
{
        unsigned short int  *compressed_values;
	float *uncompressed_values;
	char *picture;
	uLongf  len, clen;
	int	i,j;
	float   *pf1;
	char	*pc1, *pc2;
	float	max, min;
	int	pixmult = 1;

	/* Do we do anything at all? */

        if (!do_local_display || !movie_running) return NOERROR;

	/* Is there any data? */

        if (mess->length == 0)
        {
                print_status(ERROR,
			"Got WFS_ANDOR_CURRENT_FRAME with no data.\n");
		send_ready_for_display = TRUE;
                return TRUE;
        }

	/* Do we know how big the image is suppose to be? */

	if (andor_setup.npixx <= 0 || andor_setup.npixy <= 0)
	{
		print_status(ERROR,"Got image without status!\n");
		send_ready_for_display = TRUE;
		return FALSE;
	}

	clen = mess->length;
	len = andor_setup.npixx * andor_setup.npixy * sizeof(float) * 2;
		
	/* Allocate memory */

	if ((uncompressed_values = malloc(len)) == NULL)
	{
		print_status(ERROR,"Ran out of memory!\n");
		send_ready_for_display = TRUE;
		return FALSE;
	}
	
	if ((compressed_values = malloc(len)) == NULL)
	{
		print_status(ERROR,"Ran out of memory!\n");
		free(uncompressed_values);
		send_ready_for_display = TRUE;
		return FALSE;
	}
	
	/* Copy the compressed stuff */

	pc1 = (char *)mess->data;
	pc2 = (char *)compressed_values;

	for(i=0; i<clen; i++) *pc2++ = *pc1++;

	/* Uncompress the data */

	if (uncompress((unsigned char *)uncompressed_values, &len, 
			(unsigned char *)compressed_values, clen) != Z_OK)
	{
		print_status(ERROR,"Uncompress failure on image.\n");
		free(compressed_values);
		free(uncompressed_values);
		send_ready_for_display = TRUE;
		return FALSE;
	}
	
	/* Does it come out right? */

	if (len != andor_setup.npix * sizeof(float))
	{
		print_status(ERROR,
			"Uncompressed size %d does not match frame size %d.\n",
			len, andor_setup.npix * sizeof(unsigned short int));
		free(compressed_values);
		free(uncompressed_values);
		send_ready_for_display = TRUE;
		return FALSE;
	}

	/*
	 * Have the dimmensions changed?
	 * NOTE: This uses NRC, matrices go from 1 not 0.
	 */

	if (andor_setup.npixx != last_npixx || andor_setup.npixy != last_npixy)
	{
		if (values != NULL) 
			free_matrix(values, 1, last_npixx, 1, last_npixy);
		values = matrix(1, andor_setup.npixx, 1, andor_setup.npixy);

		if (picture_window_up)
		{
			XDestroyWindow(theDisplay, picture_window);
			picture_window_up = FALSE;
		}
		last_npixx = andor_setup.npixx;
		last_npixy = andor_setup.npixy;
	}

	/* Copy the data, J inverted for display reasons */

	for(pf1 = uncompressed_values, i = 1; i <= andor_setup.npixx; i++)
	for(j = 1; j <= andor_setup.npixy; j++) values[i][j] = *pf1++;

	/* OK, finished with the memory */

	free(compressed_values);
	free(uncompressed_values);

	/* Work out a decent pixel multiplier */

	pixmult = andor_setup.width / andor_setup.npixx; 
	if (pixmult < 1) pixmult = 1;

	/* Put up the window if we have to */

	if (!picture_window_up)
	{
		picture_window = openWindow(server_name,
			theWidth-andor_setup.npixx*pixmult-30 ,5,
			andor_setup.npixx*pixmult, andor_setup.npixy*pixmult);
		picture_window_up = TRUE;
	}

	/* Display it */

	max = 0.0;
	min = 0.0;
	picture = make_scaled_picture(andor_setup.npixx,andor_setup.npixy,
			pixmult, values,LIN, &min, &max);
	picture_image = XCreateImage(theDisplay,theVisual,
			theDepth,ZPixmap,0, picture,
			andor_setup.npixx*pixmult,andor_setup.npixy*pixmult,
			theBitmapPad, 0);

	/* We need to add GREEN boxes */

	for(i=0; i<subap_centroids_ref.num; i++)
	{
		overlay_rectangle(andor_setup.npixx*pixmult,
			andor_setup.npixy*pixmult,
			picture,
                    	max(subap_centroids_ref.xp[i] -
				subap_centroids_ref.size/2-1,0)*pixmult 
				+ 0.5*pixmult,
                        max(andor_setup.npixy - 
				subap_centroids_ref.yp[i] -
				subap_centroids_ref.size/2,0)*pixmult +
				0.5*pixmult,
                       min(subap_centroids_ref.xp[i] + 
				subap_centroids_ref.size/2-1,
				andor_setup.npixx-1)*pixmult+ 0.5*pixmult,
                       min(andor_setup.npixy - 
				subap_centroids_ref.yp[i] +
				subap_centroids_ref.size/2,andor_setup.npixy-1)
				*pixmult+ 0.5*pixmult,
                    		-1,PLOT_GREEN);
	}

	XPutImage(theDisplay,picture_window,theGC, picture_image,0,0,0,0,
			andor_setup.npixx*pixmult,andor_setup.npixy*pixmult);
	XMapWindow(theDisplay, picture_window);
	XFlush(theDisplay);
	if (picture_image != NULL) XDestroyImage(picture_image);

	/* We must be ready for another image */

	send_ready_for_display = TRUE;

        return TRUE;

} /* message_wfs_andor_current_frame() */

/************************************************************************/
/* clear_picture_callback()                                             */
/*                                                                      */
/************************************************************************/

void clear_picture_callback(GtkButton *button, gpointer user_data)
{
	if (picture_window_up)
        {
                XDestroyWindow(theDisplay, picture_window);
		XFlush(theDisplay);
                picture_window_up = FALSE;
		send_ready_for_display = TRUE;
        }       

} /* clear_picture_callback() */

/************************************************************************/
/* toggle_movie_running_callback()                                      */
/*                                                                      */
/************************************************************************/

void toggle_movie_running_callback(GtkButton *button, gpointer user_data)
{
	if (movie_running)
	{
		movie_running = FALSE;
		send_ready_for_display = FALSE;
		clear_picture_callback(button, user_data);
	}
	else
	{
		movie_running = TRUE;
		send_ready_for_display = TRUE;
	}

} /* toggle_movie_running_callback() */
