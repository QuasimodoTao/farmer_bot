#pragma once
#include <string.h>
#include <string>
#include <objbase.h>

class StreamArray {
private:
	unsigned char* buf = nullptr;
	unsigned int len = 0;
	unsigned int buf_size = 0;
	unsigned int read_index = 0;
	unsigned int write_index = 0;
	const unsigned int inc_sclae = 4096;


protected:
	static int utf16_utf8(std::wstring* str, void* utf8) {
		const wchar_t* s;
		unsigned char* d;
		int len;
		int i;
		int ch;

		s = str->c_str();
		d = (unsigned char*)utf8;
		len = str->length();
		i = 0;
		while (len) {
			ch = s[i];
			if ((ch & 0xfc00) == 0xd800) {
				ch &= 0x3ff;
				ch <<= 10;
				ch |= s[i + 1] & 0x3ff;
				len--;
				i++;
				ch += 0x10000;
			}
			else if((ch & 0xfc00) == 0xdc00) break;
			len--;
			i++;
			if (ch < 0x80) {
				*d++ = ch;
			}
			else if (ch < 0x800) {
				*d++ = 0x80 | (ch >> 6);
				*d++ = 0x80 | (ch & 0x3f);
			}
			else if (ch < 0x10000) {
				*d++ = 0xe0 | (ch >> 12);
				*d++ = 0x80 | ((ch >> 6) & 0x3f);
				*d++ = 0x80 | (ch & 0x3f);
			}
			else {
				*d++ = 0xf0 | (ch >> 18);
				*d++ = 0x80 | ((ch >> 12) & 0x3f);
				*d++ = 0x80 | ((ch >> 6) & 0x3f);
				*d++ = 0x80 | (ch & 0x3f);
			}
		}
		return d - (unsigned char*)utf8;
	}
	static std::wstring* utf8_utf16(void * utf8,int len) {
		unsigned char* s;
		wchar_t* d;
		int i = 0;
		std::wstring* res;
		unsigned int ch;

		s = (unsigned char*)utf8;
		d = new wchar_t[len + 1];
		while (len) {
			ch = *s;
			s++;
			len--;
			if ((ch & 0x80) != 0x80) {
				d[i] = ch & 0x7f;
				i++;
				continue;
			}
			else if ((ch & 0xe0) == 0xc0) {
				ch &= 0x3f;
				ch <<= 6;
				ch |= *s & 0x3f;
				s++;
				len--;
				d[i] = ch;
				i++;
				continue;
			}
			else if ((ch & 0xf0) == 0xe0) {
				ch &= 0x1f;
				ch <<= 12;
				ch |= ((int)(s[0] & 0x3f)) << 6;
				ch |= ((int)(s[1] & 0x3f));
				s += 2;
				len -= 2;
			}
			else if ((ch & 0xf8) == 0xf0) {
				ch &= 0xf;
				ch <<= 18;
				ch |= ((int)(s[0] & 0x3f)) << 12;
				ch |= ((int)(s[1] & 0x3f)) << 6;
				ch |= ((int)(s[2] & 0x3f));
				s += 3;
				len -= 3;
			}
			else {
				break;
			}
			if (ch >= 0x10000) {
				ch -= 0x10000;
				d[i] = (ch >> 10) | 0xd800;
				i++;
				d[i] = (ch & 0x3ff) | 0xdc00;
				i++;
			}
			else {
				d[i] = ch;
				i++;
			}
		}
		d[i] = 0;
		res = new std::wstring(d);
		delete[] d;
		return res;
	}
	void Set(StreamArray* a) {
		try {
			if (!a) return;
			if (!a->buf_size) return;
			if (this->buf) delete this->buf;
			this->buf = nullptr;
			this->buf = new unsigned char[a->buf_size];
			memcpy_s(this->buf, a->buf_size, a->buf, a->buf_size);
			this->len = a->len;
			this->buf_size = a->buf_size;
			this->read_index = a->read_index;
			this->write_index = a->write_index;
		}
		catch (int e) {
		
		}
	}
public:
	const static int access_overflow = 1;

	StreamArray(StreamArray * a) {
		if (!a) return;
		if (!a->buf_size) return;
		this->buf = new unsigned char[a->buf_size];
		memcpy_s(this->buf, a->buf_size, a->buf, a->buf_size);
		this->len = a->len;
		this->buf_size = a->buf_size;
		this->read_index = a->read_index;
		this->write_index = a->write_index;
	}
	StreamArray() {
	}
	StreamArray(void* _b, unsigned int len) {
		if (!_b) return;
		if (!len) return;
		this->buf_size = (len + inc_sclae) & ~(inc_sclae - 1);
		this->buf = new unsigned char[this->buf_size];
		memcpy_s(this->buf, this->buf_size, _b, len);
		this->len = len;
	}
	~StreamArray() {
		if (this->buf) delete this->buf;
	}
	void seekg(int val) {
		if (val >= this->len) this->read_index = this->len;
		else this->read_index = val;
	}
	int tellg() {
		return this->read_index;
	}
	const void* RBuf() {
		return this->buf + this->read_index;
	}
	const void* WBuf() {
		return this->buf + this->write_index;
	}
	const void* CBuf() {
		return this->buf;
	}
	unsigned int Len() {
		return this->len;
	}
	unsigned int RLen() {
		return this->len - this->read_index;
	}
	unsigned int ReadUByte(){
		unsigned int val;
		if (!this->buf) throw access_overflow;
		if (this->read_index >= this->len) throw access_overflow;
		val = this->buf[this->read_index];
		this->read_index++;
		return val;
	}
	int ReadByte() {
		int val;
		if (!this->buf) 
			throw access_overflow;
		if (this->read_index >= this->len) 
			throw access_overflow;
		val = this->buf[this->read_index];
		this->read_index++;
		if (val & 0x80) return 0xffffff00 | val;
		else return val;
	}
	unsigned int ReadUShort() {
		unsigned int val;
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 1) >= this->len) throw access_overflow;
		val = this->buf[this->read_index + 1];
		val |= ((unsigned int)(this->buf[this->read_index])) << 8;
		this->read_index += 2;
		return val;
	}
	unsigned int ReadShort() {
		int val;
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 1) >= this->len) throw access_overflow;
		val = this->buf[this->read_index + 1];
		val |= ((unsigned int)(this->buf[this->read_index])) << 8;
		this->read_index += 2;
		if (val & 0x8000) return 0xffff0000 | val;
		else return val;
	}
	unsigned int ReadUInt() {
		unsigned int val;
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 3) >= this->len) throw access_overflow;
		val = this->buf[this->read_index + 3];
		val |= ((unsigned int)(this->buf[this->read_index + 2])) << 8;
		val |= ((unsigned int)(this->buf[this->read_index + 1])) << 16;
		val |= ((unsigned int)(this->buf[this->read_index])) << 24;
		this->read_index += 4;
		return val;
	}
	int ReadInt() {
		int val;
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 3) >= this->len) throw access_overflow;
		val = this->buf[this->read_index + 3];
		val |= ((unsigned int)(this->buf[this->read_index + 2])) << 8;
		val |= ((unsigned int)(this->buf[this->read_index + 1])) << 16;
		val |= ((unsigned int)(this->buf[this->read_index])) << 24;
		this->read_index += 4;
		return val;
	}
	unsigned long long ReadULong() {
		unsigned long long val;
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 7) >= this->len) throw access_overflow;
		val = this->buf[this->read_index + 7];
		val |= ((unsigned long long)(this->buf[this->read_index + 6])) << 8;
		val |= ((unsigned long long)(this->buf[this->read_index + 5])) << 16;
		val |= ((unsigned long long)(this->buf[this->read_index + 4])) << 24;
		val |= ((unsigned long long)(this->buf[this->read_index + 3])) << 32;
		val |= ((unsigned long long)(this->buf[this->read_index + 2])) << 40;
		val |= ((unsigned long long)(this->buf[this->read_index + 1])) << 48;
		val |= ((unsigned long long)(this->buf[this->read_index])) << 56;
		this->read_index += 8;
		return val;
	}
	long long ReadLong() {
		long long val;
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 7) >= this->len) throw access_overflow;
		val = this->buf[this->read_index + 7];
		val |= ((unsigned long long)(this->buf[this->read_index + 6])) << 8;
		val |= ((unsigned long long)(this->buf[this->read_index + 5])) << 16;
		val |= ((unsigned long long)(this->buf[this->read_index + 4])) << 24;
		val |= ((unsigned long long)(this->buf[this->read_index + 3])) << 32;
		val |= ((unsigned long long)(this->buf[this->read_index + 2])) << 40;
		val |= ((unsigned long long)(this->buf[this->read_index + 1])) << 48;
		val |= ((unsigned long long)(this->buf[this->read_index])) << 56;
		this->read_index += 8;
		return val;
	}
	void WriteUByte(unsigned int val) {
		unsigned char* t_buf;
		
		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if (this->write_index >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index] = val;
		this->write_index++;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void WriteByte(int val) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if (this->write_index >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index] = val;
		this->write_index++;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void WriteUShort(unsigned int val) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 1) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index + 1] = val;
		this->buf[this->write_index] = val >> 8;
		this->write_index += 2;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void WriteShort(int val) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 1) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index + 1] = val;
		this->buf[this->write_index] = val >> 8;
		this->write_index += 2;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void WriteUInt(unsigned int val) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 3) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index + 3] = val;
		this->buf[this->write_index + 2] = val >> 8;
		this->buf[this->write_index + 1] = val >> 16;
		this->buf[this->write_index] = val >> 24;
		this->write_index += 4;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void WriteInt(int val) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 3) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index + 3] = val;
		this->buf[this->write_index + 2] = val >> 8;
		this->buf[this->write_index + 1] = val >> 16;
		this->buf[this->write_index] = val >> 24;
		this->write_index += 4;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void WriteULong(unsigned long long val) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 7) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index + 7] = val;
		this->buf[this->write_index + 6] = val >> 8;
		this->buf[this->write_index + 5] = val >> 16;
		this->buf[this->write_index + 4] = val >> 24;
		this->buf[this->write_index + 3] = val >> 32;
		this->buf[this->write_index + 2] = val >> 40;
		this->buf[this->write_index + 1] = val >> 48;
		this->buf[this->write_index] = val >> 56;
		this->write_index += 8;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void WriteLong(long long val) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 7) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		this->buf[this->write_index + 7] = val;
		this->buf[this->write_index + 6] = val >> 8;
		this->buf[this->write_index + 5] = val >> 16;
		this->buf[this->write_index + 4] = val >> 24;
		this->buf[this->write_index + 3] = val >> 32;
		this->buf[this->write_index + 2] = val >> 40;
		this->buf[this->write_index + 1] = val >> 48;
		this->buf[this->write_index] = val >> 56;
		this->write_index += 8;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	float ReadFloat() {
		union {
			unsigned char vi[4] ;
			float vf;
		};
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 3) >= this->len) throw access_overflow;
		vi[3] = this->buf[this->read_index];
		vi[2] = this->buf[this->read_index + 1];
		vi[1] = this->buf[this->read_index + 2];
		vi[0] = this->buf[this->read_index + 3];
		this->read_index += 4;
		return vf;
	}
	double ReadDouble() {
		union {
			unsigned char vi[8];
			double vf;
		};
		if (!this->buf) throw access_overflow;
		if ((this->read_index + 3) >= this->len) throw access_overflow;
		vi[7] = this->buf[this->read_index];
		vi[6] = this->buf[this->read_index + 1];
		vi[5] = this->buf[this->read_index + 2];
		vi[4] = this->buf[this->read_index + 3];
		vi[3] = this->buf[this->read_index + 4];
		vi[2] = this->buf[this->read_index + 5];
		vi[1] = this->buf[this->read_index + 6];
		vi[0] = this->buf[this->read_index + 7];
		this->read_index += 8;
		return vf;
	}
	void WriteFloat(float fv) {
		union {
			unsigned char vi[4];
			float vf;
		};
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 3) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		vf = fv;
		this->buf[this->write_index] = vi[3];
		this->buf[this->write_index + 1] = vi[2];
		this->buf[this->write_index + 2] = vi[1];
		this->buf[this->write_index + 3] = vi[0];
		this->write_index += 4;
		if (this->len < this->write_index) this->len = this->write_index;
	
	}
	void WriteDouble(double fv) {
		union {
			unsigned char vi[8];
			double vf;
		};
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf = new unsigned char[inc_sclae];
			this->buf_size = inc_sclae;
		}
		else if ((this->write_index + 7) >= this->buf_size) {
			t_buf = new unsigned char[this->buf_size + inc_sclae];
			memcpy_s(t_buf, this->buf_size + inc_sclae, this->buf, this->buf_size);
			this->buf_size += inc_sclae;
			delete this->buf;
			this->buf = t_buf;
		}
		vf = fv;
		this->buf[this->write_index] = vi[7];
		this->buf[this->write_index + 1] = vi[6];
		this->buf[this->write_index + 2] = vi[5];
		this->buf[this->write_index + 3] = vi[4];
		this->buf[this->write_index + 4] = vi[3];
		this->buf[this->write_index + 5] = vi[2];
		this->buf[this->write_index + 6] = vi[1];
		this->buf[this->write_index + 7] = vi[0];
		this->write_index += 8;
		if (this->len < this->write_index) this->len = this->write_index;
	}
	void ReadArray(void* _p, unsigned int len) {
		if (!this->buf) throw access_overflow;
		if ((this->len - this->read_index) < len) throw access_overflow;
		memcpy_s(_p, len, this->buf + this->read_index, len);
		this->read_index += len;
	}
	void WriteArray(void* _p, unsigned int len) {
		unsigned char* t_buf;

		if (!this->buf) {
			this->buf_size = (len + inc_sclae) & ~(inc_sclae - 1);
			this->buf = new unsigned char[this->buf_size];
		}
		else if (this->write_index + len > this->buf_size) {
			t_buf = new unsigned char[this->buf_size + (len + inc_sclae) & ~(inc_sclae - 1)];
			memcpy_s(t_buf, this->buf_size + (len + inc_sclae) & ~(inc_sclae - 1), this->buf, this->buf_size);
			delete this->buf;
			this->buf = t_buf;
			this->buf_size = this->buf_size + (len + inc_sclae) & ~(inc_sclae - 1);
		}
		memcpy_s(this->buf + this->write_index, this->buf_size - this->write_index, _p, len);
		this->write_index += len;
		if (this->write_index > this->len) this->len = this->write_index;
	}
	void ReadArray(void* _p, unsigned int start, unsigned int len) {
		if (!len) return;
		if (start + len > this->len) throw access_overflow;
		memcpy_s(_p, len, this->buf + start, len);
	}
	int ReadVarInt() {
		int val = 0;
		int shift = 0;
		int byte;

		try {

			while (1) {
				byte = this->ReadByte();
				val |= (byte & 0x7f) << (shift * 7);
				if (!(byte & 0x80)) return val;
				shift++;
			}
		}
		catch (int e) {
			throw e;
		}
	}
	int ReadVarLong() {
		long long val = 0;
		int shift = 0;
		long long byte;

		while (1) {
			byte = this->ReadByte();
			val |= (byte & 0x7f) << (shift * 7);
			if (!(byte & 0x80)) return val;
			shift++;
		}
	}
	void WriteVarInt(int val) {
		char varInt[5];
		int c = 0;
		char byte;

		varInt[0] = 0;
		while (val) {
			byte = val & 0x7f;
			val >>= 7;
			if (!val) {
				varInt[c] = byte;
				break;
			}
			varInt[c] = byte | 0x80;
			c++;
		}
		c++;
		this->WriteArray(varInt, c);
	}
	void WriteVarLong(long long val) {
		char varInt[12];
		int c = 0;
		char byte;

		varInt[0] = 0;
		while (val) {
			byte = len & 0x7f;
			len >>= 7;
			if (!len) {
				varInt[c] = byte;
				break;
			}
			varInt[c] = byte | 0x80;
			c++;
		}
		c++;
		this->WriteArray(varInt, c);
	}
	std::wstring* ReadString() {
		int len;
		std::wstring* str;
		char* str_buf;

		len = this->ReadVarInt();
		if (!len) return new std::wstring();
		str_buf = new char[len];
		this->ReadArray(str_buf, len);
		str = utf8_utf16(str_buf, len);
		delete[] str_buf;
		return str;
	}
	void WriteString(std::wstring * str) {
		int len;
		char* utf8;

		len = str->length();
		if (!len) {
			this->WriteVarInt(len);
			return;
		}
		utf8 = new char[len * 4];
		len = utf16_utf8(str, utf8);
		this->WriteVarInt(len);
		this->WriteArray(utf8, len);
		delete[] utf8;
	}

};
class Pack : public StreamArray {
private:
	int pack_id = -1;
public:
	Pack() {}
	Pack(StreamArray* _ar) {
		try {
			StreamArray::Set(_ar);
			pack_id = this->ReadVarInt();
		}
		catch (int e) {

		}
	}
	~Pack() {
	}
	Pack(int id) {
		this->pack_id = id;
		StreamArray::WriteVarInt(id);
	}
	int GetId() {
		return pack_id;
	}
	void WritePos(int x, int y, int z) {
		unsigned long long val = 0;
		val = (((long long)x) & 0x3ffffff) << 38;
		val |= (((long long)z) & 0x3ffffff) << 12;
		val |= y & 0xfff;
		this->WriteULong(val);
	}
	void ReadPos(int* x, int* y, int* z) {
		unsigned long long val;
		int _x, _y, _z;

		val = this->ReadULong();
		_x = val >> 38;
		_y = val & 0xfff;
		_z = (val >> 12) & 0x3ffffff;
		if (_x & 0x02000000) _x |= 0xfc000000;
		if (_y & 0x800) _y |= 0xfffff000;
		if (_z & 0x02000000) _z |= 0xfc000000;
		*x = _x;
		*y = _y;
		*z = _z;
	}
	double ReadFixed() {
		int val = this->ReadInt();
		double fv = val;
		fv /= 32;
		return fv;
	}
	void WriteFixed(double fv) {
		int val;
		fv *= 32;
		val = fv;
		this->WriteInt(val);
	}
	void ReadUUID(UUID* id) {
		long long h = this->ReadLong();
		long long l = this->ReadLong();
		((long long*)id)[0] = l;
		((long long*)id)[1] = h;
	}
	void WriteUUID(UUID* id) {
		long long h = ((long long*)id)[1];
		long long l = ((long long*)id)[0];
		this->WriteLong(h);
		this->WriteLong(l);
	}
	std::wstring* ReadShortString() {
		int len;
		std::wstring* str;
		char* str_buf;

		len = this->ReadUShort();
		if (!len) return new std::wstring();
		str_buf = new char[len];
		this->ReadArray(str_buf, len);
		str = utf8_utf16(str_buf, len);
		delete[] str_buf;
		return str;
	}
	std::wstring* ReadName(int len) {
		char* n;
		if (!len) return new std::wstring();
		n = new char[len];
		this->ReadArray(n, len);


	}
};