# 命名规则

本次新增的原生代码、工具、生成资源和文档使用以下文件名规则：小写英文字母、数字和下划线，不使用大写字母或连字符。十六进制编号也使用小写，例如
`chr_6b.h`、`page_0d.h`、`audio_editor_01a010.h`。

方法名和字段名暂时沿用原生工程现有的小驼峰：`decodeSprite`、`mapAttribute`、`cameraX`、`treasureCount`
。这是在用户消息后半句未完整给出时采用的现有风格，后续明确指定的规则优先。

原始maps数据、外部NSF库和Android既有公开类不进行附带重命名。`CMakeLists.txt`由CMake约定，Android
Java公开类文件名需与类名一致；这些已有约定保留。本次新增的独立Java参考工具改为 `sprite_reference.java`
，其入口类为合法的非public类 `SpriteReference`。

`tools/import_reverse.py`同时校验输出文件名，禁止生成不符合规则的资源文件。它仍不写入maps；文件命名调整不改变图像、脚本、数值数据和所属模块。
