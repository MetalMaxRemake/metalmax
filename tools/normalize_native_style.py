#!/usr/bin/env python3
"""Normalize project-owned native identifiers to lower_snake_case.

The pass is token based: comments and string literals are preserved, so JNI
symbol strings, include paths and resource bytes are not rewritten. Platform
ABI names and the vendored NSF player stay untouched; project-owned wrappers,
gameplay code and generated bin2c symbols are normalized.
"""
from pathlib import Path
import re

PROJECT = Path(__file__).resolve().parents[1]
CPP = PROJECT / "app/src/main/cpp"

EXCLUDED_PARTS = {
    "xgm",
    "vcm",
    "map_data",
}

EXCLUDED_FILES = {
    "tile_bmp.cpp",
    "tile_bmp.h",
    "active_tile_bmp.cpp",
    "active_tile_bmp.h",
}

ALLOW_PREFIXES = (
    "JNI",
    "Java_",
    "ANative",
    "Android",
    "ANDROID_",
    "AAsset",
    "EGL",
    "GL",
    "gl",
    "EGL",
    "egl",
    "SL",
    "XA",
    "CLOCK_",
    "PRId",
    "PRIu",
)

ALLOW_EXACT = set(
    """
    EOF FILE NULL nullptr size_t
    JNIEnv jobject jclass jstring jint jboolean jsize
    JNIEXPORT JNICALL JNI_TRUE JNI_FALSE
    GetStringUTFChars ReleaseStringUTFChars
    AttachCurrentThread DetachCurrentThread GetJavaVM
    uint8_t uint16_t uint32_t uint64_t
    int8_t int16_t int32_t int64_t uintptr_t
    pthread_t pthread_mutex_t pthread_once_t pthread_mutexattr_t
    O_RDONLY R_OK SEEK_SET MAP_COUNT
    PTHREAD_ONCE_INIT PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_INITIALIZER
    PROT_READ PROT_WRITE PROT_EXEC MAP_ANONYMOUS MAP_PRIVATE MAP_FAILED
    CreateEngine Realize GetInterface CreateOutputMix CreateAudioPlayer Destroy Enqueue SetPlayState
    RegisterCallback Clear
    NSF NSFPlayer NSFPlayerConfig Load SetConfig SetPlayFreq Reset SetSong Render
    SetChannelMask SetLoopNum SetFadeTime SetPlayTime SetChannels UpdateInfinite EnableStereoPosition SetStereoPosition samplesPerSec slCreateEngine SetEnvironmentalReverbProperties
    renderWave WINDOW_FORMAT_RGBA_8888 glGetAttribLocation glDrawElements eglReleaseThread
    """.split()
)

MANUAL = {
    "LOGI": "log_i",
    "LOGD": "log_d",
    "LOGW": "log_w",
    "LOGE": "log_e",
    "Range": "range_t",
    "NpcData": "npc_data_t",
    "Entrance": "entrance_t",
    "Point": "point_t",
    "Border": "border_t",
    "ItemData": "item_data_t",
    "MonsterData": "monster_data_t",
    "ShopEntry": "shop_entry_t",
    "ShopData": "shop_data_t",
    "TreasureData": "treasure_data_t",
    "PlayerData": "player_data_t",
    "ScriptNode": "script_node_t",
    "ScriptData": "script_data_t",
}


def to_snake(name: str) -> str:
    if name in MANUAL:
        return MANUAL[name]
    text = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", name)
    text = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", text)
    text = re.sub(r"([A-Za-z])([0-9]+)", r"\1_\2", text)
    text = re.sub(r"([0-9])([A-Za-z])", r"\1_\2", text)
    return re.sub(r"_+", "_", text).lower()


def should_rewrite(token: str) -> bool:
    if token in ALLOW_EXACT or token.startswith(ALLOW_PREFIXES) or token.startswith("__"):
        return False
    if not re.search(r"[A-Z]", token):
        return False
    return to_snake(token) != token


def iter_files():
    for path in CPP.rglob("*"):
        if path.suffix not in {".c", ".cpp", ".h"}:
            continue
        if path.name in EXCLUDED_FILES:
            continue
        if any(part in EXCLUDED_PARTS for part in path.parts):
            continue
        yield path


def rewrite_source(text: str) -> str:
    out = []
    i = 0
    state = "code"
    while i < len(text):
        ch = text[i]
        if state == "code":
            if ch == '"' or ch == "'":
                quote = ch
                start = i
                i += 1
                while i < len(text):
                    if text[i] == "\\":
                        i += 2
                        continue
                    if text[i] == quote:
                        i += 1
                        break
                    i += 1
                out.append(text[start:i])
                continue
            if ch == "/" and i + 1 < len(text) and text[i + 1] == "/":
                start = i
                i = text.find("\n", i)
                if i < 0:
                    out.append(text[start:])
                    break
                out.append(text[start:i])
                continue
            if ch == "/" and i + 1 < len(text) and text[i + 1] == "*":
                start = i
                end = text.find("*/", i + 2)
                i = len(text) if end < 0 else end + 2
                out.append(text[start:i])
                continue
            if ch.isalpha() or ch == "_":
                start = i
                i += 1
                while i < len(text) and (text[i].isalnum() or text[i] == "_"):
                    i += 1
                token = text[start:i]
                out.append(to_snake(token) if should_rewrite(token) else token)
                continue
            if ch.isdigit():
                start = i
                i += 1
                while i < len(text) and (text[i].isalnum() or text[i] in "._'"):
                    i += 1
                out.append(text[start:i])
                continue
            out.append(ch)
            i += 1
    return "".join(out)


def main():
    changed = 0
    for path in iter_files():
        old = path.read_text(errors="ignore")
        new = rewrite_source(old)
        if new != old:
            path.write_text(new)
            changed += 1
    print(f"NATIVE_STYLE_NORMALIZED files={changed}")


if __name__ == "__main__":
    main()
