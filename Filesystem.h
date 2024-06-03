#pragma once

#include <Filesystem.h>
#include <Bytes.h>
#include <Type.h>

class Filesystem : public RNS::Filesystem {

public:
	bool init();

public:
	virtual bool file_exists(const char* file_path);
	virtual const RNS::Bytes read_file(const char* file_path);
	virtual bool write_file(const RNS::Bytes& data, const char* file_path);
	virtual bool remove_file(const char* file_path);
	virtual bool rename_file(const char* from_file_path, const char* to_file_path);
	virtual bool directory_exists(const char* directory_path);
	virtual bool create_directory(const char* directory_path);
	virtual bool remove_directory(const char* directory_path);
	virtual std::list<std::string> list_directory(const char* directory_path);

};
