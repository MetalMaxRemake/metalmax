package org.example;

import javax.sound.midi.ShortMessage;

public class ParkShortMessage extends ShortMessage {

    private int track;

    public ParkShortMessage(ShortMessage shortMessage, int track) {
        this.track = track;
        try {
            setMessage(shortMessage.getMessage(), shortMessage.getLength());
//            System.out.println("data=" + getData1() + "," + getData2() + ", length=" + length);
        } catch (Throwable tr) {
            tr.printStackTrace();
        }
    }

    public int getTrack() {
        return track;
    }

    public void setTrack(int track) {
        this.track = track;
    }
}
