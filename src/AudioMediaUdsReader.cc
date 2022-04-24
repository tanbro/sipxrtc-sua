#include "AudioMediaUdsReader.hh"

#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <functional>
#include <iostream>
#include <sstream>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

#include <glog/logging.h>

#define THIS_FILE "AudioMediaUdsReader.cxx"

using namespace std;
using namespace pj;

namespace sipxsua {

AudioMediaUdsReader::AudioMediaUdsReader() : AudioMedia() {
  id == PJSUA_INVALID_ID;
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "AudioMediaUdsReader", 8192, 8192,
                        NULL);
  CHECK_NOTNULL(pool);
}

AudioMediaUdsReader::~AudioMediaUdsReader() {
  /// TODO: 死循环，退不出来的
  if (read_thread.joinable()) {
    read_thread.join();
  }
  if (id != PJSUA_INVALID_ID) {
    unregisterMediaPort();
  }
  if (sockfd != 0) {
    PCHECK(close(sockfd));
  }
  if (NULL != src_state) {
    src_delete(src_state);
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void AudioMediaUdsReader::createPlayer(const pj::MediaFormatAudio &audioFormat,
                                       const std::string &path,
                                       unsigned sampleRate,
                                       unsigned bufferMSec) {
  CHECK_EQ(id, PJSUA_INVALID_ID) << ":不允许重复创建 player";

  // 只支持 Mono channel
  CHECK_EQ(1, audioFormat.channelCount) << "仅支持 mono audio";
  // 只支持 16bits sampling
  CHECK_EQ(16, audioFormat.bitsPerSample) << "仅支持 16bits sampling";

  this->audioFormat = audioFormat;
  this->sampleRate = sampleRate;

  // 远端的声音媒体作为 source，获取它的格式规格
  // 计算缓冲区大小
  size_t bytes_per_sec = audioFormat.clockRate * audioFormat.channelCount *
                         audioFormat.bitsPerSample / 8; // 每秒的字节数
  buffer_size = bytes_per_sec * bufferMSec / 1000;
  unsigned int samples_per_frame = audioFormat.clockRate *
                                   audioFormat.channelCount *
                                   audioFormat.frameTimeUsec / 1000000;

  //分配缓冲区
  assert(buffer_size == sizeof(buffer0));
  buffer = (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));

  // 准备绑定文件
  memset(&recv_addr, 0, sizeof(sockaddr_un));
  recv_addr.sun_family = AF_LOCAL;
  strncpy(recv_addr.sun_path, path.c_str(), sizeof(recv_addr.sun_path) - 1);
  struct stat statbuf;
  if (!stat(recv_addr.sun_path, &statbuf)) {
    if (unlink(recv_addr.sun_path)) {
      ostringstream oss;
      oss << THIS_FILE " unlink error (" << errno << "): " << strerror(errno);
      cerr << oss.str() << endl;
      throw new runtime_error(oss.str());
    }
  }
  // 打开 Unix domain socket
  sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    ostringstream oss;
    oss << THIS_FILE " socket error (" << errno << "): " << strerror(errno);
    cerr << oss.str() << endl;
    throw new runtime_error(oss.str());
  }
  // 绑定文件
  if (bind(sockfd, (const sockaddr *)&recv_addr, sizeof(recv_addr))) {
    ostringstream oss;
    oss << THIS_FILE " socket bind error (" << errno
        << "): " << strerror(errno);
    cerr << oss.str() << endl;
    throw new runtime_error(oss.str());
  }

  // 建立内存播放 Audio Port
  memset(buffer0, 0, sizeof(buffer0));
  memset(buffer, 0, buffer_size);
  DVLOG(1) << "createPlayer() ... " << endl
           << "  buffer_size=" << buffer_size << ", " << endl
           << "  sample_rate=" << audioFormat.clockRate << ", " << endl
           << "  channel=" << audioFormat.channelCount << ", " << endl
           << "  samples_per_frame=" << samples_per_frame << ", " << endl
           << "  bits_per_sample=" << audioFormat.bitsPerSample;
  PJSUA2_CHECK_EXPR(pjmedia_mem_player_create(
      pool, buffer, buffer_size, audioFormat.clockRate,
      audioFormat.channelCount, samples_per_frame, audioFormat.bitsPerSample, 0,
      &port));
  // 接收回调
  PJSUA2_CHECK_EXPR(
      pjmedia_mem_player_set_eof_cb2(port, (void *)this, cb_mem_play_eof));
  // 如果上面一步失败，就不会产生有效的 media id.
  // C++ way： 把 Port 加入到 conf，并接收新的 port id 到这个类的 id 属性
  registerMediaPort2(port, pool);
  DVLOG(1) << "createPlayer() ... id=" << id;

  // 启动接收线程（先来个死循环试试）
  DVLOG(3) << "createPlayer() ... Reader worker thread starting ...";
  /// FIXME: 这里有死锁！
  {
    mutex mtx;
    condition_variable cv;
    unique_lock<mutex> lk(mtx);
    read_thread =
        thread(&AudioMediaUdsReader::read_worker, this, ref(mtx), ref(cv));
    cv.wait(lk);
  }
  DVLOG(3) << "createPlayer() ... Reader worker thread started.";
}

void AudioMediaUdsReader::onBufferEof() {
  lock_guard<mutex> lk(read_mutext);
  memcpy(buffer, buffer0, sizeof(buffer0));
  memset(buffer0, 0, sizeof(buffer0));
}

void AudioMediaUdsReader::read_worker(std::mutex &mtx,
                                      std::condition_variable &cv) {
  { lock_guard<mutex> lk(mtx); }
  cv.notify_all();
  uint8_t buf_tmp[1920];
  ssize_t length;
  for (;;) {
    length = recv(sockfd, buf_tmp, sizeof(buf_tmp), 0);
    // cout << THIS_FILE " read_worker:  " << length << "bytes received." <<
    // endl;
    if (length == -1) {
      PCHECK(errno) << ": recv() error in read_worker()";
    }
    // TRTC SDK 一定是 1920
    assert(length == 1920);
    {
      lock_guard<mutex> lk(read_mutext);
      memcpy(buffer0, buf_tmp, sizeof(buffer0));
    }
  }
}

void AudioMediaUdsReader::cb_mem_play_eof(pjmedia_port *port, void *usr_data) {
  ((AudioMediaUdsReader *)usr_data)->onBufferEof();
}

} // namespace sipxsua
