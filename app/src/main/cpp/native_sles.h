//
// Created by youngpark on 2023/4/4.
//

#ifndef SUPERCUBE_NATIVE_SLES_H
#define SUPERCUBE_NATIVE_SLES_H

extern "C" void initSL();
extern "C" short *getAudioBuffer();
extern "C" void changeAudio(int audioIdx);
extern "C" int getAudioCount()


#endif //SUPERCUBE_NATIVE_SLES_H
