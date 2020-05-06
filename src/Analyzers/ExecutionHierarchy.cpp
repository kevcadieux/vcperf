#include "ExecutionHierarchy.h"

#include <assert.h>

using namespace Microsoft::Cpp::BuildInsights;
using namespace Activities;
using namespace SimpleEvents;

using namespace vcperf;

ExecutionHierarchy::ExecutionHierarchy() :
    entries_{},
    roots_{}
{
}

AnalysisControl ExecutionHierarchy::OnStartActivity(const EventStack& eventStack)
{
    if (   MatchEventStackInMemberFunction(eventStack, this, &ExecutionHierarchy::OnNestedActivity)
        || MatchEventInMemberFunction(eventStack.Back(), this, &ExecutionHierarchy::OnRootActivity))
    {

    }

    return AnalysisControl::CONTINUE;
}

AnalysisControl ExecutionHierarchy::OnStopActivity(const EventStack& eventStack)
{
    return AnalysisControl::CONTINUE;
}

AnalysisControl ExecutionHierarchy::OnSimpleEvent(const EventStack& eventStack)
{
    return AnalysisControl::CONTINUE;
}

void ExecutionHierarchy::OnRootActivity(const Activity& root)
{
    Entry* entry = CreateEntry(root);
    
    assert(std::find(roots_.begin(), roots_.end(), entry) == roots_.end());
    roots_.push_back(entry);
}

void ExecutionHierarchy::OnNestedActivity(const Activity& parent, const Activity& child)
{
    auto parentEntryIt = entries_.find(parent.EventInstanceId());
    assert(parentEntryIt != entries_.end());

    Entry* childEntry = CreateEntry(child);

    auto& children = parentEntryIt->second.Children;
    assert(std::find(children.begin(), children.end(), childEntry) == children.end());
    children.push_back(childEntry);
}

ExecutionHierarchy::Entry* ExecutionHierarchy::CreateEntry(const Activity& activity)
{
    assert(entries_.find(activity.EventInstanceId()) == entries_.end());

    Entry& entry = entries_[activity.EventInstanceId()];

    entry.Id = activity.EventInstanceId();
    entry.ProcessId = activity.ProcessId();
    entry.ThreadId = activity.ThreadId();
    entry.StartTimestamp = ConvertTime(activity.StartTimestamp(), activity.TickFrequency());
    entry.StopTimestamp = ConvertTime(activity.StopTimestamp(), activity.TickFrequency());
    // TODO: cache Name, refer to ContextBuilder::CacheString
    entry.Name = activity.EventWideName();

    return &entry;
}

std::chrono::nanoseconds ExecutionHierarchy::ConvertTime(long long ticks, long long frequency) const
{
    return std::chrono::nanoseconds{ Internal::ConvertTickPrecision(ticks, frequency, std::chrono::nanoseconds::period::den) };
}