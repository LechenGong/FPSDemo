#include "Engine/BehaviorTree/TreeNodes/LeafNodes/ExecutionWait.h"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/Core/Clock.hpp"

ExecutionWait::ExecutionWait( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
    : LeafNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionWait::ParseDataFromXml( XmlElement* xmlElement )
{
    m_duration = ParseXmlAttribute( *xmlElement, "Duration", 0.f );
}

void ExecutionWait::ExportAttributeToXml( XmlElement* xmlElement )
{
    xmlElement->SetAttribute( "Duration", m_duration );
}

void ExecutionWait::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
    m_timeStarts = m_btRef->m_clock->GetTotalSeconds();
}

NodeStatus ExecutionWait::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
    float timeNow = m_btRef->m_clock->GetTotalSeconds();
    float timeElapsed = timeNow - m_timeStarts;
    if (timeElapsed >= m_duration)
    {
        if (m_btRef->m_lastRunningNode == this)
            m_btRef->m_lastRunningNode = nullptr;
        return m_status = NodeStatus::SUCCESS;
    }
    m_btRef->m_lastRunningNode = this;
    return m_status = NodeStatus::RUNNING;
}

std::string ExecutionWait::GetTypeName() const
{
    return "Wait";
}
