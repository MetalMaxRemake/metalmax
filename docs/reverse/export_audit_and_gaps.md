# Metal Max 导出审计、错误修正与数据缺口分析

审计日期：2026-10-21。对象是当前 `metalmax-reverse` 导出、MetalMaxRe编辑器、FCEUX内核与原始`mm.nes`
。原始ROM没有修改。

结论：**原始ROM字节、地图内容、全部文本页和原剧情脚本保存正确，但上一轮部分派生数据和关联确实有问题。**
原先只证明汇编能重建ROM，不足以证明JSON含义、NPC脚本关联、图像上下文或音频编号正确。本轮已修正明确问题，并建立独立字段与像素校验。

当前独立数据校验 **6375/6375通过**，关联校验 **2121/2121通过**，完成的字体NMI上传 **174/174通过**
。这些结果有具体范围，不扩展成“所有游戏算法都已证明”。

## 1. 已确认并修复的问题

| 问题                      | 旧结果的影响                                     | 修正                                                             |
|-------------------------|--------------------------------------------|----------------------------------------------------------------|
| 空值Map.Entry使用Map.of     | 三处调查点变成unavailable                         | 用可含null的Map保留坐标和值；墓碑、乌鲁米井和母亲照片恢复                               |
| 反射读取父类同名字段覆盖子类          | TextScript.Event的page被null覆盖，39个条件对话节点关联缺失 | 子类字段优先；保留trueLine/falseLine及事件条件                               |
| 静态脚本编号错位                | NPC talk2=N错误关联editor文件N                   | 游戏1..108对应editor0..107；0为无脚本，dynamic0..168不偏移                  |
| 音频编号错位                  | gameID与记录数组不一致，缺少最后ID123预览                 | game1..123对应editor0..122，ID0仅停止；补123并重新精确切片                    |
| 装备攻击/防御实际值缺失            | 只查attack/defense，没有识别继承的value              | 按类型解释value索引，补attack_value、defense_value、capacity、重量与price特殊标志 |
| 战斗模型图像规则不完整             | 忽略offset、tile0透明、palette                   | 与BattleSpriteModel/SpriteModelHelper一致；同时标明静态CHR配置限制           |
| 普通模型palette统一取0         | 模型head中的palette未应用                         | 使用getPaletteIndex；原模型和像素索引仍保留                                  |
| 标题/logo图像猜测             | 使用未证实CHR和第一组palette                        | 用实际title帧PPU pattern/palette与布局合成；logo缺自身上下文明确标注               |
| 错把25..3D重复bank视图称字体     | 字模来源含义错误                                   | 重命名raw_bank_views；确认20..23实际字模并导出1671个glyph                    |
| 资源地址声明当作实际读取范围          | 遗漏字段或大量无关数据归类                              | 独立按Load消费长度校验并保存expanded/source记录                              |
| 汇编CPU地址按bank最小窗口猜测      | 部分寻址或跨bank解释可能偏移                           | 加逐指令CPU上下文；分支地址校验；静态候选独立标记                                     |
| snapshot调用ARead污染read证据 | 预览快照造成伪资源引用                                | snapshot suppressTrace；新增真实prg-data-reads.csv                  |
| 音频片段按平均samples/frame切分  | 帧边界近似，预览混入相邻片段                             | 保存实际每帧sample_count，精确切分                                        |

空值调查点恢复结果：塔玛墓碑 map73、(3,1)；乌鲁米井 map5F、(6,5)；母亲照片 map03、(8,2)
。这些是ROM坐标，名称来自编辑器注释。

## 2. 哪些数据已与源代码对应

| 范围             | 独立核对方法                                                  | 当前结果           |
|----------------|---------------------------------------------------------|----------------|
| 原始资源区          | 每个bin与原ROM对应offset直接比较                                  | 一致             |
| 普通地图239张       | 从ROM重新读7bit RLE，比较tile矩阵与Source；独立从CHR、组合、属性和palette渲染  | 全部内容与PNG像素一致   |
| 地图属性           | 从两卷索引读取基础24字节，核对flags/宽高/指针/图块/音乐                       | 一致             |
| 世界地图           | 独立解析2bit、1bit、一字节索引及四池                                  | 65536tile一致    |
| 文本22页          | 所有bytes_hex串联后与原页比较                                     | 2669段、每页字节完全一致 |
| NPC            | 250索引组，每个数量+6bytes与原记录比较                                | 639条记录一致       |
| 物品             | 按实际字段表读取221项的flags、动画、重量、value、defense、price及引擎capacity | 一致             |
| 数值             | 独立读71一字节、101二字节、61特殊倍率项                                 | 233项一致         |
| 怪物             | 131项分表字段、模型尺寸、CHR索引、调色板与布局原字节                           | 一致             |
| 精灵模型           | 69map、54system、254battle的原始索引及结构                        | 非空记录原字节一致      |
| 人物             | 初始金钱和字段major数组、装备、经验                                    | 实际171bytes一致   |
| 战车             | 18条自有/出租记录、SP、重量、位置                                     | 一致             |
| 宝藏、调查点、计算机、经验表 | 原表分字段读取与对象比较                                            | 一致             |
| 剧情脚本           | 所有277原bin直接比索引区、重编译bin比较                                | 一致，游戏ID关联已修正   |
| 音频             | 实际one-based指针、recordhead、四channelptr、按物理窗口bin切片         | 一致；流语义仍部分解析    |
| 运行字体           | 174次完成上传的scratch与PPUCHR8byte块比较                         | 全部一致           |

独立校验见 [independent-checks.json](../audit/independent-checks.json)、[relations-validation.json](../audit/relations-validation.json)
。汇编全文件重组见 [verification.json](../analysis/verification.json)。

## 3. source地址与declared地址差异

源码有些DataAddress只是起点约定或写错了区间，不是完整结构边界：

- `PlayerEditor`声明115bytes，但Load实际读171bytes；所有读取已核对并保存来源。
- `globalSpritePalettes`实际跳过每行首色后读3色，四行共16bytes，不能只导12bytes。
- `monsterAttributes`声明130bytes，Load按131怪物读；`monsterModelSize`声明78bytes，读取79项。
- 电梯顶层只读5bytes，但结束地址误写到25154，不能将声明范围中的代码都视为电梯数据。
- 多项世界块池、赏金和怪物攻击序列地址没有结束值，需从消费者与相邻表确定边界。

新工具以实际消费范围与原字节为依据，保留原始声明和expanded记录方便定位源编辑器问题。没有修改用户原源码。

## 4. 新确认的用途与缺失资源

### 汉字字模

bank0A的9ADA、9AE9、9AF7和9B03根据字符第一字节的列号、第二字节的行号确定PRG
bank和CPU指针。真实字体位于bank20..23。单字18bytes由三个4列带组成，各6bytes表示12行，每byte高半字节先、低半字节后。

已导出1671个明确编码的12×12
glyph、原18bytes、字符名、CPU指针和文件offset：[glyphs/index.json](../resources/fonts/glyphs/index.json)
。glyph几何来自ROM位移/掩码代码，字符名字仍来自fonts.txt，未定义空槽位没有擅自命名。

此前25..3D是完全重复扩展内容，可视化有价值，但与字库没有已证实关联。已移到graphics/raw_bank_views，原图在audit/previous保留。

### 音频与DPCM

有效音乐/效果ID为1..123，0停止。声道指针8000..9FFF通过R6读取0D/0E/0F组；指针A000..BFFF直接属于音频驱动bank1D，112..123是这类流。AA6F组3值FF表示不切R6，不是bank0A。

音频command表实际在CPUAA84，53个16位handler地址；此前工具的候选起点AA94晚了16bytes。instrument表A010可确认16个ptr，后面是参数数据而非更多ptr。

DPCM handler AE9B调用CFC0表，每项rate/address/length三字节，导出14项描述和.dmc原采样。长度来自
`16*lengthReg+1`，CPU起点来自`C000+64*addressReg`。APU音符/包络/loop的全部语义还没有完成转译成MIDI，原流和WAV保留。

### 剧情、场景与RNG

恢复了108条静态脚本真正游戏编号、39个按事件分支的文本引用，以及NPC
action到169动态脚本的关联。脚本处理表target-1的116项、静态指针表ID0sentinel和音频ID0sentinel都作为独立bin保存。

场景加载A6B8..A6F5中的地图ID/事件对照表控制城镇或特殊NPC组标志；数据已导出，不把表中的备注扩展成必须线性通关顺序。

RNG更新函数D02E已确认：用状态37/38的移位、加法和ADC进位更新；D01D等待帧时反复调用，战斗决策也调用。它既依赖旧状态也涉及调用时6502进位，不能简化成忽略carry的常见LCG公式。当前记录完整指令，精确分布及所有调用约束仍待验证。

## 5. 未分类区域的变化

上一轮未分类 **99,497bytes**；本轮 **44,840bytes**
。类别是“已经给出源格式/用途边界”，不表示其中所有业务语义已恢复。尤其音频流保留next-pointer/bank-end范围，不能等价为已经识别每个音符的实际结束。

当前new_regions索引：

| 区域                             |  文件起点 |   长度 | 依据及置信度                                                                                                                 |
|--------------------------------|------:|-----:|------------------------------------------------------------------------------------------------------------------------|
| chinese_glyph_bank_20          | 40010 | 8192 | packed12x12Chinese font tables and tail code in bank23；source_derived_font_bank                                        |
| chinese_glyph_bank_21          | 42010 | 8192 | packed12x12Chinese font tables and tail code in bank23；source_derived_font_bank                                        |
| chinese_glyph_bank_22          | 44010 | 8192 | packed12x12Chinese font tables and tail code in bank23；source_derived_font_bank                                        |
| chinese_glyph_bank_23          | 46010 | 8192 | packed12x12Chinese font tables and tail code in bank23；mixed_font_and_tail_code                                        |
| item_equipment_fields          | 22285 |  698 | equipment flags, animations, weights, attack/defense indices and all221 prices；source_derived                          |
| monster_attack_mode_streams    | 2277D |  594 | monster attack/action sequences;594bytes, higher-level format still under analysis；source_bounded_semantics_partial    |
| script_handler_rts_table       | 34993 |  232 | dynamic/static script opcode handlers target-1；source_derived                                                          |
| audio_record_pointer_table     | 3AEEE |  248 | game audioID0 sentinel and123one-based record pointers；source_derived                                                  |
| static_script_pointer_table    | 27387 |  218 | static scriptID0 sentinel and108one-based script pointers；source_derived                                               |
| audio_command_pointer_table    | 3AA84 |  106 | audio commands dispatcher atAA73, last E-sample handler；source_derived                                                 |
| audio_instrument_pointer_table | 3A020 |   32 | instrument/envelope substream table;16 valid pointers, following8bytes are parameter data；source_derived               |
| map_sprite_event_tables        | 34706 |   52 | 23scene/mapIDs paired with23eventIDs and special map/event remapping；source_derived                                    |
| direction_to_input_table       | 7D144 |   15 | input directions decoded from low nibble of controller state；source_derived                                            |
| rng_state_step                 | 7D03E |   31 | 16bit state update based on37/38, shift/add and ADC carries；executed_code_semantics                                    |
| dpcm_descriptor_table          | 7CFD0 |   42 | DPCM rate/address/length descriptors, sample ID selects triple；source_derived                                          |
| audio_stream_bank_0D           | 1A010 | 8192 | channel sequence bytecode (note/duration/instrument/loop/etc), including shared tails；source_bounded_semantics_partial |
| audio_stream_bank_0F           | 1E010 | 8192 | channel sequence bytecode (note/duration/instrument/loop/etc), including shared tails；source_bounded_semantics_partial |
| audio_stream_bank_1D           | 3B720 | 2288 | channel sequence bytecode (note/duration/instrument/loop/etc), including shared tails；source_bounded_semantics_partial |
| audio_stream_bank_0E           | 1C010 | 8192 | channel sequence bytecode (note/duration/instrument/loop/etc), including shared tails；source_bounded_semantics_partial |

完整缺口见 [gap-analysis.json](../audit/gap-analysis.json)，保留每段未知区域、实测读字节数量与消费者指令，便于继续追踪。

## 6. 静态推测审计

当前指令分级：实测executed **12162条**，递归静态 **3070条**，static_candidate **10815条**
。候选入口来源包括终止指令后模式与指针表；主汇编可以字节准确但候选仍可能解码数据，因此不能作为原作者源代码。

当前187条原静态未解析边中，68条具有实测目标，119条仍缺bank/动态指针上下文。实测目标只证明这个场景选择了该目标，不能穷尽所有游戏状态。

constant传播现在在未知JSR后清除寄存器、内存指针和bank假设，处理会改变A/内存的指令；将读ROM数据和写mapper寄存器区分。CPU上下文由新instruction-contexts或原trace记录，不能再用某bank同时出现的所有窗口套到每条指令。

7个执行过的重叠入口单独表示，不将重叠的同一字节重复写入重组ROM。函数列表仍为入口/区段索引，不是精确恢复的C语言函数边界。

### 原版固定区与汉化固定区对应

继续按相同偏移比较：bank1E与3E有8105/8192bytes相同、87bytes不同；bank1F与3F有8021/8192bytes相同、171bytes不同。前者可与1881条当前已解码指令对应，后者与2953条对应。这支持它们主要保存继承的原版固定代码/数据，而汉化复制并修改了新的末尾bank。

对应索引见 [original-fixed-bank-correspondence.json](../audit/original-fixed-bank-correspondence.json)
。这属于字节及逻辑对应证据，物理bank1E/1F没有在本次轨迹中执行，不能将对应副本记入运行覆盖率。

## 7. 仍存在的限制和数据缺口

- LOGO自身显示时的完整bank/palette上下文尚未单独采集；其preview明确使用title上下文。
- battle/system模型的全场景实际CHR选择还没穷尽；结构、offset、透明、palette规则已修正，静态组合配置保留来源。
- 未命名的怪物行动序列594bytes、部分音频流、DPCM调用条件和特殊scene分发表仍只有范围与部分消费者。
- 战斗伤害、命中、逃跑与所有RNG公式尚未完整恢复；已验证HP/SP扣减、奖励及一个普通战斗路径。
- 非开场剧情、赏金首战和终局分支未全部动态执行，事件备注仍来自编辑器。
- 真正的源符号与精确函数边界无法由该ROM自动恢复；当前提供带证据的分析标签。
- WAV是十秒预览：当前124命令ID含0停止与123记录，不声称每条长曲已完整播放。

### 仍需追踪的较大区域

| 文件范围         |    大小 | 当前证据                                                         |
|--------------|------:|--------------------------------------------------------------|
| 3C010..4000F | 16384 | bank1E/1F含与固定3E/3F相似的原版逻辑/数据，但本次运行未访问这些物理bank；是否备份或仍有特定调用未证明 |
| 48010..4A00F |  8192 | bank24未观察到读取/执行，不能直接称空白或字体                                   |
| 7C010..7CF4F |  3904 | 前段包括DPCM样本区；NMI前数据已部分确认DPCM，剩余结构没有全部分离                       |
| 232B0..23CA4 |  2549 | 有1158个字节实际被文本读函数FAB2和战斗参数读B203访问，属于文本/战斗命令相关数据，细分记录格式未全部恢复   |
| 314A4起       |  2274 | bank18服务逻辑和表混合的静态未分类区；开场轨迹未命中，不能凭线性解码判为确定代码                  |
| B6D4起        |  1852 | 普通地图空间尾部到属性索引之前的未分类区；是否剩余地图/保留数据仍需查指针来源                      |
| 3B2FC起       |  1060 | APU驱动通过多条指令读取，包含instrument/envelope等子流；已知消费者而没有完整命令解码        |

以上范围由[remaining-large-gaps.json](../audit/remaining-large-gaps.json)
保留起点、大小、读字节和读指令。即使有实际消费者，也只按该范围表达部分用途，不把整段全部命名。

## 8. 复现与持续校验

```bash
python3 metalmax-reverse/tools/export_resources.py
python3 metalmax-reverse/tools/analyze_game.py
python3 metalmax-reverse/tools/audit_data.py
python3 metalmax-reverse/tools/audit_graphics.py
python3 metalmax-reverse/tools/audit_relations.py
python3 metalmax-reverse/tools/analyze_gaps.py
python3 metalmax-reverse/tools/coverage.py
python3 metalmax-reverse/tools/build_database.py
python3 metalmax-reverse/tools/verify_export.py
```

audit_data独立解析ROM，不是仅镜像exporter输出。若重新生成图像/关联，要按该次序重新覆盖audited派生资源。新的FCEUX探针保存instruction-contexts、prg-data-reads、audio-frame-boundaries和完成font-upload证据，snapshot期间禁用记录。

本报告与[游戏逻辑与流程](game_logic_and_flow.md)
及JSON/SQLite索引已同步。以后“验证通过”至少需要原字节、字段/像素、编号关系和对应运行证据分开报告。

