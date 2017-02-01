#include <iostream>
#include <iomanip>
#include <sstream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller(const bool debug)
  : debug_(debug)
  , window_size_(1)
  , datagram_list_()
  , log_()
{
    time_t t = time(nullptr);
    struct tm *now = localtime(&t);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "rtt-%Y-%m-%dT%H-%M-%S.log", now);
    string filename(buffer);
    cerr << "Log saved to " + filename << endl;

    log_.reset(new ofstream(filename));
}

/* Payload size of every datagram */
unsigned int Controller::payload_size(void)
{
  return 0;
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size(void)
{
  if (debug_) {
    cerr << "At time " << timestamp_ms()
    << " window size is " << window_size_ << endl;
  }

  return window_size_;
}

/* If window is open to send more datagrams */
bool Controller::window_is_open(void)
{
  return datagram_list_.size() < window_size_;
}

/* Set the period in ms of timeout timer (return 0 to disable timer) */
unsigned int Controller::timer_period(void)
{
  return 0;
}

/* Timeout timer fires */
void Controller::timer_fires(void)
{
  if (debug_) {
    cerr << "At time " << timestamp_ms()
    << " timeout timer fires" << endl;
  }
}

/* A datagram was sent */
void Controller::datagram_was_sent(
    const uint64_t sequence_number,
    /* of the sent datagram */
    const uint64_t send_timestamp)
    /* in milliseconds */
{
  if (debug_) {
    cerr << "At time " << send_timestamp
    << " sent datagram " << sequence_number << endl;
  }

  datagram_list_.emplace_back(sequence_number, send_timestamp);
}

/* An ack was received */
void Controller::ack_received(
    const uint64_t sequence_number_acked,
    /* what sequence number was acknowledged */
    const uint64_t send_timestamp_acked,
    /* when the acknowledged datagram was sent (sender's clock) */
    const uint64_t recv_timestamp_acked,
    /* when the acknowledged datagram was received (receiver's clock) */
    const uint64_t timestamp_ack_received)
    /* when the ack was received (by sender) */
{
  if (debug_) {
    cerr << "At time " << timestamp_ack_received
    << " received ack for datagram " << sequence_number_acked
    << " (send @ time " << send_timestamp_acked
    << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
    << endl;
  }

  /* Write RTTs to log */
  uint64_t rtt = timestamp_ack_received - send_timestamp_acked;
  cerr << rtt << endl;
  *log_ << rtt << endl;

  auto it = datagram_list_.begin();
  while (it != datagram_list_.end()) {
    if (it->first == sequence_number_acked) {
      it = datagram_list_.erase(it);
      break;
    } else if (it->first > sequence_number_acked) {
      break;
    } else {
      it++;
    }
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
int Controller::timeout_ms(void)
{
  return -1;
}
