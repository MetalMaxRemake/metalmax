//
// Created by youngpark on 2023/4/4.
//

#ifndef SUPERCUBE_NATIVE_SLES_H
#define SUPERCUBE_NATIVE_SLES_H

extern "C" void initSL();
extern "C" void releaseSL();
extern "C" short *getAudioBuffer();
extern "C" void changeAudio(int audioIdx);
extern "C" void renderCache(int preRender);
extern "C" int getAudioCount();
extern "C" int getAudioIdx();


#endif //SUPERCUBE_NATIVE_SLES_H
