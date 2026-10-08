# 题目
以自己熟悉的AI工具实现一个跨MCU平台的sensor框架
 - 以温度sensor为实现案例
 - 需要灵活支持多种滤波方式
 - 应用层需要解耦进行GUI显示，发送给MQTT以及进行业务计算
 - 优先使用C++实现，如果C++不熟悉可以使用C语言
 - 完成的代码提交到Github，自己创建Github仓库
 - 2个小时后将Github仓库地址发送到 alex.wu@blueair.com
 - 2个小时后如果觉得代码不满意，可以继续提交代码

# 架构
整体分三层app层用于执行业务，framwork层用于解耦app和driver，driver为底层固件驱动。
- app 调用framwork层接口，执行GUI、MQTT、业务计算等操作。除非业务逻辑有变更，此层基本不需要改变。
- framwork 将driver层init 等函数打包，返回执行app层操作或返回特定接口格式的数据。此层对外接口基本不变，根据硬件不同修改接口函数内部逻辑。
- driver 寄存器操作，支持 init config read write等硬件操作。此层依据硬件变化较大。

 