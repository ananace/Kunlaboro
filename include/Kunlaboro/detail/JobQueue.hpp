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

			template<typename Functor, typename... Args>
			auto submit(Functor&& functor, Args&&... args)->std::future<decltype(functor(args...))>;
			template<typename Functor>
			auto submit(Functor&& functor)->std::future<decltype(functor())>;

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

		template<typename Functor, typename... Args>
		auto JobQueue::submit(Functor&& functor, Args&&... args) -> std::future<decltype(functor(args...))>
		{
			assert(!mExiting);

			auto data = std::make_shared<std::packaged_task<decltype(functor(args...))()>>(
				std::bind(std::forward<Functor>(functor), std::forward<Args>(args)...)
				);

			{
				std::lock_guard<std::mutex> lock(mMutex);
				mJobQueue.emplace_back([data]() {
					(*data)();
				});
			}
			mSignal.notify_one();

			return data->get_future();
		}
		
		template<typename Functor>
		auto JobQueue::submit(Functor&& functor) -> std::future<decltype(functor())>
		{
			assert(!mExiting);

			auto data = std::make_shared<std::packaged_task<decltype(functor())()>>(
				std::forward<Functor>(functor)
			);

			{
				std::lock_guard<std::mutex> lock(mMutex);
				mJobQueue.emplace_back([data]() {
					(*data)();
				});
			}
			mSignal.notify_one();

			return data->get_future();
		}

	}

}
