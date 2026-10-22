// Resource ownership stays in the original modules. No data is defined here.
#include "game_data.h"
namespace metalmax::game_data {
const uint8_t *audio_editor_source(uint32_t,unsigned);const uint8_t *dog_system_editor_source(uint32_t,unsigned);const uint8_t *elevator_editor_source(uint32_t,unsigned);const uint8_t *event_tiles_editor_source(uint32_t,unsigned);const uint8_t *item_editor_source(uint32_t,unsigned);const uint8_t *monster_probability_source(uint32_t,unsigned);const uint8_t *sprite_editor_source(uint32_t,unsigned);const uint8_t *sprite_script_editor_source(uint32_t,unsigned);const uint8_t *tank_editor_source(uint32_t,unsigned);const uint8_t *text_editor_source(uint32_t,unsigned);const uint8_t *treasure_editor_source(uint32_t,unsigned);
const uint8_t *source_bytes(uint32_t file,unsigned length){if(const uint8_t *p=audio_editor_source(file,length))return p;if(const uint8_t *p=dog_system_editor_source(file,length))return p;if(const uint8_t *p=elevator_editor_source(file,length))return p;if(const uint8_t *p=event_tiles_editor_source(file,length))return p;if(const uint8_t *p=item_editor_source(file,length))return p;if(const uint8_t *p=monster_probability_source(file,length))return p;if(const uint8_t *p=sprite_editor_source(file,length))return p;if(const uint8_t *p=sprite_script_editor_source(file,length))return p;if(const uint8_t *p=tank_editor_source(file,length))return p;if(const uint8_t *p=text_editor_source(file,length))return p;if(const uint8_t *p=treasure_editor_source(file,length))return p;return nullptr;}

    unsigned packed_bytes() { return 514811; }
}
