#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

JobSystem* g_jobSystem = nullptr;

void JobSystem::Startup()
{
	int threadCount;
	if (m_config.m_workerCount == -1)
	{
		threadCount = std::thread::hardware_concurrency() - 1;
	}
	else
	{
		threadCount = m_config.m_workerCount;
	}
	CreateNewWorkers( threadCount );
}

void JobSystem::BeginFrame()
{
}

void JobSystem::EndFrame()
{
}

void JobSystem::Shutdown()
{
	m_isQuitting = true;
	DestroyWorkers();
}

void JobSystem::QueueNewJob( Job* newJob )
{
	newJob->SetStatus( JobStatus::QUEUED );
	m_queuedJobsMutex.lock();
	m_queuedJobs.push_back( newJob );
	m_queuedJobsMutex.unlock();
}

Job* JobSystem::ClaimFirstJob()
{
	m_queuedJobsMutex.lock();
	if (m_queuedJobs.size() > 0)
	{
		Job* job = m_queuedJobs.front();
		m_queuedJobs.pop_front();
		m_queuedJobsMutex.unlock();

		job->SetStatus( JobStatus::EXECUTING );

		m_claimedJobsMutex.lock();
		m_claimedJobs.push_back( job );
		m_claimedJobsMutex.unlock();

		return job;
	}
	m_queuedJobsMutex.unlock();
	return nullptr;
}

void JobSystem::FinishJob( Job* finishedJob )
{
	m_claimedJobsMutex.lock();
	for (auto claimedJobIter = m_claimedJobs.begin(); claimedJobIter != m_claimedJobs.end(); claimedJobIter++)
	{
		if (*claimedJobIter == finishedJob)
		{
			m_claimedJobs.erase( claimedJobIter );
			m_claimedJobsMutex.unlock();

			finishedJob->SetStatus( JobStatus::COMPLETED );

			m_completedJobsMutex.lock();
			m_completedJobs.push_back( finishedJob );
			m_completedJobsMutex.unlock();
			return;
		}
	}
	m_claimedJobsMutex.unlock();
}

Job* JobSystem::RetrieveJob( Job* retrivedJob )
{
	m_completedJobsMutex.lock();
	if (retrivedJob)
	{
		for (auto claimedJobIter = m_completedJobs.begin(); claimedJobIter != m_completedJobs.end(); claimedJobIter++)
		{
			if (*claimedJobIter == retrivedJob)
			{
				retrivedJob->SetStatus( JobStatus::RETRIEVED );
				m_completedJobs.erase( claimedJobIter );
				m_completedJobsMutex.unlock();
				return retrivedJob;
			}
		}
		m_completedJobsMutex.unlock();
		return nullptr;
	}
	else
	{
		if (m_completedJobs.size() > 0)
		{
			Job* firstJob = m_completedJobs.front();
			firstJob->SetStatus( JobStatus::RETRIEVED );
			m_completedJobs.pop_front();
			m_completedJobsMutex.unlock();
			return firstJob;
		}
		else
		{
			m_completedJobsMutex.unlock();
			return nullptr;
		}
	}
}

bool JobSystem::HasUnfinishedJob()
{
	m_queuedJobsMutex.lock();
	m_claimedJobsMutex.lock();

	bool flag = !(m_queuedJobs.empty() && m_claimedJobs.empty());

	m_queuedJobsMutex.unlock();
	m_claimedJobsMutex.unlock();

	return flag;
}

bool JobSystem::IsEmpty()
{
	m_queuedJobsMutex.lock();
	m_claimedJobsMutex.lock();
	m_completedJobsMutex.lock();

	bool flag = !(m_queuedJobs.empty() && m_claimedJobs.empty() && m_completedJobs.empty());

	m_queuedJobsMutex.unlock();
	m_claimedJobsMutex.unlock();
	m_completedJobsMutex.unlock();

	return flag;
}

void JobSystem::CreateNewWorkers( int workerCount )
{
	for (int i = 0; i < workerCount; i++)
	{
		JobWorker* worker = new JobWorker( i, this );
		m_workers.push_back( worker );
	}
}

void JobSystem::DestroyWorkers()
{
	for (int i = 0; i < m_workers.size(); i++)
	{
		delete m_workers[i];
		m_workers[i] = nullptr;
	}
	m_workers.clear();
}

JobWorker::JobWorker( int id, JobSystem* jobSystem )
	: m_id( id )
	, m_jobSysRef( jobSystem )
{
	m_thread = new std::thread( &JobWorker::ThreadMain, this );
}

JobWorker::~JobWorker()
{
	m_thread->join();
}

void JobWorker::ThreadMain()
{
	while (!g_jobSystem->m_isQuitting)
	{
		Job* claimedJob = m_jobSysRef->ClaimFirstJob();
		if (claimedJob)
		{
			claimedJob->Execute();
			m_jobSysRef->FinishJob( claimedJob );
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );
		}
	}
}
