# 按原模块组织的 bin2c 资源

本版本恢复原有maps文件和数组；新增资源分别生成到所属模块的头文件。没有集中资源包、全局资源偏移布局或APK
assets/JSON。原有455×256索引画面、BaseRender场景和JNI边界保持。

## maps 保持原样

以下文件直接恢复到仓库原有路径，内容逐字节一致：

- `maps/tile_bmp.cpp`、`maps/tile_bmp.h`：原1,904个16×16图块像素和feature。
- `maps/map_data/map_0.cpp/.h` 到 `map_239.cpp/.h`：地图格子、入口、精灵旧记录等原数组。
- `maps/map_data/map_data.cpp/.h`：原地图指针、尺寸、填充、音乐、可移动区域等索引。
- `maps/active_tile_bmp.cpp/.h`：原水面图像、波浪和水块分类数据。
- 原全局`graphic/palette_data.cpp/.h`也保持原有数据。

map.cpp通过原始`short_map_data[map]`读取格子，通过`bitmaps[id]`
绘制图块。没有将地图格子编码为11-bit块，没有将图块变成局部调色板压缩数据，也没有将这些数组移入工具目录。

`map.cpp`保留安全视口裁剪，按可见图块复制原始像素。这里仅调整读取/绘制代码，地图文件、数据结构、像素和数组未改变。

转换工具在入口记录maps全部文件哈希，在结束前再次比对。工具不向maps写入任何输出。本次489个maps数据/头文件与仓库原版一致；包括运行时代码在内，490个maps文件通过生成前后不变检查。

## 新资源分散在所属模块

```text
sprite/
  sprite_data/chr_XX.h             每个精灵CHR bank单独一份
  sprite_data/pose_table.h         四向双帧姿态规则
  sprite_data/sprite_palette.h     精灵四组颜色映射
  sprite_data/special_NNN.h        每个特殊模型单独一份
  sprite_data/system_NNN.h         每个系统OAM模型单独一份
  npc_data/npc_map_NNN.h           每个地图/额外精灵组的原始NPC字段
  npc_data/map_NNN_attributes.h    每张地图附加逻辑属性，仅供NPC/通行
  sprite_resources.cpp             只包含本模块姿态/CHR/模型头文件
  npc_resources.cpp                只包含本模块NPC与附加属性头文件

monster/
  monster_data/monster_NNN.h        每个怪物图像单独一份
  monster_data/monster_NNN_name.h   怪物名称
  monster_data/monster_stats.h      怪物基础数值
  monster_data/monster_group.h      原普通怪物组表
  monster_resources.cpp            怪物资源查询

charset/
  text_data/page_XX.h              每页文字独立的索引与字符串池
  font_data/bank_XX.h              每个源bank的中文压缩字形
  font_data/asci.h、zh_cn.h         原辅助点阵
  text_resources.cpp、font_resources.cpp

logic/
  scripts/dynamic_NNN.h            每条动态脚本
  scripts/static_NNN.h             每条静态脚本
  map_events/map_NNN_events.h      每张地图补充入口/边界信息
  status/player_data/player_N.h    各人物初值
  status/player_data/experience.h  累计经验
  treasure_data/treasure_NNN.h     各宝藏记录

item/item_data/                    按人类/战车装备、工具等类别的头文件
store/store_data/                  按商店类型和清单页的头文件
splash/splash.h、logo.h、logo_2.h   三张开场图片分别存储
audio/mm_sound_data.h              原NSF单独头文件
各模块/source_data/                尚未移植的原表按来源和物理范围分别存储
```

原有数据域不会跨模块合并。怪物/标题图片也不再与地图图块共享一个`visuals.h`
池。文件可通过模块内的资源.cpp各包含一次，避免同一头文件在多个翻译单元重复定义大数组。

`data/game_data.h`仅声明小型值视图与查询接口；`data/game_data.cpp`
只转发原始表查询和汇总字节数量。它们不定义资源数组，也不包含全部资源头文件。`data/resource_decode.h`
仅有通用整数和像素读取算法。旧`data/generated/`及其中的sprites/world/visuals/text/stats/source集中头文件已移除。

## 内存约束在模块内部实现

保持maps原设计是优先约束，不能用全局压缩来替换地图数据。其余模块按各自记录优化：

- 精灵CHR每个bank内部去重，使用byte索引和16字节NES 2bpp图块。四向双帧按ROM原姿态规则绘制，不常驻展开全部姿态。
- 特殊/系统精灵按模型或OAM部件绘制，每个模型独立头文件，不分配整张展开图片。
- 每个怪物、每张标题图片在自己的头文件中做8×8块去重。图像间不共用合并池。
- 文字每页独立，使用16-bit页内偏移；只借用当前页字符串，不建立全局char*资源表。
- 脚本每条独立，单个操作为8字节，跳转目标为条内索引。没有全部脚本混在一个连续指令池。
- 附加属性每地图独立，原maps格子和图块不会被复制到这个头文件里。
- 物品数值、人物状态、商店数量和ID仍使用能够表示真实范围的较小整数。

NPC对象88字节、ScriptMachine52字节、人物状态24字节、State156字节；存档写v2紧凑状态，保留经过校验的v1读取兼容。

`tools/build/manifest.json`
记录每个模块头文件、单独二进制中间文件、大小和哈希。当前新增模块资源约513,691字节；此数字不包含恢复的原地图数组，也不能当作整个so或进程内存。地图保留原数组后，上一版依赖地图压缩的包大小/只读段统计不再适用。

## 转换工具与校验

```bash
# 二进制转单个头文件
python3 tools/bin2c.py input.bin output.h --symbol kInput

# 生成分散到各模块的资源头文件，不写maps
python3 tools/import_reverse.py

# ASan/UBSan执行真实C++逻辑和资源读取
python3 tools/test_host.py

# Java、NDK与APK构建
python3 tools/build_debug.py

# 核对maps原文件、分散头文件、实际C++输出和APK内容
python3 tools/verify_bin2c.py
```

工具中间文件和JSON报告仅位于`tools/build`（已忽略），不进入APK。原始逆向资料留在工作区供开发使用。生成器不会把maps移动到
`tools/inputs/legacy`或把它们重新打包。

本次验证结果：

- 489个maps数据/头文件与仓库原版完全一致，490个maps文件生成前后未改变。
- 1,674个分散头文件与其二进制中间文件逐字节一致。
- 1,904张原地图图块、131个怪物、3张开场图片、3张水面图像，共2,041张图像保持数值一致。
- 179,598个原地图格子及补充逻辑属性通过回读检查。
-
5,544个普通精灵四向双帧姿态、1,353个特殊/系统模型组合与原ROM的实际6502例程生成的OAM渲染一致；详见 [角色贴图复核](sprite_review.md)。
- NPC活动、对话、存档、装备、商店、碰撞、入口和基础战斗的ASan/UBSan检查通过。
- 当前Android配置下APK打包成功，APK无assets/JSON；最新大小和摘要见开发验证报告。

结果文件为`tools/build/validation.json`，产物为同级`metalmax-analysis/metalmax-debug.apk`
。本次只修正资源组织与maps恢复，不扩大基础玩法和未完成原版语义的声明。
