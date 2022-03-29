#ifndef __SIGNAL_H__
#define __SIGNAL_H__

//signal from https://blog.csdn.net/p2016/article/details/76925595

#include<functional>
#include<memory>
#include<vector>
#include<mutex>
#include<assert.h>
#include<iostream>

template<typename callback>
class slot_impl;

template<typename callback>
class signal_impl
{
public:
	typedef std::vector<std::weak_ptr<slot_impl<callback>>> slot_list;

	std::shared_ptr<slot_list> slots_;
	std::mutex mutex_;
public:
	signal_impl() :slots_(new slot_list) {}
	void copyonwrite()//写操作会调用他，前提已经加锁。
	{
		if (!slots_.unique())
		{
			slots_.reset(new slot_list(*slots_));
		}
		assert(slots_.unique());
	}
	void clean()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		copyonwrite();
		slot_list& slots_ref(*slots_);
		typename slot_list::iterator iter = slots_ref.begin();
		while (iter != slots_ref.end())
		{
			if (iter->expired())
				iter = slots_ref.erase(iter);
			else
				iter++;
		}
	}
};

template<typename callback>
class slot_impl
{
public:
	std::weak_ptr<signal_impl<callback>> signal_;
	callback cb_;
	std::weak_ptr<void> tie_;
	bool tied_;
public:
	slot_impl(const std::shared_ptr<signal_impl<callback>>& signal_args_, callback&& cb_arg_)
		:signal_(signal_args_), cb_(cb_arg_), tie_(), tied_(false)
	{

	}
	slot_impl(const std::shared_ptr<signal_impl<callback>>& signal_args_, callback&& cb_arg_, const std::shared_ptr<void> tie_arg)
		:signal_(signal_args_), cb_(cb_arg_), tie_(tie_arg), tied_(true)
	{

	}
	~slot_impl()
	{
		std::shared_ptr<signal_impl<callback>> sp_signal_imp_(signal_.lock());
		if (sp_signal_imp_)
			sp_signal_imp_->clean();
	}
};

typedef std::shared_ptr<void> slot;

template<typename signature>
class signal;

template<typename RET, typename... ARGS>
class signal<RET(ARGS...)>
{
public:
	typedef std::function<void(ARGS...)> callback;
	typedef signal_impl<callback> signal_imp;
	typedef slot_impl<callback> slot_imp;
        typedef typename signal_impl<callback>::slot_list slot_list;
private:
	std::shared_ptr<signal_imp> signal_imp_;
	void add(const std::shared_ptr<slot_imp>& slot)//写操作
	{
		signal_imp&  signal_imp_ref_(*signal_imp_);
		{
			std::lock_guard<std::mutex> lock(signal_imp_ref_.mutex_);
			signal_imp_ref_.copyonwrite();
			signal_imp_ref_.slots_->push_back(slot);
		}
	}
public:
	signal() :signal_imp_(new signal_imp) {}
	~signal() {}

	void clean() {
		signal_imp&  signal_imp_ref_(*signal_imp_);
		{
			signal_imp_ref_.clean();
		}
	}
	slot connect(callback&& cb_)
	{
		std::shared_ptr<slot_imp> ret(new slot_imp(signal_imp_, std::forward<callback>(cb_)));
		add(ret);
		return ret;
	}
	slot connect(callback&& cb_, const std::shared_ptr<void>& tie)
	{
		std::shared_ptr<slot_imp> ret(new slot_imp(signal_imp_, std::forward<callback>(cb_), tie));
		add(ret);
		return ret;
	}
	void call(ARGS... args)
	{
		signal_imp&  signal_imp_ref_(*signal_imp_);
		std::shared_ptr<slot_list> sp_slots_;//为了让引用计数加1。保护sp的读，临界区一行
		{
			std::lock_guard<std::mutex> lock(signal_imp_ref_.mutex_);
			sp_slots_ = signal_imp_ref_.slots_;
		}

		slot_list&  slots_ref_(*sp_slots_);//这时就可以大胆的取 槽数组 了
		for (typename signal_imp::slot_list::iterator iter = slots_ref_.begin(); iter != slots_ref_.end(); iter++)
		{
			std::shared_ptr<slot_imp> sp_slot_imp = iter->lock();
			if (sp_slot_imp)//感知生命，对应操作
			{
				std::shared_ptr<void> guard;
				if (sp_slot_imp->tied_)
				{
					guard = sp_slot_imp->tie_.lock();
					if (guard)
						sp_slot_imp->cb_(args...);
				}
				else
					sp_slot_imp->cb_(args...);
			}
		}
	}
};

//using namespace std;
//void hello()
//{
//	int a = 0;
//	int b = 1;
//	cout << "hello" << endl;
//}
//void print(int i)
//{
//	cout << "print " << i << endl;
//}
//void test()
//{
//	{
//		signal<void(void)> sig;
//		slot slot1 = sig.connect(&hello);
//		sig.call();
//	}
//	signal<void(int)> sig1;
//	slot slot1 = sig1.connect(&print);
//	slot slot2 = sig1.connect(std::bind(&print, std::placeholders::_1));
//	std::function<void(int)> func1(std::bind(&print, std::placeholders::_1));
//	slot slot3 = sig1.connect(std::move(func1));
//	{
//		slot slot4 = sig1.connect(std::bind(&print, 666));
//		sig1.call(4);
//	}
//	sig1.call(4);
//}

#endif