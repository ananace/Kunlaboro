#pragma once

#include <atomic>
#include <chrono>
#include <deque>
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
			void start();
			void stop();
			void wait(bool restart = true);

			template<typename Functor, typename... Args>
			void submit(Functor&& functor, Args&&... args);
			template<typename Functor>
			void submit(Functor&& functor);

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
		void JobQueue::submit(Functor&& functor, Args&&... args)
		{
			assert(!mExiting);

			{
				std::lock_guard<std::mutex> lock(mMutex);
				mJobQueue.emplace_back(std::bind(std::forward<Functor>(functor), std::forward<Args>(args)...));
			}
			mSignal.notify_one();
		}
		
		template<typename Functor>
		void JobQueue::submit(Functor&& functor)
		{
			assert(!mExiting);

			{
				std::lock_guard<std::mutex> lock(mMutex);
				mJobQueue.emplace_back(std::forward<Functor>(functor));
			}
			mSignal.notify_one();
		}

	}

}
