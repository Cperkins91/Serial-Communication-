

//---------------------------------------------------------------------------

#include <windows.h>   // for serial stuff
#include <iostream>  // for cout
#include <conio.h>     // for getch and kbhit
#pragma hdrstop
using namespace std;

//---------------------------------------------------------------------------

/* This function handles sending characters out the serial port.
   We use special handling for the CR character, sending an LF as well.
   This performs proper line returns when the user presses the "ENTER" key.

   Two parameters:
     1. char value -- a single character to be transmitted
     2. HANDLE hSerial -- a handle for the open serial port
   No return value
 */
void send_value(char value, HANDLE hSerial) {
    DWORD dwBytesSent = 0;   // number of bytes sent

    // send value, and handle any errors (also fail if value didn't successfully send)
    if (!WriteFile(hSerial, &value, 1, &dwBytesSent, NULL) || dwBytesSent != 1) {
        std::cout<<"Error sending value."<<std::endl;
        return;
    }
    cout<<value;             // echo to user the value he typed

    // special handling for line returns:
    if (value == 13) {  // if CR (user pressed enter)
        send_value(10, hSerial);   // send LF (move to next line)
    }
}

/* This function handles receiving characters from the serial port.
   It attempts to read a value, and, if a value was successfully read,
   displays the value on the user's screen.

   One parameter:
     1. HANDLE hSerial -- a handle for the open serial port
   No return value
 */
void recv_value(HANDLE hSerial) {
    char szBuff[2] = {0};  // read one byte
    DWORD dwBytesRead = 0;   // number of bytes read

    // try to recieve data, and handle any errors
    if (!ReadFile(hSerial, szBuff, 1, &dwBytesRead, NULL)) {
        cout<<"error reading data"<<endl;
        return;
    }

    if (dwBytesRead > 0) {        // if we received a value,
        cout<<szBuff[0];            // print the value
    }
}

/* This is our main function.
   In here, we open the serial port, then set its parameters.
   We then enter the main program loop where we continually attempt to
   read values from the serial port and the local keyboard.
   When a key is pressed, we send the value. (using send_value)
   When a value is received, we print it. (recv_value does this)
   Program execution ends when the user presses the "ESCAPE" key,
   at which time we break out of the while loop, close the serial port,
   and exit.
 */
int main()
{
    HANDLE hSerial;     // handle for serial port (like a file pointer)

    // open the port
    // note that the next two lines should be typed on one line only
    hSerial = CreateFile("COM4", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    // do error handling if we can't open the port:
    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cout<<"Serial port does not exist.  Uh oh!"<<std::endl;
            return -1;
        }
        std::cout<<"Some error occurred.  Uh oh!"<<std::endl;
        return -1;
    }

    DCB dcbSerialParams = {0};        // parameters variable

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);  // workaround strange Windows bug

    // try to get the current serial port state
    if (!GetCommState(hSerial, &dcbSerialParams)) {
       std::cout<<"Couldn't get serial port state.  Uh oh!"<<std::endl;
        return -1;
    }

    // customize values to our particular application:
    dcbSerialParams.BaudRate = CBR_9600;

    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    // set the values and handle any errors
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cout<<"Error setting serial port state.  Uh oh!"<<std::endl;
        return -1;
    }

    COMMTIMEOUTS timeouts={0};           // timeout parameters variable

    // these values determine how long Windows will wait for input
    // (see PDF file for details)
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    // set the values and handle any errors
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cout<<"Error setting timeouts. Uh oh!"<<std::endl;
        return -1;
    }

    char value = 0;                     // variable for user's last entered value

    std::cout<<"Serial Terminal!"<<std::endl;  // print a message to indicate that things are working ok
    while (value != 27) {               // loop until ESC pressed
        // check for key pressed:
        if (kbhit()) {
            value = getch();                // read value of key pressed
            send_value(value, hSerial);     // send value
        }
        recv_value(hSerial);              // try to recieve data
    }

    CloseHandle(hSerial);               // close the port
    return 0;                             // we're done
}
//---------------------------------------------------------------------------
