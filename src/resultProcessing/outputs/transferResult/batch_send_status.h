#ifndef BATCH_SEND_STATUS_H
#define BATCH_SEND_STATUS_H

/**
 * @brief 批次结果发送状态枚举
 */
enum BatchSendStatus {
    BATCH_IDLE,               // 空闲状态，可以发送新批次
    BATCH_SENDING,            // 正在发送数据到共享内存
    BATCH_WAITING_RECEIVE,    // 数据已发送，等待接收方读取
    BATCH_TIMEOUT,            // 等待超时
    BATCH_ERROR               // 发送过程中出现错误
};

#endif // BATCH_SEND_STATUS_H
