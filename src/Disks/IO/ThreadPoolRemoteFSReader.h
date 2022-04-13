#pragma once

#include <IO/AsynchronousReader.h>
#include <IO/SeekableReadBuffer.h>
#include <Common/ThreadPool.h>
#include <Disks/IO/ReadBufferFromRemoteFSGather.h>
#include <Disks/IDiskRemote.h>


namespace DB
{

template <class Reader>
class ThreadPoolRemoteFSReader : public IAsynchronousReader
{
public:
    ThreadPoolRemoteFSReader(size_t pool_size, size_t queue_size_);

    std::future<Result> submit(Request request) override;

private:
    ThreadPool pool;
};


template <class Reader>
class RemoteFSFileDescriptor : public IAsynchronousReader::IFileDescriptor
{
public:
    explicit RemoteFSFileDescriptor(std::shared_ptr<Reader> reader_) : reader(std::move(reader_)) { }

    IAsynchronousReader::Result readInto(char * data, size_t size, size_t offset, size_t ignore = 0);

private:
    std::shared_ptr<Reader> reader;
};

}
