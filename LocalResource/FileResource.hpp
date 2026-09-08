#ifndef FILERESOURCE_HPP
#define FILERESOURCE_HPP

#include "LocalResource.hpp"
#include <fstream>
#include <string>

class FileResource : public LocalResource {
	private:
		std::string     path;
		std::ifstream   file;
		bool            headers_sent;

		void    generate_headers(std::string& write_buffer);

	public:
		FileResource(const std::string& base_path, const std::string& req_uri);
		virtual ~FileResource();

		virtual bool process(std::string& write_buffer);
};

#endif
