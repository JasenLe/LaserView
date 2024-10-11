#include "inc/IniFile.h"

IniFileSTL::IniFileSTL(const string &fileName)
    :m_fileName(fileName),
    m_open(false),
    m_write_flag(false)
{
    m_vctLine.clear();
    ReadFile();
}

IniFileSTL::~IniFileSTL(void)
{
    if (m_write_flag)
        WriteFile();
    m_open = false;
    m_vctLine.clear();
}

bool IniFileSTL::ReadFile( void )
{
    ifstream in(m_fileName.c_str());
    bool bopen = in.is_open();
    if(bopen)
        m_open = true;
    else
        return false;

    string line;
    while( getline(in,line) )
    {
        m_vctLine.push_back(line);
    }
    in.close();
    return true;
}

string IniFileSTL::ReadString( const string &section, const string &key, const string &value )
{
    for( size_t i = 0;i < m_vctLine.size(); ++i )
    {
        string &section_line = m_vctLine[i];
        size_t sec_begin_pos = section_line.find('[');
        if( sec_begin_pos == string::npos || sec_begin_pos != 0 )
        {
            continue;
        }
        size_t sec_end_pos = section_line.find( ']', sec_begin_pos );
        if( sec_end_pos == string::npos )
        {
            continue;
        }

        if( section != Trim(section_line.substr(sec_begin_pos+1, sec_end_pos-sec_begin_pos-1) ) )
        {
            continue;
        }

        for( ++i; i < m_vctLine.size(); ++i )
        {
            string &key_line = m_vctLine[i];
            size_t sec_pos = key_line.find('[');
            if( sec_pos != string::npos && sec_pos == 0 )
            {
                --i;  //reback a step,find again
                break;//the line is section line
            }

            size_t begin_key_pos = key_line.find(key);
            if( begin_key_pos == string::npos )
            {
                continue;
            }
            size_t equal_pos = key_line.find( '=', begin_key_pos );
            if( equal_pos == string::npos )
            {
                continue;
            }
            if( key != Trim(key_line.substr( begin_key_pos, equal_pos - begin_key_pos) ) )
            {
                continue;
            }

            size_t comment_pos = key_line.find( "#", equal_pos + 1 );
            if( comment_pos != string::npos )
            {
                return Trim(key_line.substr( equal_pos + 1, comment_pos - equal_pos - 1 ));
            }

            return Trim(key_line.substr( equal_pos + 1 ));
        }
    }

    return value;
}

bool IniFileSTL::WriteString( const string &section, const string &key, const string &value )
{
    for( size_t i = 0;i < m_vctLine.size(); ++i )
    {
        string &section_line = m_vctLine[i];
        size_t sec_begin_pos = section_line.find('[');
        if( sec_begin_pos == string::npos || sec_begin_pos != 0 )
        {
            continue;
        }
        size_t sec_end_pos = section_line.find( ']', sec_begin_pos );
        if( sec_end_pos == string::npos )
        {
            continue;
        }
        if( section != Trim(section_line.substr(sec_begin_pos+1, sec_end_pos-sec_begin_pos-1)) )
        {
            continue;
        }

        for( ++i; i < m_vctLine.size(); ++i )
        {
            string &key_line = m_vctLine[i];
            size_t sec_pos = key_line.find('[');
            if( sec_pos != string::npos && sec_pos == 0 )
            {
                --i;  //reback a step,find again
                break;//the line is section line
            }

            size_t begin_key_pos = key_line.find(key);
            if( begin_key_pos == string::npos )
            {
                continue;
            }
            size_t equal_pos = key_line.find( '=', begin_key_pos );
            if( equal_pos == string::npos )
            {
                continue;
            }
            if( key != Trim(key_line.substr( begin_key_pos, equal_pos - begin_key_pos )) )
            {
                continue;
            }

            size_t comment_pos = key_line.find( "#", equal_pos + 1 );
            string new_line = key_line.substr( 0, equal_pos + 1 ) + value;
            if( comment_pos != string::npos )
            {
                new_line += key_line.substr( comment_pos );
            }
            key_line = new_line;

            m_write_flag = true;
            return true;
        }
        m_vctLine.insert( m_vctLine.begin() + i, key + "=" + value );

        m_write_flag = true;
        return true;
    }

    m_vctLine.insert( m_vctLine.end(), "" );
    m_vctLine.insert( m_vctLine.end(), "[" + section + "]" );
    m_vctLine.insert( m_vctLine.end(), key + "=" + value );
    m_write_flag = true;
    return true;
}

bool IniFileSTL::RemoveSection( const string &section )
{
    for( size_t i = 0;i < m_vctLine.size(); ++i )
    {
        string &section_line = m_vctLine[i];
        size_t sec_begin_pos = section_line.find('[');
        if( sec_begin_pos == string::npos || sec_begin_pos != 0 )
        {
            continue;
        }
        size_t sec_end_pos = section_line.find( ']', sec_begin_pos );
        if( sec_end_pos == string::npos )
        {
            continue;
        }
        if( section != Trim(section_line.substr(sec_begin_pos+1, sec_end_pos-sec_begin_pos-1)) )
        {
            continue;
        }


        size_t del_begin = i;
        for( ++i ; i < m_vctLine.size(); ++i )
        {
            string &next_section = m_vctLine[i];
            size_t next_pos = next_section.find('[');
            if( next_pos == string::npos || next_pos != 0 )
            {
                continue;
            }

            break;
        }
        m_vctLine.erase( m_vctLine.begin() + del_begin, m_vctLine.begin()+i );
        return true;
    }
    return false;
}

bool IniFileSTL::RemoveKey( const string &section, const string &key )
{
    for( size_t i = 0;i < m_vctLine.size(); ++i )
    {
        string &section_line = m_vctLine[i];
        size_t sec_begin_pos = section_line.find('[');
        if( sec_begin_pos == string::npos || sec_begin_pos != 0 )
        {
            continue;
        }
        size_t sec_end_pos = section_line.find( ']', sec_begin_pos );
        if( sec_end_pos == string::npos )
        {
            continue;
        }
        if( section != Trim(section_line.substr(sec_begin_pos+1, sec_end_pos-sec_begin_pos-1)) )
        {
            continue;
        }

        for( ++i ; i < m_vctLine.size(); ++i )
        {
            string &key_line = m_vctLine[i];
            size_t key_pos = key_line.find( key );
            if( key_pos == string::npos || key_pos != 0 )
            {
                continue;
            }
            else
            {
                m_vctLine.erase( m_vctLine.begin() + i );
                return true;
            }
        }
    }
    return false;
}

bool IniFileSTL::WriteFile( void )
{
    ofstream out(m_fileName.c_str());
    // int error = 0;
    // while (!out.is_open())
    // {
    //     out.open(m_fileName.c_str()/*, ios::out*/);
    //     if (++error > 2)
    //         return false;
    // }
    for( size_t i = 0; i < m_vctLine.size(); ++i )
    {
        out<<m_vctLine[i]<<endl;
    }
    out.close();
    m_write_flag = false;
    return true;
}

int IniFileSTL::ReadInt( const string &section, const string &key, int value )
{
    string str = ReadString( section, key, "" );
    if( "" == str )
    {
        return value;
    }

    std::istringstream in( str.c_str() );
    int ret = 0;
    in>>ret;
    return ret;
}
float IniFileSTL::ReadFloat( const string &section, const string &key, float value )
{
    string str = ReadString( section, key, "" );
    if( "" == str )
    {
        return value;
    }

    std::istringstream in( str.c_str() );
    float ret = 0;
    in>>ret;
    return ret;
}

bool IniFileSTL::WriteInt( const string &section, const string &key, int value )
{
    std::stringstream out;
    out<<value;
    std::string str = out.str();
    return WriteString( section, key, str.c_str() );
}
bool IniFileSTL::WriteFloat( const string &section, const string &key, float value )
{
    std::stringstream out;
    out<<value;
    std::string str = out.str();
    return WriteString( section, key, str.c_str() );
}

string IniFileSTL::LTrim( const string &str )
{
    size_t pos = 0;
    while( pos != str.size() )
    {
        if( ' ' == str[pos] )
        {
            ++pos;
        }
        else
        {
            break;
        }
    }

    return str.substr(pos);
}

string IniFileSTL::RTrim( const string &str )
{
    size_t pos = str.size() - 1;
    while( pos >= 0 )
    {
        if(' ' == str[pos])
        {
            --pos;
        }
        else
        {
            break;
        }
    }

    return str.substr( 0, pos + 1 );
}

string IniFileSTL::Trim(const string &str)
{
    return LTrim( RTrim(str) );
}
