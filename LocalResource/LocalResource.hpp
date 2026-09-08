#ifndef LOCALRESOURCE_HPP
#define LOCALRESOURCE_HPP

#include <string>
#include "HttpRequest.hpp"

class LocalResource {
	public:
		virtual ~LocalResource() {}

		// True if finished, false if not so we can nonblock
		virtual bool process(std::string& write_buffer) = 0;
};

#endif
