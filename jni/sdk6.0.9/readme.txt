sdk5.04:2410平台
sdk6.04:2416平台兼容2410平台

2013.6.28
把libfunc.h中encrypt,decrypt 改名为_encrypt,_decrypt
原因是函数名与编译器下面unistd.h文件中函数名称相同

2013.7.1
com.c由原来的serial目录移动com目录

2013.7.2
libserial.c 相当于公司protocol格式的定义
initcom.c　相当于公司protocol命令的定义,与内部设备通讯使用

2013.7.8
备注：MF1 Card 返回的是10进制的卡号

2013.7.10
增加无线模块m35的支持

2013.7.11
增加电池电量检测，电压高低检测及显示(增加power目录)
解决串口及spi口冲突问题。

2013.7.12
移除buzzer目录
移除http目录
移除pc-finger目录
demo目录下serial目录改名为com

2013.7.15
initcom.c是对公司protocol的再次封装

2013.7.18
打印机型号:citizen s310

2013.7.19
增加sysconsts.h文件来控制使用硬件平台

2013.7.22
把com.c中的函数comm_put_data，comm_get_data等移到libserial.c中
demo目录下的print目录改名为printer
把lib目录下的print目录改名为printer.并且把print.*改名为printer.*，把printcom.*改名为s310.*,并把函数按照接口的方式进行了整合，使结构
更清晰。
把libserial.c改名为protocol.c

2013.8.27
把sysconsts.h 重命名为version.h