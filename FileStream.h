#pragma once

#ifdef HAS_RNS

#include "FileSystem.h"
#include "FileSystemType.h"

	class FileStream : public RNS::FileStreamImpl {

	private:
		std::unique_ptr<File> _file;
		bool _closed = false;

	public:
		FileStream(File* file) : RNS::FileStreamImpl(), _file(file) {}
		virtual ~FileStream() { if (!_closed) close(); }

	public:
		inline virtual const char* name() { return _file->name(); }
		inline virtual size_t size() { return _file->size(); }
		inline virtual void close() { _closed = true; _file->close(); }

		// Print overrides
		inline virtual size_t write(uint8_t byte) { return _file->write(byte); }
		inline virtual size_t write(const uint8_t *buffer, size_t size) { return _file->write(buffer, size); }

		// Stream overrides
		inline virtual int available() { return _file->available(); }
		inline virtual int read() { return _file->read(); }
		inline virtual int peek() { return _file->peek(); }
		inline virtual void flush() { _file->flush(); }

	};

#endif
