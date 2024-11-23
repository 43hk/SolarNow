# SolarNow

#### 介绍
基于开源教程TinyRender，编写和封装图形库SolarGL，利用CPU渲染并写入序列帧，图片的转换和播放皆由ffmpeg完成。

工程中提供部分星球贴图可供测试，但本渲染可以渲染几乎所有obj文件，只要保证模型只有三角形和四边形，以及单个PNG贴图，可以用于简单obj文件的快速预览。

#### 使用
将obj和png放入model文件夹，只要保证两者同名即可。然后运行SolarNow.exe，即可弹出预览窗口，按空格可以暂停旋转。

#### 贡献
1.TinyRender开源教程https://github.com/ssloy/tinyrenderer

2.开源软件ffmpeg

3.纹理贴图来源https://www.solarsystemscope.com/textures/
