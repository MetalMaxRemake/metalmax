package org.example;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.SourceDataLine;
import java.util.List;
import java.util.Random;

public class RawStreamPlayer {

    private static SourceDataLine line = null;

    private static final int BUFFER_SIZE = 1024;

    public static void initPlayer() {
        // 采样率
        float sampleRate = 44.1000f;
        // 每个样本的位数
        int sampleSizeInBits = 8;
        // 声道数
        int channels = 1;
        // 是否带符号
        boolean signed = true;
        // 是否大端字节序
        boolean bigEndian = false;
        AudioFormat format = new AudioFormat(sampleRate * 1000, sampleSizeInBits, channels, signed, bigEndian);
        try {
            line = AudioSystem.getSourceDataLine(format);
            line.open(format, BUFFER_SIZE);
            line.start();
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    public static void releasePlayer() {
        try {
            line.drain();
            line.close();
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    public static void playBuffer(byte[] buffer) {
        try {
            line.write(buffer, 0, buffer.length);
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }

    static long tickIdx = 0;
    public static List<Pair<Long, ParkShortMessage>> midiEvent;
    static float sampleRate = 44.1000f;
    static double duration = BUFFER_SIZE / sampleRate;
    static long tickIdxDiff = (long) (duration * 110);
    static long lastConsume = 0;

    public static void initTickIdx(List<Pair<Long, ParkShortMessage>> midiEvent) {
        RawStreamPlayer.midiEvent = midiEvent;
        tickIdx = RawStreamPlayer.midiEvent.get(0).getFirst();
    }

    public static void tick() {
        tickIdx += tickIdxDiff;
//        System.out.println("time diff=" + (System.currentTimeMillis() - lastConsume) + ", tickDiff=" + tickIdxDiff);
        lastConsume = System.currentTimeMillis();
        consumeMidiEvent(tickIdx);
        byte[] buffer = generateBuffer(tickIdx);
        try {
            line.write(buffer, 0, buffer.length);
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }


    private static final byte[][] sqrtbl = {
            {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
            {1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    public static byte[] generateSquare(float frequency, float sampleRate, float volume, long tickIdx) {
        int period = (int) (sampleRate / frequency);
        double oneLast = (period * 1.d / 16.d);
//        System.out.println("period=" + period + ", oneLast=" + oneLast);
        byte[] buffer = new byte[BUFFER_SIZE];

        byte maxAmplitude = (byte) (127 * volume);  // 按音量缩放最大振幅

        int lastIdx = (int) ((BUFFER_SIZE) / oneLast);
        long tickTime = (tickIdx / tickIdxDiff);
        int phase = (int) ((tickTime * lastIdx) % 16);

        for (int i = 0; i < buffer.length; i++) {
            int idx = (int) (i / oneLast) + phase;
            buffer[i] = (sqrtbl[1][idx % 16] == 1) ? maxAmplitude : (byte) -maxAmplitude;
        }
        return buffer;
    }

    private static final byte[] tritbl =
            {
                    15, 14, 13, 12, 11, 10, 9, 8,
                    7, 6, 5, 4, 3, 2, 1, 0,
                    0, 1, 2, 3, 4, 5, 6, 7,
                    8, 9, 10, 11, 12, 13, 14, 15,
            };

    public static byte[] generateTriangle(float frequency, float sampleRate, float volume, long tickIdx) {
        int period = (int) (sampleRate / frequency);
        double oneLast = (period * 1.d / 32.d);
//        System.out.println("period=" + period + ", oneLast=" + oneLast);
        byte[] buffer = new byte[BUFFER_SIZE];

        byte maxAmplitude = (byte) (60); //6502的三角波不支持调幅

        int lastIdx = (int) ((BUFFER_SIZE) / oneLast);
        long tickTime = (tickIdx / tickIdxDiff);
        int phase = (int) ((tickTime * lastIdx) % 32);

        int level = (maxAmplitude * 2) / 15;

        for (int i = 0; i < buffer.length; i++) {
            int idx = (int) (i / oneLast) + phase;
            buffer[i] = (byte) ((tritbl[idx % 32] * level) - maxAmplitude);
        }
        return buffer;
    }

    private static final Random random = new Random();

    public static byte[] generateNoise(float frequency, float sampleRate, float volume, long tickIdx) {
        int samples = BUFFER_SIZE;
        byte[] buffer = new byte[samples];
        byte maxAmplitude = (byte) (127 * volume);  // 按音量缩放最大振幅
        if (maxAmplitude > 0) {
            byte randomValue = (byte) (random.nextInt(maxAmplitude * 2) - maxAmplitude);
            for (int i = 0; i < samples; i++) {
                buffer[i] = randomValue;
                if (i % 3 == 0) {
                    randomValue = (byte) (random.nextInt(maxAmplitude * 2) - maxAmplitude);
                }
            }
        }
        return buffer;
    }

    private static long pcm1Idx = -1;
    private static long pcm2Idx = -1;

    public static byte[] generateDPCM(int note, float volume, long tickIdx) {
        int samples = BUFFER_SIZE;
        byte[] buffer = new byte[samples];
        if (note == 1) {
            if (pcm1Idx == -1) {
                pcm1Idx = tickIdx;
            }
            long tickTime = ((tickIdx - pcm1Idx) / tickIdxDiff);
            int size = Math.min(buffer.length, Pcm1.pcm1.length - ((int) tickTime * BUFFER_SIZE));
            if (size > 0) {
                System.arraycopy(Pcm1.pcm1, (int) tickTime * BUFFER_SIZE, buffer, 0, size);
            }
        } else if (note == 2) {
            if (pcm2Idx == -1) {
                pcm2Idx = tickIdx;
            }
            long tickTime = ((tickIdx - pcm2Idx) / tickIdxDiff);
            int size = Math.min(buffer.length, Pcm2.pcm2.length - ((int) tickTime * BUFFER_SIZE));
//            System.out.println("size=" + size + ", buffer.length=" + buffer.length + ", pcm2.length=" + Pcm2.pcm2.length + ", tickTime=" + tickTime);
            if (size > 0) {
                System.arraycopy(Pcm2.pcm2, (int) tickTime * BUFFER_SIZE, buffer, 0, size);
            }
        } else {
            //todo not impl yet
        }
        return buffer;
    }

    public static void resetDPCMIdx(int note) {
        pcm1Idx = -1;
        pcm2Idx = -1;
    }

    private static byte[] generateBuffer(long tickIdx) {
        float sampleRate = 44.1000f;
        byte[] buffer = new byte[BUFFER_SIZE];
        if (channelNode[0] != 0) {
            var channelBuffer = generateSquare(midiNoteToFrequency(channelNode[0]), sampleRate, channelVolume[0], tickIdx);
            mixAudioData(buffer, channelBuffer);
        }
        if (channelNode[1] != 0) {
            var channelBuffer = generateSquare(midiNoteToFrequency(channelNode[1]), sampleRate, channelVolume[1], tickIdx);
            mixAudioData(buffer, channelBuffer);
        }
        if (channelNode[2] != 0) {
            var channelBuffer = generateTriangle(midiNoteToFrequency(channelNode[2]), sampleRate, channelVolume[2], tickIdx);
            mixAudioData(buffer, channelBuffer);
        }
        if (channelNode[3] != 0) {
            var channelBuffer = generateNoise(midiNoteToFrequency(channelNode[3]), sampleRate, channelVolume[3], tickIdx);
            mixAudioData(buffer, channelBuffer);
        }
        if (channelNode[4] != 0) {
//            System.out.println("channel5:" + channelNode[4]);
            var channelBuffer = generateDPCM(channelNode[4], channelVolume[4], tickIdx);
            mixAudioData(buffer, channelBuffer);
        } else {
            resetDPCMIdx(channelNode[4]);
        }
        return buffer;
    }

    public static byte[] mixAudioData(byte[] buffer1, byte[] buffer2) {
        for (int i = 0; i < buffer1.length; i++) {
            int mixedSample = buffer1[i] + buffer2[i];
            buffer1[i] = (byte) Math.max(Math.min(mixedSample, 127), -128);
        }
        return buffer1;
    }

    public static float midiNoteToFrequency(int note) {
        return (float) (440.0 * Math.pow(2.0, (note - 69) / 12.0)) / 1000.f;
    }

    private static void consumeMidiEvent(long tickIdx) {
        var top = midiEvent.get(0);
        while (tickIdx >= top.getFirst()) {
            //consume
            midiEvent.remove(0);
            dispatchMidiEvent(top.component2());
            top = midiEvent.get(0);
        }
    }

    private static int[] channelNode = new int[6];
    private static float[] channelVolume = new float[6];

    private static void dispatchMidiEvent(ParkShortMessage shortMessage) {
        String messageType;
        String details = "";

        int command = shortMessage.getCommand();
        int channel = shortMessage.getChannel();
        int track = shortMessage.getTrack();
        int data1 = shortMessage.getData1();
        int data2 = shortMessage.getData2();
        switch (command) {
            case 144:
                messageType = "Note On";
                details = "Note: " + getNoteName(data1) + ", Velocity: " + data2;
                if (track == 4) {
                    resetDPCMIdx(data1);
                }
                channelNode[track] = data1;
                channelVolume[track] = convertMidiVolumeToFloat(data2);
                break;
            case 128:
                messageType = "Note Off";
                details = "Note: " + getNoteName(data1) + ", Velocity: " + data2;
                if (track == 4) {
                    resetDPCMIdx(data1);
                }
                channelNode[track] = 0;
                channelVolume[track] = convertMidiVolumeToFloat(data2);
                break;
            case 176:
                messageType = "Control Change";
                details = "Controller: " + data1 + ", Value: " + data2;
                break;
            case 192:
                messageType = "Program Change";
                details = "Program: " + data1;
                break;
            case 224:
                messageType = "Pitch Bend";
                int bend = (data2 << 7) + data1 - 8192;
                details = "Bend: " + bend;
                break;
            default:
                messageType = "Unknown";
                details = "Data1: " + data1 + ", Data2: " + data2;
                break;
        }
        if (false) {
            System.out.println(command + "(" + messageType + ")" + ",Track:" + track + ",Channel:" + (channel + 1));
            System.out.println(details);
        }
    }

    public static String getNoteName(int noteNumber) {
        String[] noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int octave = (noteNumber / 12) - 1;
        int noteIndex = noteNumber % 12;
        return noteNames[noteIndex] + octave;
    }

    public static float convertMidiVolumeToFloat(int midiVolume) {
        if (midiVolume < 0 || midiVolume > 127) {
            throw new IllegalArgumentException("MIDI volume must be between 0 and 127");
        }
        // 将范围从0-127转换到0.0-0.1
        return (float) midiVolume / 127.0f * 0.08f;
    }

}
