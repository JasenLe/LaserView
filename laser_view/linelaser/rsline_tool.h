#ifndef RSLINE_TOOL_H
#define RSLINE_TOOL_H
#include <QWidget>

class Point3f_t
{
public:
    Point3f_t() : x_(0), y_(0), z_(0), intensity_(0xFF) {}
    Point3f_t(float x, float y, float z, uint8_t intensity) : x_(x), y_(y), z_(z), intensity_(intensity) {}
    inline float &X() { return x_; }
    inline float &Y() { return y_; }
    inline float &Z() { return z_; }
    inline uint8_t &intensity() { return intensity_; }

private:
    float x_;
    float y_;
    float z_;
    uint8_t intensity_;
};

class Message
{
public:
    Message() : addr_(0), cmd_(0), offset_(0) {}
    Message(int addr, int cmd) : addr_(addr), cmd_(cmd), offset_(0) {}
    Message(int addr, int cmd, const uint8_t *data, const int len)
        : addr_(addr),
        cmd_(cmd),
        offset_(0),
        data_(data, data + len) {}
    Message(int addr, int cmd, const std::vector<uint8_t> &data)
        : addr_(addr),
        cmd_(cmd),
        offset_(0),
        data_(data) {}
    Message(int addr, int cmd, std::vector<uint8_t> &&data)
        : addr_(addr),
        cmd_(cmd),
        offset_(0),
        data_(data) {}
    Message(Message &msg) = default;
    Message(Message &&msg) = default;

    bool IsNull()
    {
        return isnull_;
    }

public:
    int addr_;
    int cmd_;
    int offset_;
    int length_;
    bool isnull_ = true;
    std::vector<uint8_t> data_;
};

#pragma pack(1)
class Protocol_vx90
{
public:
    static uint8_t check(const uint8_t *data, size_t len)
    {
        uint8_t cs = 0;
        for (size_t i = sizeof(start_bit_); i < len - 1; i++)
            cs += data[i];
        return cs;
    }

    const uint8_t start_bit_[2] = {0xAA, 0xAA};
    uint8_t addr_;
    uint8_t cmd_;
    uint16_t offset_;
    uint16_t length_;
    // data
    union
    {
        uint8_t data_;
        uint8_t cs_;
    };
};
#pragma pack()
#endif // RSLINE_TOOL_H
