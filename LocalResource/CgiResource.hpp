#ifndef CGIRESOURCE_HPP
#define CGIRESOURCE_HPP

#include "LocalResource.hpp"
#include <string>
#include "HttpRequest.hpp"

class CgiResource : public LocalResource {
	private:
		std::string script_path;
		bool        is_done;

	public:
		CgiResource(const std::string& base_path, const std::string& req_uri, const HttpRequest& req);
		virtual ~CgiResource();

		virtual bool process(std::string& write_buffer);
};

#endif
