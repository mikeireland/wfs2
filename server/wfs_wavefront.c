/************************************************************************/
/* wfs_wavefront.c                                                      */
/*                                                                      */
/* Does the wavefront analysis.						*/
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
/* Author : Xia Che some mods by Theo					*/
/* Date   : Aug 2014                                                    */
/************************************************************************/

#include "./wfs_server.h"

static float **pix_mask = NULL;
static int	pix_mask_min = 0, pix_mask_max = 0;
static int centroid_type = CENTROID_NONLINEAR;

/************************************************************************/
/* subap_calc_pitch							*/
/*									*/
/* estimate the average pitch of the subaps in unit of			*/
/* detector pixels							*/
/************************************************************************/

void subap_calc_pitch(void)
{
	int i,j, index=0, count=0;
	float subap_dist[WFS_DFT_SUBAP_NUMBER * (WFS_DFT_SUBAP_NUMBER-1)/2];
	float min_subap_dist = 9999, pitch = 0. ;

	for(i = 0; i < subap_centroids_ref.num-1 ; i++)
	for( j =i+1; j < subap_centroids_ref.num; j++)
	{
		subap_dist[index] = sqrt(pow(subap_centroids_ref.x[i] - 
				              subap_centroids_ref.x[j], 2) +
					 pow(subap_centroids_ref.y[i] - 
					      subap_centroids_ref.y[j], 2) );
		index++;
	}

	for (i = 0 ; i< index ; i++)
	{
		/* 
		 * to set a lower limit of 2 pixels in case some 
		 * subaps overlap at zero position
		 */

		if( subap_dist[i] > 2 && subap_dist[i] < min_subap_dist)
			min_subap_dist = subap_dist[i];
	}

	for ( i =0 ; i < index ; i++)
	{
		/* 
		 * 1.1 is arbitrary, because the distances between two 
		 * neightboring subap may be slightly different
		 * and I want to find the average values
		 */

		if(subap_dist[i] >2 && subap_dist[i] < min_subap_dist*1.1 )
		{
			pitch += subap_dist[i];
			count ++;
		}
	}

	subap_centroids_ref.pitch = pitch / count;

} /* subap_calc_pitch() */

/************************************************************************/
/* subap_send_centroids_ref()                                           */
/*                                                                      */
/* send reference centroids positions to all clients			*/
/************************************************************************/

int subap_send_centroids_ref(void)
{
        struct smessage mess;

        mess.type = WFS_SUBAP_GET_CENTROIDS_REF;
        mess.length = sizeof(struct s_wfs_subap_centroids);
        mess.data = (unsigned char *)&subap_centroids_ref;

        if (server_send_message_all( &mess) != NOERROR)
        {
                return error(ERROR,
                        "Failed to send current reference subap centroids.");
        }

        return NOERROR;

} /* subap_send_centroids_ref() */

/************************************************************************/
/* wfs_simulation_centroid()                                            */
/*									*/
/* to fake the centroid positions                                       */
/* angle in unit of degree, the rest are in unit of pixel               */
/************************************************************************/

void wfs_simulation_centroid(float centerx, float centery, 
			     float pitch, float angle)
{
	int i;

	angle = angle/180.*PI;

	for( i =0;i < 6; i++)
	{
	  subap_centroids_ref.x[i] = centerx+ pitch * cos(i * PI / 3 + angle);
	  subap_centroids_ref.y[i] = centery+ pitch * sin(i * PI / 3 + angle);
	}

	for( i =6;i < 12; i++)
	{
	  subap_centroids_ref.x[i] = centerx+ 2 * pitch * 
					cos(i * PI / 3 + angle);
	  subap_centroids_ref.y[i] = centery+ 2 * pitch * 
					sin(i * PI / 3 + angle);
	}

	for( i =12;i < 18; i++)
	{
	  subap_centroids_ref.x[i] = centerx+ sqrt(3) * pitch * 
					cos((i+0.5) * PI / 3 + angle);
	  subap_centroids_ref.y[i] = centery+ sqrt(3) * pitch * 
					sin((i+0.5) * PI / 3 + angle);
	}


	if (WFS_DFT_SUBAP_NUMBER == 36)
	{

	    for( i =18;i < 24; i++)
	    {
		subap_centroids_ref.x[i] = centerx+ 3 * pitch * 
			cos(i * PI / 3 + angle);
		subap_centroids_ref.y[i] = centery+ 3 * pitch * 
			sin(i * PI / 3 + angle);
	    }

	    for( i =24;i < 30; i++)
	    {
	    	subap_centroids_ref.x[i] = centerx+ sqrt(7) * pitch * 
			cos(i * PI / 3 + 0.333473 + angle);
	      	subap_centroids_ref.y[i] = centery+ sqrt(7) * pitch * 
			sin(i * PI / 3 + 0.333473 + angle);
	    }


	    for( i =30;i < WFS_DFT_SUBAP_NUMBER; i++)
	    {
	      	subap_centroids_ref.x[i] = centerx+ sqrt(7) * pitch * 
			cos(i * PI / 3 + 0.713724 + angle);
	      	subap_centroids_ref.y[i] = centery+ sqrt(7) * pitch * 
			sin(i * PI / 3 + 0.713724 + angle);
	    }

	}

	subap_centroids_ref.pitch = pitch;
	subap_centroids.pitch = pitch;
	subap_calc_pix_mask();

	/* now send the new reference centroids position to the client */

        if (subap_send_centroids_ref() != NOERROR)
        {
                error(ERROR, "reference centroids sent failed");
        }

} /* wfs_simulation_centroid() */

/************************************************************************/
/* subap_calc_pix_mask()						*/
/*									*/
/* calculate pixel indexes for all subapture				*/
/************************************************************************/

void subap_calc_pix_mask(void)
{
	int	i, j;
	float	x, y, r;

	if (pix_mask != NULL) free_matrix(pix_mask, pix_mask_min,
				pix_mask_max, pix_mask_min, pix_mask_max);

	/* The size must be odd! */

	if (subap_centroids_ref.size%2 == 0) ++subap_centroids_ref.size;

	/* How big is it? */

	pix_mask_max = subap_centroids_ref.size/2;
	pix_mask_min = -1 * pix_mask_max;

	/* Make the mask */

	pix_mask = matrix(pix_mask_min,pix_mask_max,pix_mask_min,pix_mask_max);

	/* Work out what is in it */

	r = (float)subap_centroids_ref.size/2.0;

	for (j= pix_mask_min; j<= pix_mask_max; j++)
	{
	    for (i= pix_mask_min; i<= pix_mask_max; i++)
	    {
		x = (float)i;
		y = (float)j;
		if (sqrt(x*x + y*y) <= r)
			pix_mask[i][j] = 1.0;
		else
			pix_mask[i][j] = 0.0;
		//printf("%.0f ", pix_mask[i][j]);
	    }
	    //printf("\n");
	}

	/* Now work out the rounded positions */

	for(i =0; i < WFS_DFT_SUBAP_NUMBER; i++)
	{
	    subap_centroids_ref.xp[i]= (int)(subap_centroids_ref.x[i] + 0.5);
	    subap_centroids_ref.yp[i] = (int)(subap_centroids_ref.y[i] + 0.5);
	}
        subap_centroids = subap_centroids_ref;

} /* subap_calc_pix_mask() */

/************************************************************************/
/* calculate_centroids()						*/
/*									*/
/* You guessed it, calculate the centroids.				*/
/* Largely based on MI's LABAO code.					*/
/************************************************************************/

void calculate_centroids()
{
	int	subap;
	int	i,j,k,l,max_i = 1, max_j = 1;
	float	flux, max;
	static  int mean_count = 32700;
	static struct s_wfs_subap_centroids subap_centroids_calc;
	float   x, y;
	int	xp, yp;


	for(subap = 0; subap < WFS_DFT_SUBAP_NUMBER; subap++)
	{
	    /* Add the labao offset if there is one */

	    x = subap_centroids_ref.x[subap] + subap_centroids_offset.x[subap];
	    y = subap_centroids_ref.y[subap] + subap_centroids_offset.y[subap];

	    xp =  (int)(x + 0.5);
	    yp =  (int)(y + 0.5);

	    if (centroid_type == CENTROID_NONLINEAR)
	    {
		/* Find the maximum pixel in the box */

		max = -1e32;
		for(i = pix_mask_min; i <= pix_mask_max; i++)
		for(j = pix_mask_min; j <= pix_mask_max; j++)
		{
		    k = i + xp;
		    l = j + yp;

		    if (k > 0 && k <= andor_setup.npixx && 
			l > 0 && l <= andor_setup.npixy &&
		        data_frame[k][l] > max)
		    {
			max_i = k;
			max_j = l;
			max = data_frame[k][l];
		    }
		}
	    }
	    else
	    {
		/* In this case we just use the center of the box */

		max_i = xp;
		max_j = yp;
	    }

	    /* Compute the center of gravity around this position */

	    subap_centroids.x[subap] = 0.0;
	    subap_centroids.y[subap] = 0.0;
	    flux = 0.0;

	    for(i = pix_mask_min; i <= pix_mask_max; i++)
	    for(j = pix_mask_min; j <= pix_mask_max; j++)
	    {
		k = i + max_i;
		l = j + max_j;

		if (k > 0 && k <= andor_setup.npixx && 
			l > 0 && l <= andor_setup.npixy)
		{
		    flux += (pix_mask[i][j]*data_frame[k][l]);
		    subap_centroids.x[subap] +=
			(k*pix_mask[i][j]*data_frame[k][l]);
		    subap_centroids.y[subap] +=
			(l*pix_mask[i][j]*data_frame[k][l]);
	        }
	    }

	    /* Divide, keeping in mind denominator clamp */

	    if (flux < clamp_fluxes.min_flux_subap)
	    {
		subap_centroids.x[subap] = x;
		subap_centroids.y[subap] = y;
	    }
	    else if (flux < clamp_fluxes.clamp_flux_subap)
	    {
		subap_centroids.x[subap] /= clamp_fluxes.denom_clamp_subap * 
						andor_setup.em_gain;
		subap_centroids.y[subap] /= clamp_fluxes.denom_clamp_subap * 
						andor_setup.em_gain;
	    }
 	    else
	    {
		subap_centroids.x[subap] /= flux;
		subap_centroids.y[subap] /= flux;
	    }
	
	    /* Find the nearest pixel to this */

	    subap_centroids.xp[i] = (int)(subap_centroids.x[i] + 0.5);
	    subap_centroids.yp[i] = (int)(subap_centroids.y[i] + 0.5);
	    subap_centroids.inten[subap] = (int)(flux + 0.5);

	    /* Add this to the mean calculation */

	    subap_centroids_calc.x[subap] += subap_centroids.x[subap];
	    subap_centroids_calc.y[subap] += subap_centroids.y[subap];
	}

	/* We now work out the mean positions */

	if (++mean_count >= num_mean_aberrations)
	{
	    for(subap = 0; subap < WFS_DFT_SUBAP_NUMBER; subap++)
	    {
		subap_centroids_mean.x[subap] = subap_centroids_calc.x[subap]/
				num_mean_aberrations;
		subap_centroids_mean.y[subap] = subap_centroids_calc.y[subap]/
				num_mean_aberrations;
	        subap_centroids_calc.x[subap] = 0.0;
	        subap_centroids_calc.y[subap] = 0.0;
	    }
	    mean_count = 0;

	    /* Is it time to use these? */

	    if (set_subap_centroids_ref)
	    {
	        for(subap = 0; subap < WFS_DFT_SUBAP_NUMBER; subap++)
	        {
		    subap_centroids_ref.x[subap] =subap_centroids_mean.x[subap];
		    subap_centroids_ref.y[subap] =subap_centroids_mean.y[subap];
	        }
		set_subap_centroids_ref = FALSE;
		error(MESSAGE,"Centroids set.");
	    }
	}

} /* calculate_centroids() */

/************************************************************************/
/* message_wfs_centroid_type()  	                                */
/*                                                                      */
/* Sets teh kind of centroiding we wish to use.				*/
/************************************************************************/

int message_wfs_centroid_type(struct smessage *message)
{

        if (message->length !=  sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_CENTROID_TYPE,");
        }

	centroid_type = *((int *)message->data);

	if (centroid_type == CENTROID_NONLINEAR)
		send_wfs_text_message("Centroiding set to NONLINEAR");
	else
		send_wfs_text_message("Centroiding set to WINDOW_ONLY");

	return NOERROR;

}/* message_wfs_centroid_type() */

/************************************************************************/
/* message_wfs_set_clamp_flux()  	                                */
/*                                                                      */
/* Sets teh kind of centroiding we wish to use.				*/
/************************************************************************/

int message_wfs_set_clamp_flux(struct smessage *message)
{
	struct s_wfs_clamp_fluxes *new_fluxes;

        if (message->length !=  sizeof(struct s_wfs_clamp_fluxes))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_SET_CLAMP_FLUXES,");
        }

	new_fluxes = (struct s_wfs_clamp_fluxes *)message->data;

	clamp_fluxes = *new_fluxes;

	return server_send_message_all(message);

} /* message_wfs_set_clamp_flux() */

/************************************************************************/
/* send_clamp_fluxes() 		 	                                */
/*                                                                      */
/* Send current flux levels to all clients.				*/
/************************************************************************/

void send_clamp_fluxes(void)
{
	struct smessage message;

	message.type = WFS_SET_CLAMP_FLUXES;
	message.length = sizeof(struct s_wfs_clamp_fluxes);
	message.data = (unsigned char *)&clamp_fluxes;

	server_send_message_all(&message);

} /* message_wfs_set_clamp_flux() */
