#pragma once
#include <thread>
#include <atomic>
#include <functional>

#include <shared_mutex>

namespace fth
{
	struct ParallelExecutor
	{
		static const uint32_t MAX_THREADS; // 100% CPU occupation, it may cause OS hitches.
		// No point to have more threads than the number of CPU logical cores.

		static const uint32_t HALF_THREADS; // 50-100% CPU occupation

		using Func = std::function<void(uint32_t, uint32_t)>; // (threadIndex, taskIndex)

		ParallelExecutor(uint32_t numThreads)
		{
			m_isLooping = true;

			FTH_ASSERT_ENGINE(numThreads > 0, "threads are valid");
			m_finishedThreadNum = 0;

			m_threads.reserve(numThreads);
			for (uint32_t i = 0; i < numThreads; ++i)
				m_threads.emplace_back([this, i]() { workLoop(i); });
		}

		~ParallelExecutor()
		{
			wait();
			m_isLooping = false;
			awake();

			for (auto& t : m_threads) t.join();
		}

		uint32_t numThreads() const { return static_cast<uint32_t>(m_threads.size()); }
		bool isWorking() const { return m_finishedThreadNum < m_threads.size(); }

		void wait()
		{
			if (!isWorking()) return;

			std::unique_lock<std::shared_mutex> lock(m_mutex);
			if (!isWorking()) return; // re-check for a case when threads finished and m_waitCV is notified before the lock is acquired

			m_waitCV.wait(lock);
		}

		// Executes a function in parallel blocking the caller thread.
		void execute(const Func& func, uint32_t numTasks, uint32_t tasksPerBatch)
		{
			if (numTasks == 0) return;
			executeAsync(func, numTasks, tasksPerBatch);
			wait();
		}

		// Executes a function in parallel asynchronously.
		void executeAsync(const Func& func, uint32_t numTasks, uint32_t tasksPerBatch)
		{
			if (numTasks == 0) return;
			FTH_ASSERT_ENGINE(tasksPerBatch > 0, "");

			wait(); // enforced waiting for completion of the previous parallel dispatch

			m_finishedThreadNum = 0;
			m_completedBatchNum = 0;

			uint32_t numBatches = (numTasks + tasksPerBatch - 1) / tasksPerBatch;

			m_executeTasks = [this, func, numTasks, numBatches, tasksPerBatch](uint32_t threadIndex)
			{
				while (true)
				{
					uint32_t batchIndex = m_completedBatchNum.fetch_add(1);
					if (batchIndex >= numBatches) return;

					uint32_t begin = (batchIndex + 0) * tasksPerBatch;
					uint32_t end = (batchIndex + 1) * tasksPerBatch;
					if (end > numTasks) end = numTasks;

					for (uint32_t taskIndex = begin; taskIndex < end; ++taskIndex)
						func(threadIndex, taskIndex);
				}
			};

			awake();
		}

	protected:
		void awake()
		{
			m_workCV.notify_all();
		}

		void workLoop(uint32_t threadIndex)
		{
			while (true)
			{
				{
					std::shared_lock<std::shared_mutex> lock(m_mutex);

					uint32_t prevFinishedNum = m_finishedThreadNum.fetch_add(1);
					if ((prevFinishedNum + 1) == m_threads.size())
					{
						m_executeTasks = {};
						m_waitCV.notify_all(); // If an outer thread waits on m_waitCV, it will remain blocked until this thread enters m_workCV.wait(),
						// because both CVs wait with the same m_mutex. This is needed to avoid this thread missing
						// a notification on m_workCV in a situation when an outer thread unblocks after this line and before this thread
						// enters m_workCV.wait(), which would result in this thread being blocked until a next notification.
					}

					// Calling this unlocks m_mutex until m_workCV is notified, then re-locks it again until the end of the scope.
					m_workCV.wait(lock);
				}

				if (!m_isLooping) return;

				FTH_ASSERT_ENGINE(m_executeTasks, "");
				m_executeTasks(threadIndex);
			}
		}

		bool m_isLooping;

		std::atomic<uint32_t> m_finishedThreadNum;
		std::atomic<uint32_t> m_completedBatchNum;
		std::function<void(uint32_t)> m_executeTasks;

		std::shared_mutex m_mutex;
		std::condition_variable_any m_waitCV;
		std::condition_variable_any m_workCV;

		std::vector<std::thread> m_threads;
	};

}