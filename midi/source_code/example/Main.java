package org.example;

public class Main {

    private static boolean running = true;

    public static void main(String[] args) {
        var midiSplitChannel = MidiFile.load();
        assert midiSplitChannel != null;
        RawStreamPlayer.midiEvent = MidiFile.mergeSortedChannel(midiSplitChannel);
//        RawStreamPlayer.midiEvent = RawStreamPlayer.midiEvent.subList(1390, RawStreamPlayer.midiEvent.size());
        RawStreamPlayer.initTickIdx(RawStreamPlayer.midiEvent);
//        MidiFile.printMergeSortedChannel(RawStreamPlayer.midiEvent);
        RawStreamPlayer.initPlayer();
        while (running) {
            RawStreamPlayer.tick();
        }
        RawStreamPlayer.releasePlayer();
    }
}