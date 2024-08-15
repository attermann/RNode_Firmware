#pragma once

#ifdef HAS_RNS

#include <FileSystem.h>
#include <Bytes.h>
#include <Type.h>

class NoopFileSystem : public RNS::FileSystemImpl {

public:
	NoopFileSystem() {}

	bool init() { return true; }
	bool format() { return false; }
	bool reformat() { return false; }

public:
	// CBA Debug
	static void listDir(const char* dir, const char* prefix = "") {}
	static void dumpDir(const char* dir) {}

public:
	virtual bool file_exists(const char* file_path) { return false; }
	virtual size_t read_file(const char* file_path, RNS::Bytes& data) { return 0; }
	virtual size_t write_file(const char* file_path, const RNS::Bytes& data) { return 0; }
	virtual bool remove_file(const char* file_path) { return false; }
	virtual bool rename_file(const char* from_file_path, const char* to_file_path) { return false; }
	virtual bool directory_exists(const char* directory_path) { return false; }
	virtual bool create_directory(const char* directory_path) { return false; }
	virtual bool remove_directory(const char* directory_path) { return false; }
	virtual std::list<std::string> list_directory(const char* directory_path) { return std::list<std::string>(); }
	virtual size_t storage_size() { return 0; }
	virtual size_t storage_available() { return 0; }

};

#endif
