package com.park.metalmax.sound;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class StreamPlayer {

    private AudioTrack mAudioTrack;

    public void initAudioTrack(int sampleRateInHz, int nb_channels) {
        int channelConfig;
        if(nb_channels == 1){
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;//单声道
        }else if(nb_channels == 2){
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;//双声道立体声
        }else{
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        }

        ////根据采样率，采样精度，单双声道来得到buffer的大小
        int bufferSize = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, AudioFormat.ENCODING_PCM_16BIT);

//        AudioTrack(int streamType, int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode)
        // AudioFormat.ENCODING_PCM_16BIT 设置音频数据块是8位还是16位，这里设置为16位。
        // AudioTrack.MODE_STREAM设置模式类型，在这里设置为流类型，第二种MODE_STATIC
        mAudioTrack = new AudioTrack(
                AudioManager.STREAM_MUSIC, // 指定流的类型
                sampleRateInHz,// 设置音频数据的采样率
                channelConfig,
                AudioFormat.ENCODING_PCM_16BIT,
                bufferSize, AudioTrack.MODE_STREAM);
    }

    public void play() {
        mAudioTrack.play();//very important  启动音频设备
    }

    public void stop() {
        mAudioTrack.stop();
    }

    public synchronized void playTrack(short[] buffer, int length){
        if(null != mAudioTrack && mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING){
            mAudioTrack.write(buffer, 0, length);
        }
    }

}
