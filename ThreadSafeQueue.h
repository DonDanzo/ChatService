#pragma once

#include <deque>
#include <mutex>


template<typename T>
class ThreadSafeQueue
{
public:
	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;
	~ThreadSafeQueue() { Clear(); }

	//return item in the front of the queue
	const T& Front()
	{
		std::scoped_lock lock(m_mutex);
		return m_doubleEdgeQueue.front();
	}

	//removes and returns item in the front of the queue
	T PopFront()
	{
		std::scoped_lock lock(m_mutex);
		auto t = std::move(m_doubleEdgeQueue.front());//store object that will be returned
		m_doubleEdgeQueue.pop_front();//remove the stored onject that will be returned
		return t;
	}

	//add items at the front of the queue
	void PushFront(const T& item)
	{
		std::scoped_lock lock(m_mutex);
		m_doubleEdgeQueue.emplace_front(std::move(item));

		std::unique_lock<std::mutex> ul(m_blockingMutex);
		m_conditionVariable.notify_one();
	}

	//return item at the back of the queue
	const T& Back()
	{
		std::scoped_lock lock(m_mutex);
		return m_doubleEdgeQueue.back();
	}

	//removes and returns item at the back of the queue
	T PopBack()
	{
		std::scoped_lock lock(m_mutex);
		auto t = std::move(m_doubleEdgeQueue.back());//store object that will be returned
		m_doubleEdgeQueue.pop_back();//remove the stored onject that will be returned
		return t;
	}

	//add items at the back of the queue
	void PushBack(const T& item)
	{
		std::scoped_lock lock(m_mutex);
		m_doubleEdgeQueue.emplace_back(std::move(item));

		std::unique_lock<std::mutex> ul(m_blockingMutex);
		m_conditionVariable.notify_one();
	}

	//checks is queue empty
	bool IsEmpty()
	{
		std::scoped_lock lock(m_mutex);
		return m_doubleEdgeQueue.empty();
	}

	//return how much elements there are in the queue
	size_t Count()
	{
		std::scoped_lock lock(m_mutex);
		return m_doubleEdgeQueue.size();
	}

	//clears the queue
	void Clear()
	{
		std::scoped_lock lock(m_mutex);
		m_doubleEdgeQueue.clear();
	}
	
	//wait until some element is added to the queue
	void Wait()
	{
		while (IsEmpty())
		{
			std::unique_lock<std::mutex> ul(m_blockingMutex);
			m_conditionVariable.wait(ul);
		}
	}

protected:
	std::mutex m_mutex;
	std::deque<T> m_doubleEdgeQueue;
	std::condition_variable m_conditionVariable;
	std::mutex m_blockingMutex;
};

