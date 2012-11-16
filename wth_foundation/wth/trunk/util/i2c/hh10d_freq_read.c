/*

  hh10d_freq_read.c

  compile command
  gcc -Wall -o hh10d_freq_read hh10d_freq_read.c -lasound -lfftw3 -lm

  This example reads the frequency of HH10D humidity sensor
  as audio signal from the default PCM device
  8-Bit signed

*/


#define _USE_MATH_DEFINES
#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PI	M_PI	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)

#define FALSE          0
#define TRUE           1

typedef struct sndconf {
  char *device;               /* playback device */
  snd_pcm_format_t format;    /* sample format */
  unsigned int rate;          /* stream rate */
  float timestep;             /* timestep between two sample */
  unsigned int channels;      /* count of channels */
  snd_pcm_uframes_t frames;
  snd_pcm_uframes_t bufsiz;
  snd_pcm_access_t access;
  unsigned int buffer_time;   /* ring buffer length in us */
  unsigned int period_time;   /* period time in us */
  int resample;               /* enable alsa-lib resampling */
  int period_event;           /* produce poll event after each period */
} sndconf_t;


typedef struct sndstat {
  snd_pcm_sframes_t buffer_size;
  snd_pcm_sframes_t period_size;
  snd_output_t *output;
} sndstat_t;

sndconf_t set_soundconfig( void) {
  sndconf_t soundcnf;

  //soundcnf.device       = "hw:0,0";
  soundcnf.device       = "default";
  soundcnf.format       = SND_PCM_FORMAT_S8;  /* 8-Bit signed */
  soundcnf.rate         = 44100;
  soundcnf.rate         = 48000;
  //soundcnf.rate         = 23950; // at this sample rate approx. 510 frames are read, 
				   // close enough to 512 data points for Numerical Recipes
                                   // four1 FFT
  soundcnf.timestep     = 1. / soundcnf.rate;
  soundcnf.channels     = 1;
  soundcnf.buffer_time  = 500000;
  soundcnf.period_time  = 100000;
  soundcnf.resample     = 1;
  soundcnf.period_event = 0;
  soundcnf.frames       = 64;
  soundcnf.access       = SND_PCM_ACCESS_RW_INTERLEAVED; 

  return soundcnf;
}

/* Display information about the PCM interface */
int print_soundconfig(snd_pcm_t *handle,
                      snd_pcm_hw_params_t *params,
                      sndconf_t sndconfig,
                      sndstat_t sndstatus) 
{
  unsigned int val, val2;
  int dir;

  printf("# Display info about PCM interface\n");
  printf("# PCM handle name = '%s'\n",
         snd_pcm_name(handle));

  printf("# PCM state = %s\n",
         snd_pcm_state_name(snd_pcm_state(handle)));

  snd_pcm_hw_params_get_access(params,
                          (snd_pcm_access_t *) &val);
  printf("# access type = %s\n",
         snd_pcm_access_name((snd_pcm_access_t)val));

  snd_pcm_hw_params_get_format(params, (snd_pcm_format_t *)&val);
  printf("# format = '%s' (%s)\n",
    snd_pcm_format_name((snd_pcm_format_t)val),
    snd_pcm_format_description(
                             (snd_pcm_format_t)val));

  snd_pcm_hw_params_get_subformat(params,
                        (snd_pcm_subformat_t *)&val);
  printf("# subformat = '%s' (%s)\n",
    snd_pcm_subformat_name((snd_pcm_subformat_t)val),
    snd_pcm_subformat_description(
                          (snd_pcm_subformat_t)val));

  snd_pcm_hw_params_get_channels(params, &val);
  printf("# channels = %d\n", val);

  snd_pcm_hw_params_get_rate(params, &val, &dir);
  printf("# rate = %d bps\n", val);

  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  printf("# period time = %d us\n", val);

  snd_pcm_hw_params_get_period_size(params,
                                    &sndconfig.frames, &dir);
  printf("# period size = %d frames\n", (int)sndconfig.frames);

  snd_pcm_hw_params_get_buffer_time(params,
                                    &val, &dir);
  printf("# buffer time = %d us\n", val);

  snd_pcm_hw_params_get_buffer_size(params,
                         &sndconfig.bufsiz);
  printf("# buffer size = %d frames\n", (int )sndconfig.bufsiz);

  snd_pcm_hw_params_get_periods(params, &val, &dir);
  printf("# periods per buffer = %d frames\n", val);

  snd_pcm_hw_params_get_rate_numden(params,
                                    &val, &val2);
  printf("# exact rate = %d/%d bps\n", val, val2);

  val = snd_pcm_hw_params_get_sbits(params);
  printf("# significant bits = %d\n", val);

  val = snd_pcm_hw_params_is_batch(params);
  printf("# is batch = %d\n", val);

  val = snd_pcm_hw_params_is_block_transfer(params);
  printf("# is block transfer = %d\n", val);

  val = snd_pcm_hw_params_is_double(params);
  printf("# is double = %d\n", val);

  val = snd_pcm_hw_params_is_half_duplex(params);
  printf("# is half duplex = %d\n", val);

  val = snd_pcm_hw_params_is_joint_duplex(params);
  printf("# is joint duplex = %d\n", val);

  val = snd_pcm_hw_params_can_overrange(params);
  printf("# can overrange = %d\n", val);

  val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
  printf("# can mmap = %d\n", val);

  val = snd_pcm_hw_params_can_pause(params);
  printf("# can pause = %d\n", val);

  val = snd_pcm_hw_params_can_resume(params);
  printf("# can resume = %d\n", val);

  val = snd_pcm_hw_params_can_sync_start(params);
  printf("# can sync start = %d\n", val);

  printf("#\n");
  return(0);
}



int set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        sndconf_t sndconfig,
                        sndstat_t sndstatus)
{
        unsigned int rrate;
        snd_pcm_uframes_t size;
        int err, dir;
        /* choose all parameters */
        err = snd_pcm_hw_params_any(handle, params);
        if (err < 0) {
                printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
                return err;
        }
        /* set hardware resampling */
        err = snd_pcm_hw_params_set_rate_resample(handle, params, sndconfig.resample);
        if (err < 0) {
                printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the interleaved read/write format */
        err = snd_pcm_hw_params_set_access(handle, params, sndconfig.access);
        if (err < 0) {
                printf("Access type not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the sample format */
        err = snd_pcm_hw_params_set_format(handle, params, sndconfig.format);
        if (err < 0) {
                printf("Sample format not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the count of channels */
        err = snd_pcm_hw_params_set_channels(handle, params, sndconfig.channels);
        if (err < 0) {
                printf("Channels count (%i) not available for playbacks: %s\n", sndconfig.channels, snd_strerror(err));
                return err;
        }
        /* set the stream rate */
        rrate = sndconfig.rate;
        err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
        if (err < 0) {
                printf("Rate %iHz not available for playback: %s\n", sndconfig.rate, snd_strerror(err));
                return err;
        }
        if (rrate != sndconfig.rate) {
                printf("Rate doesn't match (requested %iHz, get %iHz)\n", sndconfig.rate, err);
                return -EINVAL;
        }
        /* set the buffer time */
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &sndconfig.buffer_time, &dir);
        if (err < 0) {
                printf("Unable to set buffer time %i for playback: %s\n", sndconfig.buffer_time, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_buffer_size(params, &size);
        if (err < 0) {
                printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
                return err;
        }
        sndstatus.buffer_size = size;
        /* set the period time */
        err = snd_pcm_hw_params_set_period_time_near(handle, params, &sndconfig.period_time, &dir);
        if (err < 0) {
                printf("Unable to set period time %i for playback: %s\n", sndconfig.period_time, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
        if (err < 0) {
                printf("Unable to get period size for playback: %s\n", snd_strerror(err));
                return err;
        }
        sndstatus.period_size = size;
        /* write the parameters to device */
        err = snd_pcm_hw_params(handle, params);
        if (err < 0) {
                printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}
static int set_swparams(snd_pcm_t *handle, 
                        snd_pcm_sw_params_t *swparams,
                        sndconf_t sndconfig,
                        sndstat_t sndstatus)
{
        int err;
        /* get the current swparams */
        err = snd_pcm_sw_params_current(handle, swparams);
        if (err < 0) {
                printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* start the transfer when the buffer is almost full: */
        /* (buffer_size / avail_min) * avail_min */
        err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (sndstatus.buffer_size / sndstatus.period_size) * sndstatus.period_size);
        if (err < 0) {
                printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* allow the transfer when at least period_size sample can be processed */
        /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
        err = snd_pcm_sw_params_set_avail_min(handle, swparams, sndconfig.period_event ? sndstatus.buffer_size : sndstatus.period_size);
        if (err < 0) {
                printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* enable period events when requested */
        if (sndconfig.period_event) {
                err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
                if (err < 0) {
                        printf("Unable to set period event: %s\n", snd_strerror(err));
                        return err;
                }
        }
        /* write the parameters to the playback device */
        err = snd_pcm_sw_params(handle, swparams);
        if (err < 0) {
                printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}


/*
 FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)

 Inputs:
	data[] : array of complex* data points of size 2*NFFT+1.
		data[0] is unused,
		* the n'th complex number x(n), for 0 <= n <= length(x)-1, 
                  is stored as:
			data[2*n+1] = real(x(n))
			data[2*n+2] = imag(x(n))
		if length(Nx) < NFFT, the remainder of the array must be 
                padded with zeros

	nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
	isign:  if set to 1, 
			computes the forward FFT
		if set to -1, 
		computes Inverse FFT - in this case the output values have
		to be manually normalized by multiplying with 1/NFFT.
 Outputs:
	data[] : The FFT or IFFT results are stored in data, overwriting 
                 the input.
*/

void 
four1(double data[], int nn, int isign)
{
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    
    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
	if (j > i) {
	    tempr = data[j];     data[j] = data[i];     data[i] = tempr;
	    tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
	}
	m = n >> 1;
	while (m >= 2 && j > m) {
	    j -= m;
	    m >>= 1;
	}
	j += m;
    }
    mmax = 2;
    while (n > mmax) {
	istep = 2*mmax;
	theta = TWOPI/(isign*mmax);
	wtemp = sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi = sin(theta);
	wr = 1.0;
	wi = 0.0;
	for (m = 1; m < mmax; m += 2) {
	    for (i = m; i <= n; i += istep) {
		j =i + mmax;
		tempr = wr*data[j]   - wi*data[j+1];
		tempi = wr*data[j+1] + wi*data[j];
		data[j]   = data[i]   - tempr;
		data[j+1] = data[i+1] - tempi;
		data[i] += tempr;
		data[i+1] += tempi;
	    }
	    wr = (wtemp = wr)*wpr - wi*wpi + wr;
	    wi = wi*wpr + wtemp*wpi + wi;
	}
	mmax = istep;
    }
}

/* return index of maximum power value */
int index_maxpower( double data[], int ndata)
{
  int i;
  float maxpower = 0.0;
  int maxindex = 0;

  for ( i = 0; i < ndata; i++){
    if (data[i] > maxpower) {
      maxpower = data[i];
      maxindex = i;
    }
  }
  return(maxindex);
}


int 
main( int argc, char **argv) {
  int i;
  int err;
  int size;
  int op;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int dir;
  char *buffer;
  int NFFT;
  int NFFTW;
  fftw_complex *hh10d_in, *hh10d_out;
  double *hh10d_abs;
  double *hh10d_power;
  fftw_plan hh10d_plan;
  double *sample;
  double *sabs;
  double *spow;
  float nyquist;
  float *freq;
  sndconf_t sndconfig;
  sndstat_t sndstatus;
  int print_audio    = FALSE;
  int print_freq     = FALSE;
  int display_info   = FALSE;
  int print_timetick = FALSE;
  int do_nrfft       = FALSE;
  int do_fftw        = FALSE;
  int verbose        = FALSE;


  printf("# HH10D frequency as audio signal\n#\n");

  while ((op = getopt(argc, argv, "ianfzvh")) != -1) {
    switch(op) {
    case 'i':
      display_info   = TRUE;
      break;
    case 'a':
      print_audio    = TRUE;
      break;
    case 'n':
      do_nrfft       = TRUE;
      print_freq     = TRUE;
      break;
    case 'f':
      do_fftw        = TRUE;
      print_freq     = TRUE;
      break;
    case 'z':
      print_timetick = TRUE;
      break;
    case 'v':
      verbose        = TRUE;
      break;
    case 'h':
      printf("Usage: hh10d_freq_read [options]\n");
      printf("where options include:\n");
      printf("\t-i\tdisplay information about sound device\n");
      printf("\t-a\tprint audio signal in time domain\n");
      printf("\t-n\tprint signal in frequency domain using Numerical Recipes fourier transformation\n");
      printf("\t-f\tprint signal in frequency domain using FFTW fourier transformation\n");
      printf("\t-z\tprint timetick and audiosignal in comma-separated format\n");
      break;
    default:
      printf("Usage: hh10d_freq_read [options]\n");
      printf("where options include:\n");
      printf("\t-i\tdisplay information about sound device\n");
    }
  }

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);
  sndconfig = set_soundconfig();

  /* Open PCM device for recording (capture). */
  err = snd_pcm_open(&handle, sndconfig.device,
                    SND_PCM_STREAM_CAPTURE, 0);
  if (err < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(err));
    exit(1);
  }

  /* configure PCM device */
  err = set_hwparams(handle, hwparams, sndconfig, sndstatus);
  if ( err < 0 ) {
    printf("Setting of hwparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  /*
  if ((err = set_swparams(handle, swparams, sndconfig, sndstatus)) < 0) {
    printf("Setting of swparams failed: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  */
  if ( display_info == TRUE ) 
    print_soundconfig(handle, hwparams, sndconfig, sndstatus);
  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(hwparams,
                                    &sndconfig.frames, &dir);
  //size = frames * 2 * 1; /* 2 bytes/sample, 1 channels */
  size = sndconfig.frames * 1 * 1; /* 1 bytes/sample, 1 channels */
  buffer = (char *) malloc(size);
  bzero(buffer, size-1);

  /* reading audio signal */
  err = snd_pcm_readi(handle, buffer, sndconfig.frames);
  if (err == -EPIPE) {
    /* EPIPE means overrun */
    fprintf(stderr, "overrun occurred\n");
    snd_pcm_prepare(handle);
  } else if (err < 0) {
    fprintf(stderr,
            "error from read: %s\n",
            snd_strerror(err));
  } else if (err != (int)sndconfig.frames) {
    fprintf(stderr, "short read, read %d frames\n", err);
  }
  printf("# frames read: %d\n", err);
  printf("# buffer size: %d ( should be frames * bytes/sample * channels)\n", size);

  /* PCM device not needed anymore */
  snd_pcm_drain(handle);
  snd_pcm_close(handle);

  /* print audio signal in time domain */
  if ( print_audio == TRUE) {
    if ( print_timetick == FALSE ) { 
       printf("# audiosignal [arb.units]\n");
       for ( i = 0; i < size; i++) 
         printf("%d\n", buffer[i]);
     } else {
       printf("# time [s], audiosignal [arb.units]\n");
       for ( i = 0; i < size; i++) 
        printf("%f, %d\n", sndconfig.timestep*(i+1), buffer[i]);
    }
  }


  /* frequency calculation using Numerical Recipes four1 FFT */
  if ( do_nrfft == TRUE) {
    printf("#\n# FFT using Numerical Recipes four1\n");

    /* calculate NFFT as the next higher power of 2 >= Nx */
    NFFT = (int)pow(2.0, ceil(log((double)size)/log(2.0)));
    printf("# Number of data points (NFFT) = %d\n", NFFT);

    nyquist = sndconfig.rate / 2;
    printf("# Nyquist frequency: %.2f (should be half of samplerate)\n", nyquist);

    /* allocate memory for NFFT complex numbers (note the +1) */
    sample = (double *) malloc((2*NFFT+1) * sizeof(double));

    /* allocate memory for NFFT absolute numbers (note the +1) */
    sabs = (double *) malloc((NFFT+1) * sizeof(double));

    /* allocate memory for NFFT absolute numbers (note the +1) */
    spow = (double *) malloc((NFFT+1) * sizeof(double));

    /* allocate memory for frequency values (note the +1) */
    freq        = (float *) malloc((NFFT+1) * sizeof(float));

    /* Storing x(n) in a complex array to make it work with four1. 
    This is needed even though x(n) is purely real in this case. */
    for(i=0; i<size; i++) {
      sample[2*i+1] = buffer[i];
      sample[2*i+2] = 0.0;
      sabs[i] = 0.0;
    }
    /* pad the remainder of the array with zeros (0 + 0 j) */
    for(i=size; i<NFFT; i++) {
      sample[2*i+1] = 0.0;
      sample[2*i+2] = 0.0;
      sabs[i] = 0.0;
    }

    if ( verbose == TRUE) {
        printf("#\n# Input complex sequence (padded to next highest power of 2):\n");
        for(i=0; i<NFFT; i++) {
          printf("sample[%d] = (%.2f + j %.2f)\n", 
                 i,
                 sample[2*i+1],
                 sample[2*i+2]);
      }
    }
    /* FFT of audiosignal */
    four1( sample, NFFT, 1);

    printf("#\n# FFT of audiosignal [arb.units]\n");
    for ( i = 0; i < NFFT; i++)  {
      sabs[i] = sqrt(sample[2*i+1]*sample[2*i+1] +
                     sample[2*i+2]*sample[2*i+2]);
      if ( verbose == TRUE) {
        printf("sample[%d] = (%.2f + j %.2f) : sabs[%d] = %.2f\n",
               i,
               sample[2*i+1],
                sample[2*i+2],
                i,
                sabs[i]);
      }
    }

    for ( i = 0; i < NFFT/2; i++) {
      freq[i] = nyquist * (float) i / (NFFT/2);
      spow[i] = sabs[i]*sabs[i];
      //printf("spow[i]: %.2f\n", spow[i]);
    }

    if ( print_timetick == TRUE) {
      printf("# frequency [Hz], power  [arb.units]\n");
      for ( i = 0; i <  NFFT/2; i++) {
          printf("%.2f, %.2f\n", freq[i], spow[i]);
      } 
    } else {
      printf("# power [arb.units]\n");
      for ( i = 0; i <  NFFT/2; i++) {
        printf("%.2f\n", sabs[i]*sabs[i]);
      } 
    } 
    /* 
       free unused arrays 
       n.b. if not done allocation of hh10d_in, hh10d_out, hh10d_abs using fftw_malloc
       will result in erroneous results
     */
    free(sample);
    free(sabs);
    free(spow);
    free(freq);

  }

  /* frequency calculation using FFTW fourier transformation */
  if ( do_fftw == TRUE) {
    /* FFT using FFTW functions */
    printf("#\n# FFT using FFTW fftw_plan_dft_1d\n");

    NFFTW = size;
    printf("#\n# Number of data points (NFFTW): %d\n", NFFTW);
    nyquist = sndconfig.rate / 2;
    printf("# Nyquist frequency: %.2f (should be half of samplerate)\n", nyquist);

    /* reserve memory and initialize hh10d_in, hh10d_out and hh10d_abs */
    hh10d_in  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*(NFFTW));
    if ( hh10d_in == NULL) {
      fprintf(stderr,"can't allocate memory for hh10d_in\n");
      exit(EXIT_FAILURE);
    }

    /* allocate memory for NFFTW absolute numbers (note the +1) */
    hh10d_abs   = (double *) malloc((NFFTW+1) * sizeof(double));
    hh10d_power = (double *) malloc((NFFTW+1) * sizeof(double));
    freq        = (float *) malloc((NFFTW+1) * sizeof(float));

    hh10d_out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*(NFFTW));
    if ( hh10d_out == NULL) {
      fprintf(stderr,"can't allocate memory for hh10d_out\n");
      exit(EXIT_FAILURE);
    }

    
    for ( i = 0; i < NFFTW; i++) {
      hh10d_in[i][0] = buffer[i];
      hh10d_in[i][1] = 0.0;

      hh10d_out[i][0] = 0.0;
      hh10d_out[i][1] = 0.0;
    
      hh10d_abs[i] = 0.0;
      if ( verbose == TRUE) { 
        printf("hh10d_in[%d] = %.2f + %.2f\n", i, hh10d_in[i][0], hh10d_in[i][1]);
      }
    }

    hh10d_plan = fftw_plan_dft_1d( NFFTW,
                                   hh10d_in, 
                                   hh10d_out,
                                   FFTW_FORWARD,
                                   FFTW_ESTIMATE);
    fftw_execute(hh10d_plan);

    for ( i = 0; i < NFFTW/2; i++) {
      freq[i] = nyquist * (float) i / (NFFTW/2);
      hh10d_abs[i]   = sqrt(hh10d_out[i][0]*hh10d_out[i][0] + 
                          hh10d_out[i][1]*hh10d_out[i][1]);
      hh10d_power[i] = hh10d_abs[i]*hh10d_abs[i];

      if ( verbose == TRUE) {
        printf("hh10d_out[%d] = (%.2f + j %.2f) : hh10d_abs[%d] = %.2f\n",
               i, 
               hh10d_out[i][0],
                 hh10d_out[i][1],
               i,
               hh10d_abs[i]);
      }
    }

    printf("#\n# Frequency of maximum power: %.2f\n#\n", 
           freq[index_maxpower(hh10d_abs, NFFTW/2)]);

    if ( print_timetick == TRUE) {
      printf("# frequency [Hz], power val [arb.units]\n");
      for ( i = 0; i <  NFFTW/2; i++) {
          printf("%.2f, %.2f\n", freq[i], hh10d_power[i]);
      } 
    } else {
      printf("# power val [arb.units]\n");
      for ( i = 0; i <  NFFTW/2; i++) {
        printf("%.2f\n", hh10d_power[i]);
      } 
    } 
    fftw_destroy_plan(hh10d_plan);
    free(hh10d_in);
    free(hh10d_out);
    free(freq);
  }

  free(buffer);
  return 0;
}
