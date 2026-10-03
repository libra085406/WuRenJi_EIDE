#ifndef __Com_debug_H__
#define __Com_debug_H__

#include "stdio.h"
#include "stdarg.h"
#include "usart.h"

// 日志输出打印在CPU运行上非常占用资源 ---> 通过比特率可以计算  打印10字节左右大概需要1ms  非常影响飞机的飞行
// 所以在后续飞机需要正常飞行的时候  需要关闭打印功能

#define DEBUG_LOG_ENABLE 1      // 设计一个日志输出打印开关

#ifdef DEBUG_LOG_ENABLE
    // 使用宏定义的方式在打印日志之前添加文件名和行数信息
    #define debug_printf(format, ...) printf("[%s : %d]  " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    // 如果没有开启日志输出打印
    #define debug_printf(format, ...)
#endif

#endif /* __Com_debug_H__ */
