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
