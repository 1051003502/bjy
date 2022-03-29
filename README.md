# README

标签 ： 建筑项目

---



##链接方法
（1）getlib下载项目源文件；
（2）在CMAKE进行VS环境链接
（3）打开链接后的项目：

- 为项目CADAnalyser添加预处理宏“_AFXDLL”；
- 将项目“CADnalyser”设置为窗口程序；
- 为项目“CADAnalyser”，“CADAnalyserCLI”，“dxfrw”，“jsoncpp”，“parser”，添加“iconv”包；(使用vs自带的NuGet搜索libiconv,添加并生成依赖即可，对于搜索不到的按此链接下载链接：https://pan.baidu.com/s/1_4Cg6VRXHM_OGCyl7wYAmQ 
提取码：afi4；手动生成依赖)
##目录结构划分
**目录cli 存放数据导出源码**
**lib目录“jsoncpp”,“libdxfrw”库的源码**
**parser目录存放数据解析源码**
**viewer目录存放调试程序的窗口源码**
##模块划分
###导出模块

设“CADAnalyserCLI”为启动项目，生成Release版".exe"可执行程序可对数据进行导出；
###显示调试模块
设“CADAnalyser”为启动项目，直接点击**“本机windows调试器”**可进行图纸的导入、数据读出测试、调试等；
###解析模块
**“parser”**主要对读出的数据重新解析整理，生成可利用的有效数据。
##数据流向
![数据流向图.png](https://upload-images.jianshu.io/upload_images/14182826-ecd0ec4ddf1648b0.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

####类：transer
- DWG格式化图纸，在此传入；
- 读出图纸内容（点、线。圈、弧、文本等）；
####类：parser
- 整理transer读出数据的数据；
- 在formatBlock函数内重新解析图纸里实体内容（最基本的实体，点、线、圈等）；
- 经过find系列函数（例如：findBorders()，findAxises()），重新整理建筑图纸的实体组件。
####项目CADAnalyser
- 一个输出端，主要用于调试程序
- 将parser内整理好的数据在此输出，在屏幕上显示；
- 类DrawingView，Viewer，ViewerDlg，是界面的一些操作；
####项目CADAnlyserCLI
- 一个输出端，是导出数据的出口，用于AI的数据分析；




