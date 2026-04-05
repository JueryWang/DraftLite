### DrafLite导图项目基本使用说明

- 基本介绍
- 软件优势
- 行业拓展

### 1.软件基本介绍

> DrafLite是面向2维CNC应用的软件,集可视化图纸、CAD文件解析、G代码生成、路径规划于一体的CAM软件。相较于其他数控软件,Draft在处理5W以上的大点数图纸可达到实时交互需求。DrafLite可针对用户的工艺需求深度定制,输出符合客户需求的NC程序,高度定制选段插补算法,减少空走,优化输出的NC路径。针对不同的毛坯,DrafLite还内置了开粗的算法,可从指定形状的毛坯开始逐层加工到图纸形状,同时开启了层级碰撞干涉检查防止表面铣过程中刀路空走时产生碰撞,影响表面形状

### 2.软件优势

- 可在加工前离线模拟走刀,可视化走刀路径和G代码当前行数,方便观察实际效果。中途可停止,步进,在绘图区缩放以查看实际路径和图纸之间的差异

<video src="./video/simulate.mp4" controls width="600"></video>

- 可按序列处理多任务,并且可以捕捉到特定点,仅生成选段区域的G代码

  ![AuthDownloadDll](./image_DraftLite/多任务.png)

![AuthDownloadDll](./image_DraftLite/选段配置.png)

![AuthDownloadDll](./image_DraftLite/选段配置2.png)

- 轮廓识别、空走路径优化

  针对无序的输入图纸,解析完成后做轮廓识别及排序的前处理程序,使得生成的G代码保持尽量少的空程路径,节省加工时间

  ##### 👉优化前路径![AuthDownloadDll](./image_DraftLite/空走路径优化1.png)

  👉优化后路径![AuthDownloadDll](./image_DraftLite/空走路径优化1 (2).png)

- 轮廓层次树剖分

  ![开粗-方向配置.png](./image_DraftLite/ContourTree.png)

  ![开粗-方向配置.png](./image_DraftLite/ContourTree2.png)

  

- 2D刀路规划

  Draft Lite针对毛坯和输入的图纸形状,可生成开粗的路径,自动识别开粗区域;并且在切换区域时规避工件和毛坯防止产生碰撞

  开粗配置可设置切削方向、余量、行距和公差,用户可根据实际需求进行配置

  ![开粗-方向配置.png](./image_DraftLite/开粗-方向配置.png)用户还可通过配置区域的颜色和组别,通过设置区域的绕中心区域角度变换,等待区域切换信号后WG_MachWiz会发送变换后的坐标至下位机仅加工特定组别的区域

![开粗-区域配置.png](./image_DraftLite/开粗-区域配置.png)

![开粗演示.png](./image_DraftLite/开粗演示.png)

- 3D刀路规划

![PathPlan](./image_DraftLite/PathPlan.png)

