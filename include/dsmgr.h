/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dsmgr.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sjhuang <hsjfans@mail.ustc.edu.cn>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/09 09:22:48 by sjhuang           #+#    #+#             */
/*   Updated: 2021/01/19 10:54:40 by sjhuang          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMPLE_DB_DSMGR
#define SIMPLE_DB_DSMGR

#include <string>
#include "common.h"

class DSMgr
{
public:
     DSMgr();

     ~DSMgr();

     /**
     * 判断page是否存在
     *     1. page_id > 0
     *     2. page_id <= page_num
     *     3. page is use
     */
     bool is_page_exist(int page_id);

     /**
     * 读取page至frame
     */
     int read_page(int page_id, Frame *frm);

     /**
     * 将frame写入page
     */
     int write_page(int page_id, Frame *frm);

     /**
     * 创建一个新的page，并将frame内容写入其中
     */
     int create_new_page(Frame *frm);

     /**
     * 获取当前page数
     */
     int get_pages_num();

     /**
     * 获取当前IO数
     */
     int get_io_count();

private:
     /**
     * 数据库文件
     */
     FILE *db_file;

     /**
     * 第一个page，存放page数与指针
     */
     int first_page[MAX_PAGE_NUM] = {0};

     /**
     * IO数
     */
     int io_count;

     /**
     * 打开DB文件
     */
     void open_file();

     /**
     * 关闭数据库文件（在析构函数中调用）
     */
     int close_file();

     /**
     * 将文件指针移动至距文件首部offset的位置
     */
     int seek(unsigned int offset);

     /**
     * 读取第一个page，内包含page数、各page在文件中的位置以及是否use
     */
     void read_first_page();

     /**
     * 将第一个page写回文件（在析构函数中调用）
     */
     void write_first_page();

     /**
     * 根据page_id获取文件指针
     */
     unsigned int get_page_pointer(int page_id);

     /**
     * 根据page_id计算文件指针并将其保存（在create_new_page时调用）
     */
     void set_page_pointer(int page_id, int pointer);

     /**
     * 设置page的使用状态，true为已被使用，false为未被使用（已被删除）
     */
     void set_use(int page_id, bool is_use);

     /**
     * 获取page的使用状态
     */
     bool get_use(int page_id);

     /**
     * 增加page数（在create_new_page时调用）
     */
     void inc_pages_num();

     /**
     * 增加IO计数（发生读或写时调用）
     */
     void inc_io_count();
};

#endif //SIMPLE_DB_DSMGR
