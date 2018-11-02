/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */

#include "multi_timer.h"

//timer handle list head.
static struct Timer* head_handle = NULL;

//Timer ticks
static uint32_t _timer_ticks = 0;
static uint8_t _timer_ticks_flag = 0;

/**
  * @brief  Initializes the timer struct handle.
  * @param  handle: the timer handle strcut.
  * @param  timeout_cb: timeout callback.
  * @param  repeat: repeat interval time.
  * @retval None
  */
void timer_init(struct Timer* handle, void(*timeout_cb)(), uint32_t timeout, uint32_t repeat)
{
	// memset(handle, sizeof(struct Timer), 0);
	handle->timeout_cb = timeout_cb;
	handle->timeout = _timer_ticks + timeout;
	handle->repeat = repeat;
}

/**
  * @brief  Start the timer work, add the handle into work list.
  * @param  btn: target handle strcut.
  * @retval 0: succeed. -1: already exist.
  */
int timer_start(struct Timer* handle)
{
	struct Timer* target = head_handle;
	while(target) {
		if(target == handle) return -1;	//already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}

/**
  * @brief  Stop the timer work, remove the handle off work list.
  * @param  handle: target handle strcut.
  * @retval None
  */
void timer_stop(struct Timer* handle)
{
	struct Timer** curr;
	for(curr = &head_handle; *curr; ) {
		struct Timer* entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
//			free(entry);
		} else
			curr = &entry->next;
	}
}

/**
  * @brief  main loop.
  * @param  None.
  * @retval None
  */
void timer_loop()
{
	struct Timer* target;
	//方式一
/* 	uint8_t flag = _timer_ticks_flag;
	_timer_ticks_flag = 0;
	for(target=head_handle; target; target=target->next){
		if(((flag == target->overFlag)&&(_timer_ticks >= target->timeout))||(flag > target->overFlag)){
			target->overFlag = 0;
			if(target->repeat == 0){
				timer_stop(target);
			} 
			else {					
				target->timeout = _timer_ticks + target->repeat;
				if(target->timeout < _timer_ticks){
					target->overFlag = 1;
				}
			}
			target->timeout_cb();
		}
	} */
	//方式二:代码换速度
	//_timer_ticks_flag变为1时的处理方式
	if(_timer_ticks_flag == 1){
		_timer_ticks_flag = 0;//复位标志
		for(target=head_handle; target; target=target->next){
			if(target->overFlag == 1){
				target->overFlag = 0;//任务和定时器都发生反转,则都复位,按照正常比较
			}
			else {//定时器已经反转了而任务还没反转,说明任务已经超时,但是没有执行,应立即执行
				if(target->repeat == 0){
					timer_stop(target);
				} 
				else {
					target->timeout = _timer_ticks + target->repeat;
					// if(target->timeout < _timer_ticks){
						// target->overFlag = 1;
					// }
				}
				target->timeout_cb();
			}
		}
	}
	//target->overFlag = 0;_timer_ticks_flag = 0;时的处理方式
	for(target=head_handle; target; target=target->next){		
		if(target->overFlag == 0){//正常比较
			if(_timer_ticks >= target->timeout){
				if(target->repeat == 0){
					timer_stop(target);
				} 
				else {					
					target->timeout = _timer_ticks + target->repeat;
					if(target->timeout < _timer_ticks){
						target->overFlag = 1;
					}
				}
				target->timeout_cb();
			}
		}
	}
}

/**
  * @brief  background ticks, timer repeat invoking interval 1ms.
  * @param  None.
  * @retval None.
  */
void timer_ticks()
{
	_timer_ticks++;
}

