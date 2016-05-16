#include <Kunlaboro/detail/JobQueue.hpp>

using namespace Kunlaboro::detail;

JobQueue::JobQueue(unsigned threadCount)
	: mExiting(false)
	, mCompleteWork(true)
{
	if (threadCount == 0)
		threadCount = std::thread::hardware_concurrency() + 1;

	mThreadPool.reserve(threadCount);
	while (threadCount--)
		mThreadPool.emplace_back(std::thread(&JobQueue::workThread, this));
}
JobQueue::~JobQueue()
{
	abort();
}

void JobQueue::abort()
{
	mExiting = true;
	mCompleteWork = false;
	mSignal.notify_all();
	joinAll();

	{
		std::lock_guard<std::mutex> lock(mMutex);
		mJobQueue.clear();
	}
}
void JobQueue::stop()
{
	mExiting = true;
	mCompleteWork = true;
	mSignal.notify_all();
}

void JobQueue::wait()
{
	stop();
	joinAll();
}

std::future<void> JobQueue::submit(typename ident<std::function<void()>>::type&& functor)
{
	assert(!mExiting);

	typedef std::pair<std::promise<void>, std::function<void()>> data_t;
	auto data = std::make_shared<data_t>(std::promise<void>(), std::move(functor));
	auto future = data->first.get_future();

	{
		std::lock_guard<std::mutex> lock(mMutex);
		mJobQueue.emplace_back([data]() {
			try
			{
				data->second();
				data->first.set_value();
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

void JobQueue::joinAll()
{
	for (auto& t : mThreadPool)
		t.join();
}

void JobQueue::workThread()
{
	std::unique_lock<std::mutex> lock(mMutex);
	while (!mExiting || (mCompleteWork && !mJobQueue.empty()))
	{
		if (mJobQueue.empty())
			mSignal.wait(lock);
		else
		{
			std::function<void()> work(std::move(mJobQueue.front()));
			mJobQueue.pop_front();
			lock.unlock();

			work();

			lock.lock();
		}
	}
}
