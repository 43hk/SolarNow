# SolarNow

#### 介绍
基于开源教程TinyRender，编写和封装图形库SolarGL，利用CPU渲染并写入序列帧，图片的转换和播放皆由ffmpeg完成。

可以展示旋转的星球。

release中提供部分星球贴图可供测试，obj是所有星球通用的。目前对于较复杂的obj文件还难以正确渲染，但可以用于简单obj文件的快速预览。

#### 使用
将obj和png放入model文件夹，只要保证两者同名即可。然后运行SolarNow.exe，即可弹出预览窗口，按空格可以暂停旋转，按方向键上或右可以旋转逆时针旋转，按左或下直接归位（我也不知道为什么不能反过来转，可能是ffmpeg的bug）

#### 贡献
1.TinyRender开源教程https://github.com/ssloy/tinyrenderer

2.开源软件ffmpeg

3.纹理贴图来源https://www.solarsystemscope.com/textures/
