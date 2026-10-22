#!/usr/bin/env python3
"""Compile gameplay against tiny Android rendering/audio stubs under ASan/UBSan."""
from pathlib import Path
import subprocess,sys
PROJECT=Path(__file__).resolve().parents[1];ROOT=PROJECT.parent;CPP=PROJECT/'app/src/main/cpp'
OUT=ROOT/'metalmax-analysis/host-tests';OUT.mkdir(parents=True,exist_ok=True)
subprocess.run([sys.executable,str(PROJECT/'tools/sprite_reference.py')],check=True)
stub=OUT/'include';(stub/'android').mkdir(parents=True,exist_ok=True)
(stub/'android/log.h').write_text('#pragma once\n#define ANDROID_LOG_INFO 4\n#define ANDROID_LOG_DEBUG 3\n#define ANDROID_LOG_WARN 5\n#define ANDROID_LOG_ERROR 6\nstatic inline int __android_log_print(int,const char*,...){return 0;}\n')
(stub/'android/native_window.h').write_text('#pragma once\nstruct ANativeWindow;\n')
(stub/'__threading_support').write_text('#include <pthread.h>\n')
for name in ['gl2.h','gl2ext.h','gl2platform.h']:
    (stub/'GLES2').mkdir(exist_ok=True);(stub/'GLES2'/name).write_text('#pragma once\n')
sources=[]
for directory in ['data','logic','maps','sprite','charset','monster','computer','store','item','tank','splash']:
    sources+=list((CPP/directory).rglob('*.cpp'))+list((CPP/directory).rglob('*.c'))
sources+=[CPP/'graphic/bitmap_render.cpp',CPP/'graphic/graphic_process.cpp',CPP/'graphic/palette_data.cpp',CPP/'audio/audio_editor_source.cpp',PROJECT/'tests/host_runtime.cpp']
command=['clang++','-std=c++17','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-pthread','-I',str(stub),'-I',str(CPP),'-x','c++']+list(map(str,sources))+['-o',str(OUT/'host_runtime')]
run=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True);(OUT/'build.log').write_text(run.stdout);print('compile',run.returncode,run.stdout[-3000:]);
if run.returncode: raise SystemExit(run.returncode)
run=subprocess.run([str(OUT/'host_runtime'),str(OUT),str(ROOT/'metalmax-analysis/sprite-reference')],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True);(OUT/'run.log').write_text(run.stdout);print(run.stdout);
if run.returncode: raise SystemExit(run.returncode)
