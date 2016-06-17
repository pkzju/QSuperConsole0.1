# QSuperConsole
Super Console serial port serial bus modbus canbus tcp/ip

Qt必须5.6版本以上

git推送服务器命令：git push -uf origin master:workon
以branch 'workon' of github.com:pkzju/QSuperConsole例


canopen协议参考：canopen/CANOpen_memento.pdf CANopen_meno.pdf
CanFestival源码手册：canopen/manual_en.pdf

0.  移植时win32时 错误出现在objdictdef.h文件 identity宏定义  改为IDENTITY后ok

1.  安装Beremiz-1.1_RC5.exe， 点击objdictedit.py 打开字典编辑器编辑对象字典
	例:(win32test.c中) 初始化对象字典：CO_Data win32test_Data = CANOPEN_NODE_DATA_INITIALIZER(win32test);
	CANOPEN_NODE_DATA_INITIALIZER宏定义在data.h中
	
记录：	
Transition from Initialisation to Pre_operational is automatic as defined in DS301.
App don't have to call SetState(d, Pre_operational) 


In states.c  function : UNS8 setState(CO_Data* d, e_nodeState newState)

											s_state_communication:
                 csBoot_Up    csSDO    csEmergency    csSYNC     csLifeGuard    csPDO        csLSS
Initialisation   {1,           0,           0,           0,           0,           0,           0}
Pre_operational  {0,           1,           1,           1,           1,           0,           1}
Operational      {0,           1,           1,           1,           1,           1,           0}
Stopped          {0,           0,           0,           0,           1,           0,           1}

csLSS:        0 -> 1  startLSS(d),	     1 -> 0  stopLSS(d))
sSDO:         0 -> 1  None,	             1 -> 0	 resetSDO(d))
csSYNC:       0 -> 1  startSYNC(d),	     1 -> 0  stopSYNC(d))
csLifeGuard:  0 -> 1  lifeGuardInit(d),	 1 -> 0  lifeGuardStop(d))
csEmergency:  0 -> 1  emergencyInit(d),	 1 -> 0  emergencyStop(d)) 
csPDO:        0 -> 1  PDOInit(d),	     1 -> 0  PDOStop(d))
csBoot_Up:    0 -> 1  None,	             1 -> 0  slaveSendBootUp(d))

In sync.c  function : UNS8 proceedSYNC(CO_Data* d)  
                          -> _sendPDOevent() 
					         -> isSyncEvent? -> 比较 TPDO 的transmit_type_parameter设定值与当前值  当前值++
								             ->  设定值==当前值? -> 发送对应TPDO, 
							 -> 检查下一个TPDO
							 
							 
qt代码中判断windows/linux/arm等系统

可使用宏判断，例如：
#include <QtGlobal>
...
#ifdef Q_OS_MAC
// mac
#endif
 
#ifdef Q_OS_LINUX
// linux
#endif
 
#ifdef Q_OS_WIN32
// win
#endif
 
#ifdef __arm__
// arm
#endif

9600 baudrate -> 1 char = 1.14ms 3.5char = 4ms


exec与show: exec 关掉对话框才能操作其他窗口    show 不必关掉都可以操作
