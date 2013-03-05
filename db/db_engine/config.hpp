#ifndef __DB_PROTOBUF_CONFIG_HPP
#define __DB_PROTOBUF_CONFIG_HPP

#include <google/protobuf/message.h>
#include <functional>


namespace proto {

	typedef std::function<void(const std::string &)>	error_handler_t;

	typedef std::shared_ptr<google::protobuf::Message>	msg_ptr;

}

#endif