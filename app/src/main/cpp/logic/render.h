//
// Created by parkyu on 2023/5/11.
//

#ifndef metalmax_render_h
#define metalmax_render_h

#include "logic.h"

namespace metalmax {
class base_render {
protected:
    const uint8_t a = 0b0001;
    const uint8_t b = 0b0010;
    const uint8_t ta = 0b0100;
    const uint8_t tb = 0b1000;
    const uint8_t key_x = 0b00010000;
    const uint8_t key_y = 0b00100000;
public:
    virtual uint8_t *render(uint8_t *screen_buffer) { return screen_buffer; };

    virtual void tik_logic() {};

    virtual bool process_key(uint8_t direct_key, uint8_t function_key) { return false; };

    virtual void process_key_click(uint8_t direct_key, uint8_t function_key) {}

    virtual void on_focus() {};

    virtual void on_un_focus() {};

    virtual void on_close() {};

    virtual ~base_render() {};
};

void push(base_render *base_render);

base_render *top();

void pop();

void change_map(int map_id, int x, int y);//only support std pos

int get_current_map();

} // namespace metalmax
#endif //METALMAX_RENDER_H
