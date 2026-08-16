#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <deque>

enum class JobStatus
{
	NEW,
	QUEUED,
	EXECUTING,
	COMPLETED,
	RETRIEVED
};

struct JobSystemConfig
{
	// 0 is not using, -1 is same amount as core
	int m_workerCount = 0;
};

class Job
{
public:
	Job() = default;
	virtual ~Job() = default;

	virtual void Execute() = 0;

	JobStatus GetStatus() const { return m_status; }

	void SetStatus( JobStatus status ) { m_status = status; }


private:
	std::atomic<JobStatus> m_status = JobStatus::NEW;
};

class JobSystem
{
	friend class JobWorker;
public:
	JobSystem( JobSystemConfig const& config )
		: m_config( config )
	{}

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

public:
	void QueueNewJob( Job* newJob );
	Job* ClaimFirstJob();
	void FinishJob( Job* finishedJob );
	Job* RetrieveJob( Job* retrivedJob = nullptr );

	bool HasUnfinishedJob();

	bool IsEmpty();

private:
	void CreateNewWorkers( int workerCount );
	void DestroyWorkers();

private:
	JobSystemConfig m_config;

	std::atomic<bool> m_isQuitting = false;

	std::vector<JobWorker*> m_workers;

	std::deque<Job*> m_queuedJobs;
	std::mutex m_queuedJobsMutex;

	std::deque<Job*> m_claimedJobs;
	std::mutex m_claimedJobsMutex;

	std::deque<Job*> m_completedJobs;
	std::mutex m_completedJobsMutex;

// 	std::deque<Job*> m_retrivedJobs;
// 	std::mutex m_retrivedJobsMutex;
};

class JobWorker
{
	friend class JobSystem;
public:
	JobWorker( int id, JobSystem* );
	~JobWorker();

	void ThreadMain();

private:
	int m_id;
	JobSystem* m_jobSysRef;
	std::thread* m_thread = nullptr;
};
