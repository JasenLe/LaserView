#ifndef _FISH_INI_FILE_
#define _FISH_INI_FILE_
#include <string>
#include <vector>
#include <fstream>
// #include <strstream>
#include<sstream>
#include <iostream>

#define WIDGET  "Widget"
#define WWIDTH  "width"
#define WHEIGHT "height"

#define SEND   "Send"
#define SDATA  "S_data"

#define CONSOLE "Console"
#define DEVICE  "device"
#define BAND    "band"

#define BACKGROUND "backgrorund"
#define BCOLOR "backgrorund color"
#define CSCOLOR "CS bg color"

#define INDICATORLINE "Indicator Line"
#define LINEDIS       "distance"
#define LINEANGLE     "angle"
#define LINECONF      "confidence"
#define LINECOLOR     "linecolor"

#define PONITPIXEL "Point info"
#define PPIXEL     "pixel"
#define PCOLOR     "color"

#define FLITER      "Filter"
#define FLITER_all  "Filter all"
#define F_SMOOTH    "smooth"
#define F_BILATERAL "bilateral"
#define F_TAIL      "tail"
#define F_INTENSITY "intensity"
#define F_NEAR      "near"
#define F_NOISE     "noise"
#define F_TINE      "tine"
#define F_WANDER    "wander"
#define F_SHADOWS   "shadows"
#define F_MEDIAN    "median"

using namespace std;
class IniFileSTL
{
public:
    explicit IniFileSTL(const string &fileName);
    ~IniFileSTL(void);
    bool ReadFile(void);
    bool WriteFile(void);
    bool fileIsOpen(void) { return m_open; };

    //其中value是数据如果读取失败时候的默认值
    string ReadString( const string &section, const string &key, const string &value );
    int    ReadInt( const string &section, const string &key, int value );
    float  ReadFloat( const string &section, const string &key, float value );
    bool WriteString( const string &section, const string &key, const string &value );
    bool WriteInt( const string &section, const string &key, int value );
    bool WriteFloat( const string &section, const string &key, float value );
    bool RemoveSection( const string &section );
    bool RemoveKey( const string &section, const string &key );

private:
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
