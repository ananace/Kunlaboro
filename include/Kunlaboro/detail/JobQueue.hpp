#pragma once

#include <chrono>
#include <deque>
#include <future>
#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>

#include <cassert>

namespace Kunlaboro
{

	namespace detail
	{

		class JobQueue
		{
			template<typename T> struct ident { typedef T type; };

		public:
			JobQueue(unsigned threadCount = 0);
			JobQueue(const JobQueue&) = delete;
			~JobQueue();

			void abort();
			void stop();
			void wait();

			template<typename Ret>
			std::future<Ret> submit(typename ident<std::function<Ret()>>::type&& functor);
			std::future<void> submit(typename ident<std::function<void()>>::type&& functor);

		private:
			void workThread();
			void joinAll();

			std::deque<std::function<void()>> mJobQueue;
			std::vector<std::thread> mThreadPool;
			std::mutex mMutex;
			std::condition_variable mSignal;

			std::atomic_bool mExiting
			               , mCompleteWork;
		};


		template<typename Ret>
		std::future<Ret> JobQueue::submit(typename ident<std::function<Ret()>>::type&& functor)
		{
			assert(!mExiting);

			typedef std::pair<std::promise<Ret>, std::function<Ret()>> data_t;
			auto data = std::make_shared<data_t>(std::promise<Ret>(), std::move(functor));
			auto future = data->first.get_future();

			{
				std::lock_guard<std::mutex> lock(mMutex);
				mJobQueue.emplace_back([data]() {
					try
					{
						data->first.set_value(data->second());
					}
					catch(...)
					{
						data->first.set_exception(std::current_exception());
					}
				});
			}
			mSignal.notify_one();

			return std::move(future);
		}

	}

}
