# metalmax
Full C / C ++ / arm-v8 assembly remake metalmax

maps 保持原有 `map_N.cpp/map_N.h`、`map_data.cpp/map_data.h` 和 `tile_bmp.cpp/tile_bmp.h`
，地图贴图数据不转换、不压缩、不迁移。新增逆向资源通过bin2c生成到精灵、怪物、文字、脚本、物品等各自模块的头文件中，分别编入so，APK不携带assets/JSON。

当前已接入四向双帧精灵、启动动画、起名与主角家开场剧情、中文对话、基础交谈、调查、角色状态、个人装备/物品、战车地图能力、战车状态/装备、完整列表商店、主要服务/计算机、存档，以及角色/战车回合战斗。文件组织与工具见 [按模块组织的bin2c资源](docs/bin2c_resource_layout.md)
；玩法边界见 [功能实现](docs/reverse_resource_integration.md)。

角色贴图已重新复核并修正主角模型及特殊/系统精灵拼接。原角色、新角色、所有方向与NPC对应图见 [贴图复核记录](docs/sprite_review.md)
和 [完整图集](docs/sprite_previews/index.html)。

```bash
python3 tools/import_reverse.py
python3 tools/test_host.py
python3 tools/review_sprites.py
python3 tools/build_debug.py
python3 tools/verify_bin2c.py
```

用c c++ arm-v8汇编重写的 重装机兵，目标Android平台
图像使用opengl es 2，通过alpha方式生成2D贴图实现硬件加速
音频使用opensl es，暂时使用nsfplayer生成PCM数据传给opensl播放，后续计划自建音频引擎，分离音频数据

# 游戏原始数据
thanks：AFoolLove提供数据支持，欢迎关注MetalMaxRe https://github.com/AFoolLove/MetalMaxRe
使用数据请遵守以上工程作者的协议，详细请查看上述链接

# 游戏音频数据
## version 1: 
使用nsfplayer加载nsf生成pcm
## version 2: 
使用完全自建的midi引擎，音频格式为自定义格式（参考standard midi format 1），生成pcm。
原始数据为通过修改的nsfplayer生成midi，再二次转为自定义格式。
