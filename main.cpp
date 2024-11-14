#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>


using namespace std;

class SerialPort
{

private:
    HANDLE hSerial;
    DCB dcbSerialParams;
    DWORD bytesRead;
    DWORD bytesWritten;

public:
    SerialPort();
    bool open(const char *portName, DWORD baudRate, BYTE dataBits, BYTE stopbits, BYTE parity);
    bool write(const char *data, int length);
    bool read(char *data, int length);
    bool close();
    int getBytesRead();
    int getBytesWritten();
    ~SerialPort();
};

SerialPort::SerialPort()
{
}

bool SerialPort::open(const char *portName, DWORD baudRate, BYTE dataBits,BYTE stopbits,BYTE parity)
{
    hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        std::cerr << "can't open serial port" << std::endl;
        return false;
    }
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        std::cerr << "get serial port state failed" << std::endl;
        CloseHandle(hSerial);
        return false;
    }
    dcbSerialParams.BaudRate = baudRate;  
    dcbSerialParams.ByteSize = dataBits;       
    dcbSerialParams.StopBits = stopbits; 
    dcbSerialParams.Parity = parity;     
    if (SetCommState(hSerial, &dcbSerialParams) == 0)
    {
        std::cout << GetLastError() << std::endl;
        std::cerr << "set serial port state failed" << std::endl;
        CloseHandle(hSerial);
        return false;
    }
    std::cout << "set serial port state success" << std::endl;
    return true;
}

bool SerialPort::write(const char *data, int length)
{
    if (!WriteFile(hSerial, data, length, &bytesWritten, NULL))
    {
        std::cerr << "write data failed" << std::endl;
        CloseHandle(hSerial);
        return false;
    }
    return true;
}

bool SerialPort::read(char *buffer, int length)
{
    if (!ReadFile(hSerial, buffer, length, &bytesRead, NULL))
    {
        std::cerr << "read data failed" << std::endl;
        CloseHandle(hSerial);
        return false;
    }
    return true;
}

bool SerialPort::close()
{
    if (CloseHandle(hSerial))
        return true;
    else
        return false;
}

int SerialPort::getBytesRead()
{
    return bytesRead;
}

int SerialPort::getBytesWritten()
{
    return bytesWritten;
}

SerialPort::~SerialPort()
{
}

class SerialPortOutStreamBuf : public streambuf
{

public:
    SerialPortOutStreamBuf(SerialPort* port_) : port(port_)
    {
    }

protected:
    int_type overflow(int_type c)
    {

        if (c != EOF)
        {
            port->write((char*)&c,1);
            int bytesWritten = port->getBytesWritten();
            if (bytesWritten <= 0)
            {
                return EOF;
            }
        }
        return c;
    }

private:
    SerialPort* port;
};
int main(int, char **)
{
    SerialPort port;
    int a = 5;
    cout << a << endl;
    std::streambuf *originalBuffer = std::cout.rdbuf();
    port.open("COM1", CBR_115200, 8, ONESTOPBIT, NOPARITY);
    SerialPortOutStreamBuf outbuf(&port);
    cout.rdbuf(&outbuf);
    cout << "Hello World!" << endl;
    port.close();
    cout.rdbuf(originalBuffer);
    cout << "Hello World!" << endl;
    return 0;
}
