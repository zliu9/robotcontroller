#ifndef SERIAL_PORT
#define SERIAL_PORT

HANDLE serialPortOpen(char *port, int rate);
int serialPortClose(HANDLE hSerial);
int sendData(HANDLE hSerial, char *data, int len);

void heartBeat(char *cmd, int *cmdLen);
void propertyGet(char *cmd, int *cmdLen);
void defaultParameterSet(char *cmd, int *cmdLen, int servo);
void defaultParameterGet(char *cmd, int *cmdLen, int servo);
void currentParameterGet(char *cmd, int *cmdLen, int servo);
void nop(char *cmd, int *cmdLen);
void robotModeSet(char *cmd, int *cmdLen, int mode);
void robotModeGet(char *cmd, int *cmdLen);
void robotPWMSet(char *cmd, int *cmdLen, int on);
void jointPositionReset(char *cmd, int *cmdLen, int servo);
void jointAbsolutePosSet(char *cmd, int *cmdLen, int servo, int* position);
void jointRelativePosSet(char *cmd, int *cmdLen, int servo, int* position);
void jointSpeedReSet(char *cmd, int *cmdLen, int servo);
void jointAbsoluteSpeedSet(char *cmd, int *cmdLen, int servo, int* speed);
void jointRelativeSpeedSet(char *cmd, int *cmdLen, int servo, int* speed);
void jointPositionLock(char *cmd, int *cmdLen, int servo);
void jointPositionUnlock(char *cmd, int *cmdLen, int servo);
void crcError(char *cmd, int *cmdLen);

#endif