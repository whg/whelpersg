#include <string>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <dirent.h>
#include <memory>
#include <unistd.h>
#include <vector>

namespace whg {

class Serial {
public:
    Serial() {}

    Serial( const std::string &device, size_t baud ) { open( device, baud ); }

    ~Serial() { close(); }

    bool open( const std::string &device, size_t baud ) {
        mHandle = ::open(("/dev/" + device).c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK );
        mIsOpen = mHandle != -1;

        if ( mIsOpen ) {
            termios options;
            tcgetattr( mHandle, &mPreviousOptions );

            cfsetispeed( &options, baud );
            cfsetospeed( &options, baud );

            options.c_cflag |= CLOCAL | CREAD | CS8;
            options.c_cflag &= ~(PARENB | CSTOPB);

            tcsetattr( mHandle, TCSANOW, &options );
        }

        return mIsOpen;
    }

    void close() {
        if ( mIsOpen ) {
            tcsetattr( mHandle, TCSANOW, &mPreviousOptions );
            ::close( mHandle );
            mIsOpen = false;
        }
    }

    int getNumBytesAvailable() const {
        int result;
        ::ioctl( mHandle, FIONREAD, &result );
        return result;
    }

    bool read( void *data, size_t length ) const {
        size_t readBytes = 0;
        while ( readBytes < length ) {
            size_t _readBytes = ::read( mHandle, data, length - readBytes );
            if ( _readBytes == -1 ) {
                break;
            }
            readBytes += _readBytes;
        }
        return readBytes == length;
    }

    std::string readAllCharacters() const {
        size_t length = getNumBytesAvailable();
        char characters[length];
        read( &characters, length );
        return std::string( &characters[0], length );
    }

    template<typename T>
    size_t write( const std::vector<T> &data ) {
        return ::write( mHandle, data.data(), data.size());
    }

    bool isOpen() const { return mIsOpen; }

protected:
    int mHandle;
    bool mIsOpen;
    termios mPreviousOptions;
};

}
