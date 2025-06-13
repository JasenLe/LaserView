#ifndef _FISH_INI_FILE_
#define _FISH_INI_FILE_
#include <string>
#include <vector>
#include <fstream>
// #include <strstream>
#include<sstream>
#include <iostream>

using namespace std;
class IniFileSTL
{
public:
    explicit IniFileSTL(const string &fileName);
    ~IniFileSTL(void);
    bool ReadFile(void);
    bool WriteFile(void);
    bool fileIsOpen(void) { return m_open; };
    bool RemoveSection( const string &section );
    bool RemoveKey( const string &section, const string &key );

    //其中value是数据如果读取失败时候的默认值
    template<typename T>
    T ReadValue( const string &section, const string &key, T value )
    {
        string str = ReadString( section, key, "" );
        if( "" == str )
        {
            return value;
        }

        std::istringstream in( str.c_str() );
        T ret;
        in>>ret;
        return ret;
    }

    template<typename T>
    bool WriteValue( const string &section, const string &key, T value )
    {
        std::stringstream out;
        out<<value;
        std::string str = out.str();
        return WriteString( section, key, str );
    }

private:
    string ReadString( const string &section, const string &key, const string &value );
    bool WriteString( const string &section, const string &key, const string &value );
    static string Trim( const string &str );
    static string LTrim( const string &str );
    static string RTrim( const string &str );
private:
    string m_fileName;
    vector<string> m_vctLine;
    bool m_open;
    bool m_write_flag;
};


#endif
