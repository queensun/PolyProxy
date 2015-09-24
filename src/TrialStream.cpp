#include "TrialStream.h"

void TrialStream::readSome(const MutableBuffers& buffers, const ReadHandler& handler)
{
	std::shared_ptr<HostStatistics> statistics = statistics_;
	nextLayer_->readSome(buffers, handler);
}
void TrialStream::writeSome(const ConstBuffers& buffers, const WriteHandler& handler)
{
	std::shared_ptr<HostStatistics> statistics = statistics_;
	nextLayer_->writeSome(buffers, handler);
}