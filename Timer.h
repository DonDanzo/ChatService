#pragma once
#include <chrono>
#include <functional>
#include <thread>
#include <future>

#include <string>
#include <iostream>

class Timer
{
public:
	//Start async timer with ID, time duration and callback function
	template<typename _Rep, typename _Period>
	static std::future<void> StartTimerAsync(std::chrono::duration<_Rep, _Period> duration, size_t timerId, std::function<void(size_t)> callback)
	{
		return std::async(std::launch::async, [duration, timerId, callback]()
			{
				std::this_thread::sleep_for(duration);
				callback(timerId);
			});
	};
};

