#include "native_sound.h"
#include "opensl.h"
#include "mm_sound_data.h"
#include "xgm/xgm.h"
#include <atomic>
#include <pthread.h>
#include <algorithm>
#include <cstring>
#include <unistd.h>

namespace {
    constexpr unsigned k_samples = 1024, k_slots = 3;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    xgm::NSFPlayer music, effect;
    xgm::NSF music_file, effect_file;
    xgm::NSFPlayerConfig config;
    short queue[k_slots][k_samples];
    unsigned write_index = 0, read_index = 0, available = 0, effect_remaining = 0;
    std::atomic<int> requested_music(0), requested_effect(-1);
    std::atomic<bool> enabled(false);
    pthread_once_t init_once = PTHREAD_ONCE_INIT;
    bool ready = false;
    int current_music = -1;

    void initialize() {
        if (!music_file.Load(const_cast<unsigned char *>(k_nsf), k_nsf_size)
            || !effect_file.Load(const_cast<unsigned char *>(k_nsf), k_nsf_size)) { return; }
        config["MASTER_VOLUME"] = 128;
        config["PLAY_ADVANCE"] = 1;
        for (auto *player: {&music, &effect}) {
            player->SetConfig(&config);
            player->UpdateInfinite();
            player->SetChannels(1);
        }
        if (!music.Load(&music_file) || !effect.Load(&effect_file)) { return; }
        for (auto *player: {&music, &effect}) {
            player->SetPlayFreq(16000);
        }
        ready = true;
    }

    void *decode(void *) {
        pthread_once(&init_once, initialize);
        for (;;) {
            if (!ready || !enabled.load()) {
                usleep(10000);
                continue;
            }
            pthread_mutex_lock(&mutex);
            int song = (requested_music.load() % 96 + 96) % 96;
            if (song != current_music) {
                music.SetSong(song);
                music.Reset();
                current_music = song;
                available = 0;
                read_index = write_index;
            }
            int sound = requested_effect.exchange(-1);
            if (sound >= 0) {
                effect.SetSong(sound % 96);
                effect.Reset();
                effect_remaining = 16;
            }
            if (available < k_slots) {
                music.Render(queue[write_index], k_samples);
                if (effect_remaining) {
                    short scratch[k_samples];
                    effect.Render(scratch, k_samples);
                    for (unsigned i = 0; i < k_samples; i++) {
                        queue[write_index][i] = std::max(-32768, std::min(32767,
                                                                         static_cast<int>(queue[write_index][i]) +
                                                                         scratch[i]));
                    }
                    effect_remaining--;
                }
                write_index = (write_index + 1) % k_slots;
                available++;
            }
            pthread_mutex_unlock(&mutex);
            usleep(1000);
        }
        return nullptr;
    }

    pthread_once_t thread_once = PTHREAD_ONCE_INIT;

    void start_decoder() {
        pthread_t id;
        if (pthread_create(&id, nullptr, decode, nullptr) == 0) { pthread_detach(id); }
    }
}
extern "C" void init_sl() {
    pthread_once(&thread_once, start_decoder);
    enabled = true;
    start_sl_engine();
}
extern "C" void release_sl() {
    enabled = false;
    stop_sl_engine();
}
extern "C" int get_audio_count() { return 96; }
extern "C" int get_audio_idx() { return (requested_music.load() % 96 + 96) % 96; }
extern "C" void change_audio(int song) { requested_music = (song % 96 + 96) % 96; }
extern "C" void render_effect(int sound) { if (sound >= 0) { requested_effect = sound; }}
extern "C" short *get_audio_buffer() {
    // Copy before recycling the decoder slot. OpenSL copies this scratch into
    // its own two alternating output buffers, which outlive Enqueue.
    static short scratch[k_samples];
    pthread_mutex_lock(&mutex);
    if (!available) {
        pthread_mutex_unlock(&mutex);
        return nullptr;
    }
    memcpy(scratch, queue[read_index], sizeof(scratch));
    read_index = (read_index + 1) % k_slots;
    available--;
    pthread_mutex_unlock(&mutex);
    return scratch;
}
