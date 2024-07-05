package org.example;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.UnsupportedAudioFileException;
import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class WavToPCM {

    public static void main(String[] args) {
        File wavFile = new File("/Users/parkyu/Documents/Untitled.wav");
        try {
//            byte[] pcmData = loadWavAsPCM(wavFile);

//            pcmData = convert16BitTo8Bit(pcmData);

            // 处理PCM数据，例如打印长度
//            System.out.println("PCM Data Length: " + pcmData.length);
//            byte[] subPcmData = new byte[8192];
//            System.arraycopy(pcmData, 22760, subPcmData, 0, subPcmData.length);
//            printByteArrayAsJavaCode(subPcmData);
            RawStreamPlayer.initPlayer();
            RawStreamPlayer.playBuffer(Pcm1.pcm1);
            Thread.sleep(100);
            RawStreamPlayer.playBuffer(Pcm1.pcm1);
            Thread.sleep(100);
            RawStreamPlayer.playBuffer(Pcm1.pcm1);
            Thread.sleep(100);
            RawStreamPlayer.playBuffer(Pcm2.pcm2);
            Thread.sleep(100);
            RawStreamPlayer.playBuffer(Pcm2.pcm2);
            Thread.sleep(100);
            RawStreamPlayer.playBuffer(Pcm2.pcm2);
            RawStreamPlayer.releasePlayer();
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    public static byte[] convert16BitTo8Bit(byte[] pcm16bit) {
        int length = pcm16bit.length / 2;
        byte[] pcm8bit = new byte[length];

        for (int i = 0; i < length; i++) {
            // Combine two bytes to make a 16-bit sample (little endian)
            int sample16bit = (pcm16bit[2 * i + 1] << 8) | (pcm16bit[2 * i] & 0xFF);

            // Convert the 16-bit sample to an 8-bit sample
            pcm8bit[i] = (byte) (sample16bit >> 8);  // Using the higher byte for 8-bit sample
        }

        return pcm8bit;
    }

    public static void printByteArrayAsJavaCode(byte[] byteArray) {
        StringBuilder sb = new StringBuilder();
        sb.append("byte[] pcm = {");

        for (int i = 0; i < byteArray.length; i++) {
            if (i % 10 == 0) {
                sb.append("\n");
            }
            sb.append(String.format("(byte) 0x%02X", byteArray[i]));
            if (i < byteArray.length - 1) {
                sb.append(", ");
            }
        }

        sb.append("};");
        System.out.println(sb.toString());
    }

    /**
     * 加载WAV文件并将其数据转换为PCM样本数组
     *
     * @param wavFile WAV文件
     * @return PCM样本数组
     * @throws IOException
     * @throws UnsupportedAudioFileException
     */
    public static byte[] loadWavAsPCM(File wavFile) throws IOException, UnsupportedAudioFileException {
        AudioInputStream audioInputStream = AudioSystem.getAudioInputStream(wavFile);
        AudioFormat format = audioInputStream.getFormat();

        // 确保WAV文件使用的是PCM编码
        if (format.getEncoding() != AudioFormat.Encoding.PCM_SIGNED && format.getEncoding() != AudioFormat.Encoding.PCM_UNSIGNED) {
            throw new UnsupportedAudioFileException("WAV文件不使用PCM编码");
        }

        // 获取WAV文件数据的字节数
        int bytesPerFrame = format.getFrameSize();
        long numFrames = audioInputStream.getFrameLength();
        long numBytes = numFrames * bytesPerFrame;

        // 创建字节数组以存储PCM数据
        byte[] pcmData = new byte[(int) numBytes];

        // 读取WAV文件数据到字节数组
        int bytesRead = audioInputStream.read(pcmData);
        if (bytesRead != numBytes) {
            throw new IOException("未能完全读取WAV文件");
        }

        audioInputStream.close();

        return pcmData;
    }

    /**
     * 将字节数组转换为短整型数组
     *
     * @param byteArray 字节数组
     * @param format    音频格式
     * @return 短整型数组
     */
    public static short[] byteArrayToShortArray(byte[] byteArray, AudioFormat format) {
        int bytesPerSample = format.getSampleSizeInBits() / 8;
        int numSamples = byteArray.length / bytesPerSample;
        short[] shortArray = new short[numSamples];

        ByteBuffer byteBuffer = ByteBuffer.wrap(byteArray);
        byteBuffer.order(format.isBigEndian() ? ByteOrder.BIG_ENDIAN : ByteOrder.LITTLE_ENDIAN);

        for (int i = 0; i < numSamples; i++) {
            shortArray[i] = byteBuffer.getShort(i * bytesPerSample);
        }

        return shortArray;
    }
}

