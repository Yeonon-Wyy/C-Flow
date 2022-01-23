/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2022-01-23 19:32:20
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-23 19:32:21
 */
#pragma once

namespace vtf {

enum NotifierType {
    NOTIFIER_TYPE_START,
    DATA_LISTEN,
    FINAL,
    NOTIFIER_TYPE_END
};

enum NotifyStatus {
    OK,
    ERROR
};

} //namespace vtf