package org.example;

import java.io.File;

public class MidiFile {

    public static HashMap<Integer, List<Pair<Long, ParkShortMessage>>> load() {
        try {
            String midiPath = "/Users/parkyu/midi/midi/mm1 2.mid";
            // 读取MIDI文件
            File midiFile = new File(midiPath);
            Sequence sequence = MidiSystem.getSequence(midiFile);

            // 获取MIDI轨道
            Track[] tracks = sequence.getTracks();

            HashMap<Integer, List<Pair<Long, ParkShortMessage>>> trackSplit = new HashMap<>();

            for (int i = 0; i < tracks.length; i++) {
                System.out.println("Track " + i + ":");
                Track track = tracks[i];

                // 遍历轨道中的MIDI事件
                for (int j = 0; j < track.size(); j++) {
                    MidiEvent event = track.get(j);
                    MidiMessage message = event.getMessage();
                    // 打印MIDI事件信息
                    if (message instanceof ShortMessage) {
                        ShortMessage sm = (ShortMessage) message;
                        List<Pair<Long, ParkShortMessage>> shortMessageList = trackSplit.get(i);
                        if (shortMessageList == null) {
                            shortMessageList = new ArrayList<>();
                        }
                        ParkShortMessage shortMessage = new ParkShortMessage(sm, i);
                        shortMessageList.add(new Pair<>(event.getTick(), shortMessage));
                        trackSplit.put(i, shortMessageList);
                    } else {
                        System.out.println("Tick: " + event.getTick() + ", Other message type: " + message.getClass());
                    }
                }
            }
            return trackSplit;
        } catch (Throwable tr) {
            tr.printStackTrace();
        }
        return null;
    }

    public static List<Pair<Long, ParkShortMessage>> mergeSortedChannel(HashMap<Integer, List<Pair<Long, ParkShortMessage>>> channelSplit) {
        ArrayList<Pair<Long, ParkShortMessage>> result = new ArrayList<>();
        var it = channelSplit.entrySet().iterator();
        while (it.hasNext()) {
            var item = it.next();
            List<Pair<Long, ParkShortMessage>> valuePairList = item.getValue();
            result.addAll(valuePairList);
        }
        Collections.sort(result, new Comparator<Pair<Long, ParkShortMessage>>() {
            @Override
            public int compare(Pair<Long, ParkShortMessage> o1, Pair<Long, ParkShortMessage> o2) {
                return Long.compare(o1.getFirst(), o2.getFirst());
            }
        });
        return result;
    }

    public static void printMergeSortedChannel(List<Pair<Long, ParkShortMessage>> mergeSortedChannel) {
        for (int i = 0; i < 1024; i++) {
            var valuePair = mergeSortedChannel.get(i);
            System.out.println("Tick: " + valuePair.component1()
                    + ", Message: " + valuePair.component2().getCommand()
                    + ", Track: " + valuePair.component2().getTrack()
                    + ", Channel: " + valuePair.component2().getChannel()
                    + ", Data1: " + valuePair.component2().getData1()
                    + ", Data2: " + valuePair.component2().getData2());
        }
    }

    public static void printMidiFile(HashMap<Integer, List<Pair<Long, ParkShortMessage>>> channelSplit) {
        try {
            var it = channelSplit.entrySet().iterator();
            while (it.hasNext()) {
                var item = it.next();
                int channel = item.getKey();
                List<Pair<Long, ParkShortMessage>> valuePairList = item.getValue();
                System.out.println("--------- channel:" + channel + " ---------");
                for (int i = 0; i < valuePairList.size(); i++) {
                    var valuePair = valuePairList.get(i);
                    System.out.println("Tick: " + valuePair.component1()
                            + ", Message: " + valuePair.component2().getCommand()
                            + ", Channel: " + valuePair.component2().getChannel()
                            + ", Data1: " + valuePair.component2().getData1()
                            + ", Data2: " + valuePair.component2().getData2());
                }
            }
        } catch (Throwable tr) {
            tr.printStackTrace();
        }
    }

}
