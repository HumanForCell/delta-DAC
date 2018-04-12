/**
** デルタD/Aコンバータを模したステレオ音声のハイレゾ化フィルタ
** version 1.00 Copyright(c)2017 yamashiro kaname
**  注意）
** 本プログラムは無保証です。
** 著作者は本プログラムによって発生したいかなる損害について責任を持ちません。
*/
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define OUT_FREQ (48000*4)
#define IN_FREQ (48000)
//#define BUF_TIME 0.04
#define BUF_TIME 1.0

int main (int argc,char *argv[])
{
	int err;
	snd_pcm_t* in_handle;
	snd_pcm_t* out_handle;
	snd_pcm_sframes_t frames;
	short in_buffer[IN_FREQ * 5];
	int out_buffer[OUT_FREQ * 5];
	char *in_device = "default";
	char *out_device = "default";
	
	if (getenv("PCM_CAPT_DEVICE") != NULL){
		in_device= getenv("PCM_CAPT_DEVICE");
	}
	printf("環境変数 PCM_CAPT_DEVICE=%s\n",in_device);
	
	if (getenv("PCM_PLAY_DEVICE") != NULL){
		out_device= getenv("PCM_PLAY_DEVICE");
	}
	printf("環境変数 PCM_PLAY_DEVICE=%s\n",out_device);

	if ((err= snd_pcm_open(&in_handle,in_device,SND_PCM_STREAM_CAPTURE,0))< 0){
		printf ("Capture open error: %s\n", snd_strerror(err));
		return 1;
	}
	if ((err= snd_pcm_set_params(in_handle,SND_PCM_FORMAT_S16_LE,SND_PCM_ACCESS_RW_INTERLEAVED,2,IN_FREQ,1
		,BUF_TIME*2000*1000))< 0){
		printf ("Capture open error: %s\n", snd_strerror(err));
		return 1;
	}
	if ((err= snd_pcm_open(&out_handle,out_device,SND_PCM_STREAM_PLAYBACK,0))< 0){
		printf("Playback open error: %s\n",snd_strerror(err));
		return 1;
	}
	if ((err= snd_pcm_set_params(out_handle,SND_PCM_FORMAT_S32_LE,SND_PCM_ACCESS_RW_INTERLEAVED,2,OUT_FREQ,1
		,BUF_TIME*2000*1000))< 0){
		printf("Playback open error: %s\n",snd_strerror(err));
		return 1;
	}
	#define VOL_HI 50000.0
	#define VOL_TR 7000.0
	#define VOL_MID (VOL_HI/2.0)

	int tmp_in_L= 0;
	int tmp_in_R= 0;
	double vol_L= VOL_MID;
	double vol_R= VOL_MID;
	double vol2_L= 0.0;
	double vol2_R= 0.0;
	double vol_max_L= -VOL_HI*65536.0;
	double vol_max_R= -VOL_HI*65536.0;
	double vol_min_L= VOL_HI*65536.0;
	double vol_min_R= VOL_HI*65536.0;
	
	time_t time0,time1,time2=1;
	time(&time0);
  
	while(1){
		time(&time1);
		if (time1-time0 >= time2){
			printf("\033[1GNow play: %d[sec] Lmin:%5.0lf Lmax:%5.0lf Rmin:%5.0lf Rmax:%5.0lf  ",time2
				,vol_min_L/65536.0,vol_max_L/65536.0
				,vol_min_R/65536.0,vol_max_R/65536.0); fflush(stdout);
			time2 += 1;
			vol_max_L= -VOL_HI*65536.0;
			vol_max_R= -VOL_HI*65536.0;
			vol_min_L= VOL_HI*65536.0;
			vol_min_R= VOL_HI*65536.0;
		}
		
		frames= snd_pcm_readi(in_handle,in_buffer,BUF_TIME*IN_FREQ);
		if (frames< 0){
			frames= snd_pcm_recover(in_handle,frames,0);
		}
		if (frames< 0){
			printf ("snd_pcm_readi failed: %s\n",snd_strerror(frames));
		}
		int i,j= 0;
		for (i=0;i< frames;i++){
			double on_diff_L= in_buffer[i*2]- tmp_in_L;
			double on_diff_R= in_buffer[i*2+1]- tmp_in_R;

			int k;
			for (k=0;k< 4;k++){
	  			#define VOL_AMP (65500.0*4.0)
				double _m= 8.0;
			
				if (on_diff_L >= 0){
					vol_L += on_diff_L/_m* ((VOL_HI-VOL_TR)-vol_L)/(VOL_HI-VOL_TR);
				} else{
					vol_L += on_diff_L/_m* (vol_L-VOL_TR)/(VOL_HI-VOL_TR);
				}
				
				if (vol_L < 0) vol_L= 0;
				if (vol_L > VOL_HI) vol_L= VOL_HI;
			
				if (on_diff_R >= 0){
					vol_R += on_diff_R/_m* ((VOL_HI-VOL_TR)-vol_R)/(VOL_HI-VOL_TR);
				} else{
					vol_R += on_diff_R/_m* (vol_R-VOL_TR)/(VOL_HI-VOL_TR);
				}
				
				if (vol_R < 0) vol_R= 0;
				if (vol_R > VOL_HI) vol_R= VOL_HI;
			
				vol2_L= (vol_L-VOL_MID)*VOL_AMP;
				vol2_R= (vol_R-VOL_MID)*VOL_AMP;
			
				out_buffer[j*2]= vol2_L;
				out_buffer[j*2+1]= vol2_R;
				j++;
			}
			if (vol_max_L < vol2_L) vol_max_L= vol2_L;
			if (vol_max_R < vol2_R) vol_max_R= vol2_R;
			if (vol_min_L > vol2_L) vol_min_L= vol2_L;
			if (vol_min_R > vol2_R) vol_min_R= vol2_R;
		
			tmp_in_L= in_buffer[i*2];
			tmp_in_R= in_buffer[i*2+1];
		}
		
		frames= snd_pcm_writei(out_handle,out_buffer,BUF_TIME*OUT_FREQ);
		if (frames< 0){
			frames= snd_pcm_recover(out_handle,frames,0);
		}
		if (frames< 0){
			printf("snd_pcm_writei failed: %s\n",snd_strerror(frames));
		}
	}
	snd_pcm_close(in_handle);
	snd_pcm_close(out_handle);
	return 0;
}
