//
// Created by youngpark on 2023/4/4.
//

#ifndef supercube_native_sles_h
#define supercube_native_sles_h

extern "C" void init_sl();
extern "C" void release_sl();
extern "C" short *get_audio_buffer();
extern "C" void change_audio(int audio_idx);
extern "C" void render_effect(int effect_audio_id);
extern "C" int get_audio_count();
extern "C" int get_audio_idx();


#endif //SUPERCUBE_NATIVE_SLES_H
