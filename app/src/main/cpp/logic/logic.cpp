#include "logic.h"
#include "render.h"
#include "render_map.h"
#include "render_splash.h"
#include "../graphic/native_graphic.h"
#include "../charset/charsets.h"
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <vector>
#include <cstdio>

namespace metalmax {
    const uint8_t up = 1, down = 2, left = 4, right = 8;
    static std::vector<base_render *> stack;
    static std::vector<base_render *> retired;
    static map_render *s_map_render = nullptr;
    static pthread_mutex_t logic_mutex;
    static pthread_once_t mutex_once = PTHREAD_ONCE_INIT;
    static pthread_t logic_thread;
    static bool started = false;
    static std::atomic<bool> running(false), paused(true);
    static std::atomic<uint8_t> direct_key(0), function_key(0);
    static uint8_t last_direct = 0, last_function = 0;

    static void init_mutex() {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&logic_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    void lock_logic() {
        pthread_once(&mutex_once, init_mutex);
        pthread_mutex_lock(&logic_mutex);
    }

    void unlock_logic() { pthread_mutex_unlock(&logic_mutex); }

    base_render *top() { return stack.empty() ? nullptr : stack.back(); }

    void push(base_render *render) {
        if (!render) { return; }
        lock_logic();
        if (stack.size() >= 32) {
            retired.push_back(render);
            unlock_logic();
            return;
        }
        if (top()) { top()->on_un_focus(); }
        if (auto *map = dynamic_cast<map_render *>(render)) { s_map_render = map; }
        stack.push_back(render);
        render->on_focus();
        unlock_logic();
    }

    void pop() {
        lock_logic();
        base_render *old = top();
        if (old) {
            old->on_un_focus();
            old->on_close();
            stack.pop_back();
            if (old == s_map_render) { s_map_render = nullptr; }
            retired.push_back(old);
            if (top()) { top()->on_focus(); }
        }
        unlock_logic();
    }

    int get_current_map() { return s_map_render ? s_map_render->get_map_id() : -1; }

    void change_map(int map_id, int x, int y) {
        lock_logic();
        if (s_map_render) {
            while (top() && top() != s_map_render) { pop(); }
            s_map_render->update_map(map_id, x, y);
        }
        unlock_logic();
    }

    void update_direct_key(uint8_t key) { direct_key.store(key); }

    void update_function_key(uint8_t key) { function_key.store(key); }

    void set_logic_paused(bool value) {
        paused.store(value);
        if (value) {
            direct_key = 0;
            function_key = 0;
        }
    }

    static void collect() {
        for (auto *render: retired) { delete render; }
        retired.clear();
    }

    static void tick() {
        lock_logic();
        uint8_t direct = direct_key.load(), function = function_key.load();
        base_render *render = top();
        if (render && !render->process_key(direct, function)) {
            if (top() == render) {
                render->process_key_click(direct & ~last_direct, function & ~last_function);
            }
        }
        last_direct = direct;
        last_function = function;
        if (top()) { top()->tik_logic(); }
        collect();
        unlock_logic();
    }

    static uint8_t *render_screen(uint8_t *buffer) {
        lock_logic();
        for (auto *render: stack) { buffer = render->render(buffer); }
        char fps[32];
        snprintf(fps, sizeof(fps), "FPS %d", native_graphic::get_fps());
        render_asci_text(buffer, fps, 0, 0);
        unlock_logic();
        return buffer;
    }

    static void *logic_task(void *) {
        while (running.load()) {
            if (!paused.load()) { tick(); }
            usleep(16000);
        }
        return nullptr;
    }

    void init_logic() {
        lock_logic();
        native_graphic::set_render_callback(render_screen);
        if (!started) {
            push(new splash_render);
            running = true;
            if (pthread_create(&logic_thread, nullptr, logic_task, nullptr) ==
                0) { started = true; }
            else {
                running = false;
                log_e("logic", "Cannot create logic thread");
            }
        }
        unlock_logic();
    }

    void release_logic_thread() {
        running = false;
        if (started) {
            pthread_join(logic_thread, nullptr);
            started = false;
        }
        lock_logic();
        while (top()) { pop(); }
        collect();
        unlock_logic();
    }
} // namespace metalmax
