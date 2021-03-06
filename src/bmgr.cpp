/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bmgr.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sjhuang <hsjfans@mail.ustc.edu.cn>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/09 09:23:27 by sjhuang           #+#    #+#             */
/*   Updated: 2021/01/19 10:54:47 by sjhuang          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bmgr.h"
#include <cstring>
#include <iostream>
#include <memory>

BMgr::BMgr()
{
    dsm = new DSMgr();
    lru = new LRU;
    free_frames_num = BUFFER_SIZE;
    hit_count = 0;
}

BMgr::~BMgr()
{
    clean_buffer();
    delete dsm;
    delete lru;
}

Frame::sptr BMgr::read_page(int page_id)
{
    int frame_id = fix_page(false, page_id);
    auto frame = std::make_shared<Frame>();
    memcpy(frame->field, (buffer + frame_id)->field, FRAME_SIZE);
    return frame;
}

void BMgr::write_page(int page_id, const Frame::sptr &frame)
{
    int frame_id = fix_page(true, page_id);
    memcpy((buffer + frame_id)->field, frame->field, FRAME_SIZE);
    set_dirty(frame_id);
}

int BMgr::fix_page(int page_id)
{
    return fix_page(false, page_id);
}

int BMgr::fix_page(bool is_write, int page_id)
{
    BCB *bcb = get_bcb(page_id);
    // buffer中不存在该page
    if (bcb == nullptr)
    {
        if (dsm->is_page_exist(page_id))
        {
            int frame_id;
            // buffer已满
            if (free_frames_num == 0)
            {
                frame_id = select_victim();
            }
            else
            {
                frame_id = BUFFER_SIZE - free_frames_num;
                free_frames_num--;
            }
            insert_bcb(page_id, frame_id);
            lru->push(frame_id);
            set_page_id(frame_id, page_id);
            // 如果是写操作，则不读取page
            if (!is_write)
            {
                dsm->read_page(page_id, buffer + frame_id);
            }
            return frame_id;
        }
        else
        {
            return -1; // page_id 不存在
        }
    }
    else
    {
        int frame_id = bcb->get_frame_id();
        lru->update(frame_id);
        inc_hit_count();
        return frame_id;
    }
}

void BMgr::fix_new_page(const Frame::sptr &frame)
{
    int frame_id;
    // buffer已满
    if (free_frames_num == 0)
    {
        frame_id = select_victim();
    }
    else
    {
        frame_id = BUFFER_SIZE - free_frames_num;
        free_frames_num--;
    }
    memcpy((buffer + frame_id)->field, frame->field, FRAME_SIZE);
    int page_id = dsm->create_new_page(buffer + frame_id);
    insert_bcb(page_id, frame_id);
    lru->push(frame_id);
    set_page_id(frame_id, page_id);
}

int BMgr::hash_func(int page_id)
{
    return page_id % BUFFER_SIZE;
}

int BMgr::select_victim()
{
    int frame_id = lru->get_victim();
    int victim_page_id = get_page_id(frame_id);

    int hash = hash_func(victim_page_id);
    auto bcb_list = page_to_frame + hash;
    for (auto i = bcb_list->begin(); i != bcb_list->end(); i++)
    {
        if (i->get_page_id() == victim_page_id)
        {
            if (i->is_dirty())
            {
                dsm->write_page(victim_page_id, buffer + frame_id);
            }
            bcb_list->erase(i);
            break;
        }
    }
    return frame_id;
}

void BMgr::clean_buffer()
{

    for (const auto &bcb_list : page_to_frame)
    {
        for (const auto &i : bcb_list)
        {
            if (i.is_dirty())
            {
                dsm->write_page(i.get_page_id(), buffer + i.get_frame_id());
            }
        }
    }
}

void BMgr::set_dirty(int frame_id)
{
    int page_id = get_page_id(frame_id);
    BCB *bcb = get_bcb(page_id);
    bcb->set_dirty();
}

void BMgr::unset_dirty(int frame_id)
{
    int page_id = get_page_id(frame_id);
    BCB *bcb = get_bcb(page_id);
    bcb->unset_dirty();
}

// 通过`pag_id` 查找对应的`BCB`
BCB *BMgr::get_bcb(int page_id)
{
    int hash = hash_func(page_id);
    auto bcb_list = page_to_frame + hash;
    for (auto &i : *bcb_list)
    {
        if (i.get_page_id() == page_id)
        {
            return &i;
        }
    }
    return nullptr;
}

void BMgr::insert_bcb(int page_id, int frame_id)
{
    int hash = hash_func(page_id);
    auto bcb_list = page_to_frame + hash;
    bcb_list->emplace_back(page_id, frame_id);
}

int BMgr::get_page_id(int frame_id)
{
    return frame_to_page[frame_id];
}

void BMgr::set_page_id(int frame_id, int page_id)
{
    frame_to_page[frame_id] = page_id;
}

int BMgr::get_free_frames_num()
{
    return free_frames_num;
}

int BMgr::get_io_count()
{
    return dsm->get_io_count();
}

void BMgr::inc_hit_count()
{
    hit_count++;
}

int BMgr::get_hit_count()
{
    return hit_count;
}