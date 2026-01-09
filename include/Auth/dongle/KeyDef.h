#ifndef KEYDEF_H
#define KEYDEF_H

#define D8_USHORT USHORT
#define D8_SHORT short

#define ErrResult -20
#define FAILEDGENKEYPAIR 	-21//不能生产密钥对
#define FAILENC          	-22//加密失败
#define FAILDEC          	-23//解密失败
#define FAILPINPWD  	 	-24//PIN码错误
#define NOT_FOUND_CMD		-25//无效命令
#define NOTOPENFILE -31//不能打开文件
#define NOTREADFILE -32 //不能读取文件
#define NOTWRITEFILE -42 //不能写文件，请查看文件是否有只读属性，或被其它文件打开
#define TRANFSERFAIL  -45 //传送数据失败
#define ERRWRITEPASSWORD -47 //写密码不正确
#define ERRREADPASSWORD -48 //读密码不正确
#define EPPROMOVERADD  -49 //读写EEPROM时，地址溢出
#define USBStatusFail -50  //USB操作失败，可能是没有找到相关的指令
#define OPENUSBFAIL  -51  //打开USB文件句柄失败
#define ENCERROR   -52 //使用加密锁加密自定义表达式，生成加密代码时生产错误
#define NOUSBORDRIVER   -53 //无法打开usb设备，可能驱动程序没有安装或没有插入加密锁。
#define LESSCOUNT      -55  //要加密的数据少于8个字节
#define GETVARERROR   -56   //返回结果变量错误
#define NotVaildFile      -65  //不是有效的BIN文件
#define ERRORKEY      -66  //不能写入加密锁，原因是密钥不正确
#define OVERLEN    -67//长度溢出
#define SETCMPFAIL -68  //USB操作失败
#define FUNCTION_LENGTH_NAME_MORE_THEN_25			-79//函数名大于25个字符
#define ErrAddrOver    -81//地址溢出
#define ErrWritePWD   -82//写密码错误
#define ErrReadPWD    -83//读密码错误
#define NOUSBKEY   -92 //找不到加密狗。
#define ErrSendData -93//发送数据错误
#define ErrGetData  -94//接收数据错误

#define CANNOT_OPEN_BIN_FILE						-8017//不能打开要调试的二进制文件，请确保文件是否编译成功
#define OVER_KEY_LEN								-8025//要设置的下载密钥的长度超过16个字符
#define CAN_NOT_READ_FILE							-8026//不能读取文件
#define CANNOT_CREATE_FILE							-8046//无法建立文件
#define OVER_BIND_SIZE								-8035//绑定电脑的数据超过最大绑定值
#define OVER_EPROM_ADDR								-8050//储存器地址溢出

#define SM2_ADDBYTE 97//加密后的数据会增加的长度
#define MAX_ENCLEN 128 //最大的加密长度分组
#define MAX_DECLEN  (MAX_ENCLEN+SM2_ADDBYTE) //最大的解密长度分组
#define USERID_LEN	  80//最大的用户身份长度

#define	MAX_FUNNAME 25
#define ECC_MAXLEN 	  32
#define PIN_LEN 	  16
#define MAX_LEN 496

//定义命令 ,不能有等于0的命令
#define GETVERSION      0x01
#define GETID      		0x02
#define S_MYENC         0X03
#define S_MYDEC         0X04
#define GETVEREX        0x05
#define SETCAL			0x06
#define	SETID			0X07
#define CAL_TEA         0x08
#ifdef _ZHOU
#define SET_TEAKEY      0x63
#else
#define SET_TEAKEY      0x09
#endif
#define CAL_TEA_2       0x0c
#define SET_TEAKEY_2    0x0d
#ifdef _F2K
#define YTREADBUF		0x10
#define YTWRITEBUF      0x11
#else
#define READBYTE   		0x10
#define WRITEBYTE     	0x11
#define YTREADBUF		0x12
#define YTWRITEBUF      0x13
#endif
#define MYRESET         0x20
#define YTREBOOT   		0x24

#define SET_ECC_PARA    0x30
#define GET_ECC_PARA	0x31
#define SET_ECC_KEY     0x32
#define GET_ECC_KEY     0x33
#define MYENC			0x34
#define MYDEC			0X35
#define SET_PIN			0X36
#define GEN_KEYPAIR     0x37

#define SET_DIS_FLAG	0X40
#define GET_DIS_FLAG	0X41

#define SHOW_U_CMD		0x42
#define GET_U_STATUS	0X43
#define GEN_LIC			0x44

#define YTSIGN          0x51
#define YTVERIFY        0x52
#define GET_CHIPID      0x53
#define YTSIGN_2        0x54
#define WRITESN			0XCE
#define GETSN			0x0F
#define WRITESN			0XCE
#define	SETHIDONLY	    0x55
#define SETREADONLY		0x56


#define GET_LIMIT_DATE		0x71
#define SET_LIMIT_DATE		0x72
#define GET_USER_ID			0x73
#define GET_LEAVE_NUMBER	0x74
#define CHECK_NUMBER		0x75
#define SET_NUMBER_AUTH		0x76
#define SET_BIND_AUTH		0x77
#define SET_DATE_AUTH		0x78
#define CHECK_DATE			0x79
#define CHECK_BIND			0x7A
#define GET_LEAVE_DAYS		0x7B
#define GET_BIND_INFO		0x7C
#define GETVERCODE			0x7D
#define YTDOWNLOAD			0x80
#define START_DOWNLOAD		0x81
#define RUN_FUNCTION		0x82
#define SET_VAR				0x84
#define GET_VAR				0x85
#define SET_DOWNLOAD_KEY	0x86
#define OPEN_KEY			0x87
#define CLOSE_KEY			0x88
#define COUNTINU_RUNCTION	0x89
#define GET_API_PARAM		0x8A
#define SET_API_PARAM		0x8B
#define GETFUNCVER			0x8C
#define WRITE_NEW_EPROM		0x8D
#define READ_NEW_EPROM		0x8E
#define SET_PWD				0x8F
#define MYREST_SELF			0x90
#define MYREST_SELF_2		0x91
#define SET_I2CADD			0x92
#define SET_BUSMODE			0x93
#define UPDATE_DOWNLOAD_KEY	   	0x94
#define EXPORT_ENC_BIN			0x95
#define	EXPORT_ENC_DWONLAD_KEY	0x96
#define SET_BIN_SIZE			0x97
#define GET_BIN_SIZE			0x98
#define GEN_RND					0x99
#define SET_RND					0x9A
#define CAL_TEA_3				0x9B
#define EXPORT_DATA				0x9C 
#define IMPORT_DATA				0x9D
#define	GET_DY_DATA_SIZE		0x9E
#define GET_DY_DATA_VALUE		0x9F
#define SET_DY_DATA_VALUE		0xA0
#define CLEAR_DY				0xA1

#define CUSTOM_ID_CMD			0xF1


//D8定义
#define VERF_CODE_STR_LEN			17
#define AUTH_CODE_STR_LEN			17
#define MAX_KEY_LEN					16
#define	MAX_FUNNAME					25
#define MAX_BUF_SIZE 				255
#define DOWNLOAD_SIZE				(MAX_BUF_SIZE-1-1-1)//一个字节长度，一个字节命令，一个字节报告
#define ENC_DOWNLAD_SIZE			(DOWNLOAD_SIZE/8*8)
#define TRANSFER_VAR_SIZE			(MAX_BUF_SIZE-1-1-4)//一个字节长度，一个字节命令，一个字节报告,4个字节内存起始地址
#define GETDATA_BUF_SIZE			(MAX_BUF_SIZE-2-1-1)//2个字节错误码，1个字节长度是否完成，一个字节为多出来的
#define VERF_CODE_SIZE				8
#define NEW_PWD_LEN					9
#define NEW_EPROM_TRANSFER_SIZE		(MAX_BUF_SIZE-(1+1+sizeof(D8_USHORT)+1+2*NEW_PWD_LEN))//一个字节报告,一个字节命令，二个字节地址，一个字节长度，2*9个字节密码，
#define MAX_BIND_MAC_SIZE			200


#define  IS_RECOUNT		0
#define  IS_BIND		1
#define  IS_INC_RECOUT	2


#define EXPORT_ERROM_CMD    0
#define EXPORT_ID_CMD		1
#define EXPORT_KEY_CMD		2

#define IMPORT_ERROM_CMD    0
#define IMPORT_ID_CMD		1
#define IMPORT_KEY_CMD		2

#define ID_LEN   		0X10//要返回的芯片ID长度，实际芯片长度为32字节，为了兼容，只返回使用了的16个字节
#define EXPORT_DATA_SIZE   128

#endif // KEYDEF_H
