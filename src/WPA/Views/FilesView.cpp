#include "FilesView.h"
#include "CppBuildInsightsEtw.h"
#include "PayloadBuilder.h"

using namespace Microsoft::Cpp::BuildInsights;
using namespace Activities;

namespace vcperf
{

AnalysisControl FilesView::OnStartActivity(const EventStack& eventStack,
    const void* relogSession)
{
    MatchEventStackInMemberFunction(eventStack, this, &FilesView::OnFileParse, relogSession);

    return AnalysisControl::CONTINUE;
}

void FilesView::OnFileParse(const FrontEndFileGroup& files, const void* relogSession)
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    PCEVENT_DESCRIPTOR desc = &CppBuildInsightsFileActivity_V1;

    auto* context = contextBuilder_->GetContextData();

    const FrontEndFile& currentFile = files.Back();

    const char* parentPath = "";

    if (files.Size() > 1)
    {
        parentPath = files[files.Size() - 2].Path();
    }

    auto& td = miscellaneousCache_->GetTimingData(currentFile);

    Payload p = PayloadBuilder<uint16_t, const char*, const char*, uint32_t, const wchar_t*, const char*,
        const char*, uint16_t, const char*, uint32_t, uint32_t, uint32_t>::Build(
            context->TimelineId,
            context->TimelineDescription,
            context->Tool,
            context->InvocationId,
            context->Component,
            currentFile.Path(),
            parentPath,
            (uint16_t)files.Size() - 1,
            "Parsing",
            (uint32_t)duration_cast<milliseconds>(td.ExclusiveDuration).count(),
            (uint32_t)duration_cast<milliseconds>(td.Duration).count(),
            (uint32_t)duration_cast<milliseconds>(td.WallClockTimeResponsibility).count()
        );
    
    InjectEvent(relogSession, &CppBuildInsightsGuid, desc, 
        currentFile.ProcessId(), currentFile.ThreadId(), currentFile.ProcessorIndex(),
        currentFile.StartTimestamp(), p.GetData(), (unsigned long)p.Size());
}

} // namespace vcperf