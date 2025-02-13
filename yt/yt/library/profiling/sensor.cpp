#include "sensor.h"
#include "impl.h"

#include <library/cpp/yt/assert/assert.h>

#include <library/cpp/yt/string/format.h>

#include <util/system/compiler.h>

#include <atomic>

namespace NYT::NProfiling {

////////////////////////////////////////////////////////////////////////////////

void TCounter::Increment(i64 delta) const
{
    if (!Counter_) {
        return;
    }

    Counter_->Increment(delta);
}

TCounter::operator bool() const
{
    return Counter_.operator bool();
}

////////////////////////////////////////////////////////////////////////////////

void TTimeCounter::Add(TDuration delta) const
{
    if (!Counter_) {
        return;
    }

    Counter_->Add(delta);
}

TTimeCounter::operator bool() const
{
    return Counter_.operator bool();
}

////////////////////////////////////////////////////////////////////////////////

void TGauge::Update(double value) const
{
    if (!Gauge_) {
        return;
    }

    Gauge_->Update(value);
}

TGauge::operator bool() const
{
    return Gauge_.operator bool();
}

////////////////////////////////////////////////////////////////////////////////

void TTimeGauge::Update(TDuration value) const
{
    if (!Gauge_) {
        return;
    }

    Gauge_->Update(value);
}

TTimeGauge::operator bool() const
{
    return Gauge_.operator bool();
}

////////////////////////////////////////////////////////////////////////////////

void TSummary::Record(double value) const
{
    if (!Summary_) {
        return;
    }

    Summary_->Record(value);
}

TSummary::operator bool() const
{
    return Summary_.operator bool();
}

////////////////////////////////////////////////////////////////////////////////

void TEventTimer::Record(TDuration value) const
{
    if (!Timer_) {
        return;
    }

    Timer_->Record(value);
}

TEventTimer::operator bool() const
{
    return Timer_.operator bool();
}

////////////////////////////////////////////////////////////////////////////////

TEventTimerGuard::TEventTimerGuard(TEventTimer timer)
    : Timer_(std::move(timer))
    , StartTime_(GetCpuInstant())
{ }

TEventTimerGuard::TEventTimerGuard(TTimeGauge timeGauge)
    : TimeGauge_(std::move(timeGauge))
    , StartTime_(GetCpuInstant())
{ }

TEventTimerGuard::~TEventTimerGuard()
{
    if (!Timer_ && !TimeGauge_) {
        return;
    }

    auto duration = GetElapsedTime();
    if (Timer_) {
        Timer_.Record(duration);
    }
    if (TimeGauge_) {
        TimeGauge_.Update(duration);
    }
}

TDuration TEventTimerGuard::GetElapsedTime() const
{
    return CpuDurationToDuration(GetCpuInstant() - StartTime_);
}

////////////////////////////////////////////////////////////////////////////////

void TGaugeHistogram::Add(double value, int count) noexcept
{
    if (!Histogram_) {
        return;
    }

    Histogram_->Add(value, count);
}

void TGaugeHistogram::Remove(double value, int count) noexcept
{
    if (!Histogram_) {
        return;
    }

    Histogram_->Remove(value, count);
}

void TGaugeHistogram::Reset() noexcept
{
    if (!Histogram_) {
        return;
    }

    Histogram_->Reset();
}

THistogramSnapshot TGaugeHistogram::GetSnapshot() const
{
    if (!Histogram_) {
        return {};
    }

    return Histogram_->GetSnapshot();
}

void TGaugeHistogram::LoadSnapshot(THistogramSnapshot snapshot)
{
    if (!Histogram_) {
        return;
    }

    Histogram_->LoadSnapshot(snapshot);
}

TGaugeHistogram::operator bool() const
{
    return Histogram_.operator bool();
}

////////////////////////////////////////////////////////////////////////////////

TString ToString(const TSensorOptions& options)
{
    return Format(
        "{sparse=%v;global=%v;hot=%v;histogram_min=%v;histogram_max=%v;histogram_bounds=%v}",
        options.Sparse,
        options.Global,
        options.Hot,
        options.HistogramMin,
        options.HistogramMax,
        options.HistogramBounds
    );
}

bool TSensorOptions::IsCompatibleWith(const TSensorOptions& other) const
{
    return Sparse == other.Sparse &&
        Global == other.Global &&
        DisableSensorsRename == other.DisableSensorsRename &&
        DisableDefault == other.DisableDefault &&
        DisableProjections == other.DisableProjections;
}

////////////////////////////////////////////////////////////////////////////////

TProfiler::TProfiler(
    const IRegistryImplPtr& impl,
    const TString& prefix,
    const TString& _namespace)
    : Enabled_(true)
    , Prefix_(prefix)
    , Namespace_(_namespace)
    , Impl_(impl)
{ }

TProfiler::TProfiler(
    const TString& prefix,
    const TString& _namespace,
    const TTagSet& tags,
    const IRegistryImplPtr& impl,
    TSensorOptions options)
    : Enabled_(true)
    , Prefix_(prefix)
    , Namespace_(_namespace)
    , Tags_(tags)
    , Options_(options)
    , Impl_(impl ? impl : GetGlobalRegistry())
{ }

TProfiler TProfiler::WithPrefix(const TString& prefix) const
{
    if (!Enabled_) {
        return {};
    }

    return TProfiler(Prefix_ + prefix, Namespace_, Tags_, Impl_, Options_);
}

TProfiler TProfiler::WithTag(const TString& name, const TString& value, int parent) const
{
    if (!Enabled_) {
        return {};
    }

    auto allTags = Tags_;
    allTags.AddTag(std::pair(name, value), parent);
    return TProfiler(Prefix_, Namespace_, allTags, Impl_, Options_);
}

void TProfiler::RenameDynamicTag(const TDynamicTagPtr& tag, const TString& name, const TString& value) const
{
    if (!Impl_) {
        return;
    }

    Impl_->RenameDynamicTag(tag, name, value);
}

TProfiler TProfiler::WithRequiredTag(const TString& name, const TString& value, int parent) const
{
    if (!Enabled_) {
        return {};
    }

    auto allTags = Tags_;
    allTags.AddRequiredTag(std::pair(name, value), parent);
    return TProfiler(Prefix_, Namespace_, allTags, Impl_, Options_);
}

TProfiler TProfiler::WithExcludedTag(const TString& name, const TString& value, int parent) const
{
    if (!Enabled_) {
        return {};
    }

    auto allTags = Tags_;
    allTags.AddExcludedTag(std::pair(name, value), parent);
    return TProfiler(Prefix_, Namespace_, allTags, Impl_, Options_);
}

TProfiler TProfiler::WithAlternativeTag(const TString& name, const TString& value, int alternativeTo, int parent) const
{
    if (!Enabled_) {
        return {};
    }

    auto allTags = Tags_;

    allTags.AddAlternativeTag(std::pair(name, value), alternativeTo, parent);
    return TProfiler(Prefix_, Namespace_, allTags, Impl_, Options_);
}

TProfiler TProfiler::WithExtensionTag(const TString& name, const TString& value, int extensionOf) const
{
    if (!Enabled_) {
        return {};
    }

    auto allTags = Tags_;

    allTags.AddExtensionTag(std::pair(name, value), extensionOf);
    return TProfiler(Prefix_, Namespace_, allTags, Impl_, Options_);
}

TProfiler TProfiler::WithTags(const TTagSet& tags) const
{
    if (!Enabled_) {
        return {};
    }

    auto allTags = Tags_;
    allTags.Append(tags);
    return TProfiler(Prefix_, Namespace_, allTags, Impl_, Options_);
}

TProfiler TProfiler::WithSparse() const
{
    if (!Enabled_) {
        return {};
    }

    auto opts = Options_;
    opts.Sparse = true;
    return TProfiler(Prefix_, Namespace_, Tags_, Impl_, opts);
}

TProfiler TProfiler::WithDense() const
{
    if (!Enabled_) {
        return {};
    }

    auto opts = Options_;
    opts.Sparse = false;
    return TProfiler(Prefix_, Namespace_, Tags_, Impl_, opts);
}

TProfiler TProfiler::WithGlobal() const
{
    if (!Enabled_) {
        return {};
    }

    auto opts = Options_;
    opts.Global = true;
    return TProfiler(Prefix_, Namespace_, Tags_, Impl_, opts);
}

TProfiler TProfiler::WithDefaultDisabled() const
{
    if (!Enabled_) {
        return {};
    }

    auto opts = Options_;
    opts.DisableDefault = true;
    return TProfiler(Prefix_, Namespace_, Tags_, Impl_, opts);
}

TProfiler TProfiler::WithProjectionsDisabled() const
{
    if (!Enabled_) {
        return {};
    }

    auto allTags = Tags_;
    allTags.SetEnabled(false);

    auto opts = Options_;
    opts.DisableProjections = true;

    return TProfiler(Prefix_, Namespace_, allTags, Impl_, opts);
}

TProfiler TProfiler::WithRenameDisabled() const
{
    if (!Enabled_) {
        return {};
    }

    auto opts = Options_;
    opts.DisableSensorsRename = true;
    return TProfiler(Prefix_, Namespace_, Tags_, Impl_, opts);
}

TProfiler TProfiler::WithProducerRemoveSupport() const
{
    if (!Enabled_) {
        return {};
    }

    auto opts = Options_;
    opts.ProducerRemoveSupport = true;
    return TProfiler(Prefix_, Namespace_, Tags_, Impl_, opts);
}

TProfiler TProfiler::WithHot(bool value) const
{
    if (!Enabled_) {
        return {};
    }

    auto opts = Options_;
    opts.Hot = value;
    return TProfiler(Prefix_, Namespace_, Tags_, Impl_, opts);
}

TCounter TProfiler::Counter(const TString& name) const
{
    if (!Impl_) {
        return {};
    }

    TCounter counter;
    counter.Counter_ = Impl_->RegisterCounter(Namespace_ + Prefix_ + name, Tags_, Options_);
    return counter;
}

TTimeCounter TProfiler::TimeCounter(const TString& name) const
{
    if (!Impl_) {
        return {};
    }

    TTimeCounter counter;
    counter.Counter_ = Impl_->RegisterTimeCounter(Namespace_ + Prefix_ + name, Tags_, Options_);
    return counter;
}

TGauge TProfiler::Gauge(const TString& name) const
{
    if (!Impl_) {
        return TGauge();
    }

    TGauge gauge;
    gauge.Gauge_ = Impl_->RegisterGauge(Namespace_ + Prefix_ + name, Tags_, Options_);
    return gauge;
}

TTimeGauge TProfiler::TimeGauge(const TString& name) const
{
    if (!Impl_) {
        return TTimeGauge();
    }

    TTimeGauge gauge;
    gauge.Gauge_ = Impl_->RegisterTimeGauge(Namespace_ + Prefix_ + name, Tags_, Options_);
    return gauge;
}

TSummary TProfiler::Summary(const TString& name) const
{
    if (!Impl_) {
        return {};
    }

    TSummary summary;
    summary.Summary_ = Impl_->RegisterSummary(Namespace_ + Prefix_ + name, Tags_, Options_);
    return summary;
}

TGauge TProfiler::GaugeSummary(const TString& name) const
{
    if (!Impl_) {
        return {};
    }

    TGauge gauge;
    gauge.Gauge_ = Impl_->RegisterGaugeSummary(Namespace_ + Prefix_ + name, Tags_, Options_);
    return gauge;
}

TTimeGauge TProfiler::TimeGaugeSummary(const TString& name) const
{
    if (!Impl_) {
        return {};
    }

    TTimeGauge gauge;
    gauge.Gauge_ = Impl_->RegisterTimeGaugeSummary(Namespace_ + Prefix_ + name, Tags_, Options_);
    return gauge;
}

TEventTimer TProfiler::Timer(const TString& name) const
{
    if (!Impl_) {
        return {};
    }

    TEventTimer timer;
    timer.Timer_ = Impl_->RegisterTimerSummary(Namespace_ + Prefix_ + name, Tags_, Options_);
    return timer;
}

TEventTimer TProfiler::Histogram(const TString& name, TDuration min, TDuration max) const
{
    if (!Impl_) {
        return {};
    }

    auto options = Options_;
    options.HistogramMin = min;
    options.HistogramMax = max;

    TEventTimer timer;
    timer.Timer_ = Impl_->RegisterTimerHistogram(Namespace_ + Prefix_ + name, Tags_, options);
    return timer;
}

TEventTimer TProfiler::Histogram(const TString& name, std::vector<TDuration> bounds) const
{
    if (!Impl_) {
        return {};
    }

    TEventTimer timer;
    auto options = Options_;
    options.HistogramBounds = std::move(bounds);
    timer.Timer_ = Impl_->RegisterTimerHistogram(Namespace_ + Prefix_ + name, Tags_, options);
    return timer;
}

TGaugeHistogram TProfiler::GaugeHistogram(const TString& name, std::vector<double> buckets) const
{
    if (!Impl_) {
        return {};
    }

    TGaugeHistogram histogram;
    auto options = Options_;
    options.GaugeHistogramBounds = std::move(buckets);
    histogram.Histogram_ = Impl_->RegisterGaugeHistogram(Namespace_ + Prefix_ + name, Tags_, options);
    return histogram;
}

void TProfiler::AddFuncCounter(
    const TString& name,
    const TRefCountedPtr& owner,
    std::function<i64()> reader) const
{
    if (!Impl_) {
        return;
    }

    Impl_->RegisterFuncCounter(Namespace_ + Prefix_ + name, Tags_, Options_, owner, reader);
}

void TProfiler::AddFuncGauge(
    const TString& name,
    const TRefCountedPtr& owner,
    std::function<double()> reader) const
{
    if (!Impl_) {
        return;
    }

    Impl_->RegisterFuncGauge(Namespace_ + Prefix_ + name, Tags_, Options_, owner, reader);
}

void TProfiler::AddProducer(
    const TString& prefix,
    const ISensorProducerPtr& producer) const
{
    if (!Impl_) {
        return;
    }

    Impl_->RegisterProducer(Namespace_ + Prefix_ + prefix, Tags_, Options_, producer);
}

const IRegistryImplPtr& TProfiler::GetRegistry() const
{
    return Impl_;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NProfiling
