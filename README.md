# metalmax
Full C / C ++ / arm-v8 assembly remake metalmax

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
