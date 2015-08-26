// To compile with MinGW:
//
//      gcc -o serial.exe serial.c
//
// To compile with cl, the Microsoft compiler:
//
//      cl serial.cpp
//
// To run:
//
//      serial.exe COM1 115200
//
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>

#include "robot_control.h"

unsigned char calCRC8(char *data, unsigned int len) {
	  unsigned char crc=0;
	  unsigned char genPoly = 0x07;
	  for (int i=0; i<len; i++) {
		    crc ^= data[i];
		    for(int j=0; j<8; j++) {
			      if(crc & 0x80) {
				        crc = (crc << 1) ^ genPoly;
				    } else {
				    	  crc <<= 1;     
		        }
	      }
	  }
	  crc &= 0xff;   
	  return crc; 
}

static TCHAR* charTotchar(char *str) {

    int len = strlen(str);
    TCHAR *strT = new TCHAR[len + 1];
    mbstowcs(strT, str, len);
    strT[len] = _T('\0');

    return strT;
}


HANDLE serialPortOpen(char *port, int rate) {
	  char port_name[20];
	  memset(port_name, '\0', 20);
	  strcpy(port_name, "\\\\.\\");
	  strcat(port_name, port); // argv[1] is port name
 
    // Declare variables and structures
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    // Open the highest available serial port number
    fprintf(stderr, "Opening serial port...");
    hSerial = CreateFile(
                charTotchar(port_name), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error\n");
        return NULL;
    } else {
    	  fprintf(stderr, "OK\n");
    }
     
    // Set device parameters (38400 baud, 1 start bit,
    // 1 stop bit, no parity)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0) {
        fprintf(stderr, "Error getting device state\n");
        CloseHandle(hSerial);
        return NULL;
    }
     
    //dcbSerialParams.BaudRate = CBR_38400;
    switch (rate) {
    	  case 300: {
            dcbSerialParams.BaudRate=CBR_300;
            break ;
        }
        case 1200: {
            dcbSerialParams.BaudRate=CBR_1200;
            break ;
        }
        case 2400: {
            dcbSerialParams.BaudRate=CBR_2400;
            break ;
        }
        case 4800: {
            dcbSerialParams.BaudRate=CBR_4800;
            break ;
        }
        case 9600: {
            dcbSerialParams.BaudRate=CBR_9600;
            break ;
        }
        case 14400: {
            dcbSerialParams.BaudRate=CBR_14400;
            break ;
        }
        case 19200: {
            dcbSerialParams.BaudRate=CBR_19200;
            break ;
        }
        /*case 28800: {
            dcbSerialParams.BaudRate=CBR_28800;
            break ;
        }*/
        case 38400: {
            dcbSerialParams.BaudRate=CBR_38400;
            break ;
        }
        case 57600 : {
            dcbSerialParams.BaudRate=CBR_57600;
            break ;
        }
        case 115200 : {
            dcbSerialParams.BaudRate=CBR_115200;
            break ;
        }
        default : {
            fprintf(stderr, "Error setting port rate\n");
            return NULL;
        }
    }
            
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if(SetCommState(hSerial, &dcbSerialParams) == 0) {
        fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(hSerial);
        return NULL;
    }
 
    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if(SetCommTimeouts(hSerial, &timeouts) == 0) {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return NULL;
    }

    return hSerial;
}

int serialPortClose(HANDLE hSerial) {
    // Close serial port
    fprintf(stderr, "Closing serial port...");
    if (CloseHandle(hSerial) == 0)
    {
        fprintf(stderr, "Error\n");
        return -1;
    }
    fprintf(stderr, "OK\n");
    
    return 0;
}

int sendData(HANDLE hSerial, char *data, int len) {
    // Send specified text (remaining command line arguments)
    DWORD bytesWritten;

    fprintf(stderr, "Sending bytes...");
    if(!WriteFile(hSerial, data, len, &bytesWritten, NULL)) {
        fprintf(stderr, "Error\n");
        CloseHandle(hSerial);
        return -1;
    }
    fprintf(stderr, "%d bytes written\n", bytesWritten);
    return 0;
}

void transfer (char *a, char *b, int input) {
    *a = input&0xff;
    *b = (input>>8)&0xff;
}

char cmdHeader = 0xab;
char cmdVersion = 0x16;
char cmdType[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
char encryption[1] = {0x00};

typedef struct SERVO{
	  int id;
	  char *name;
	  int init;
	  int min;
	  int max;
	  int speed;
} servo;

#define SERVONUMBER 8
/*servo robotServo[SERVONUMBER] = {{1, "head", 500, -15, -15, 100},
	                     {2, "neck", 500, -90, 90, 100},
	                     {16, "right shoulder", 500, -90, 90, 100},
	                     {17, "right arm", 500, -90, 90, 100},
	                     {18, "right elbow", 500, 0, 60, 100},
	                     {32, "left shoulder", 500, -90, 90, 100},
	                     {33, "left arm", 500, -90, 90, 100},
	                     {34, "left elbow", 500, 0, 60, 100},
	                     {35, "left elbow", 500, 0, 60, 100}};
*/
servo robotServo[SERVONUMBER] = {{1, "head", 0, -15, -15, 60}, 
	                     {2, "neck", 0, -90, 90, 60},
	                     {16, "right shoulder", 0, -90, 90, 60},
	                     {17, "right arm", -90, -90, 90, 60},
	                     {18, "right elbow", 0, 0, 60, 60},
	                     {32, "left shoulder", 0, -90, 90, 60},
	                     {33, "left arm", -90, -90, 90, 60},
	                     {34, "left elbow", 0, 0, 60, 60}};

#define PROLOG cmd[0] = cmdHeader; \
               cmd[2] = cmdVersion; \
               cmd[3] = cmdType[0]; \
               cmd[5] = encryption[0]; \
               int ITER = 6;

#define EPILOG cmd[1] = ITER+1; \
               cmd[cmd[1]-1] = calCRC8(cmd, cmd[1]-1); \
               *cmdLen = cmd[1];

void heartBeat(char *cmd, int *cmdLen) {
	  PROLOG

    // ID
    cmd[4] = 0x00;

    // data

    EPILOG
}

void propertyGet(char *cmd, int *cmdLen) {
    PROLOG

    // ID
    cmd[4] = 0x01;
    
    // data
    
    EPILOG
}

void defaultParameterSet(char *cmd, int *cmdLen, int servo) {
    PROLOG

    // ID
    cmd[4] = 0x02;

    // data
    cmd[ITER++] = servo;
    if (servo == 0) {
        cmd[ITER++] = 0x00;
        cmd[ITER++] = 0x00;
        cmd[ITER++] = 0x00;
        cmd[ITER++] = 0x00;

        for (int i=0; i<SERVONUMBER; i++) {
            transfer(&cmd[ITER], &cmd[ITER+1], robotServo[i].init);
            transfer(&cmd[ITER+2], &cmd[ITER+3], robotServo[i].speed);
            ITER+=4;
        }
    } else {
        transfer(&cmd[ITER++], &cmd[ITER++], robotServo[servo].init);
        transfer(&cmd[ITER++], &cmd[ITER++], robotServo[servo].speed);
    }

    EPILOG
}

void defaultParameterGet(char *cmd, int *cmdLen, int servo) {
    PROLOG
    
    // ID
    cmd[4] = 0x03;
    
    // data
    cmd[ITER++] = servo;
    
    EPILOG
}

void currentParameterGet(char *cmd, int *cmdLen, int servo) {
    PROLOG

    // ID
    cmd[4] = 0x04;
    
    // data
    cmd[ITER++] = servo;
    
    EPILOG
}

void nop(char *cmd, int *cmdLen) {
    PROLOG
    
    // ID
    cmd[4] = 0x05;
    
    // data
    
    EPILOG
}

void robotModeSet(char *cmd, int *cmdLen, int mode) {
    PROLOG
    
    // ID
    cmd[4] = 0x06;
    
    // data
    if (mode == 0) {
        cmd[ITER++] = 0x00;
    } else {
        cmd[ITER++] = 0x01;
    }
    
    EPILOG
}

void robotModeGet(char *cmd, int *cmdLen) {
    PROLOG
    
    // ID
    cmd[4] = 0x07;
    
    // data
    
    EPILOG
}

void robotPWMSet(char *cmd, int *cmdLen, int on) {
    PROLOG
    
    // ID
    cmd[4] = 0x08;
    
    // data
    if (on == 0) {
    	  cmd[ITER++] = 0;
    } else {
    	  cmd[ITER++] = 1;
    }

    EPILOG
}

void jointPositionReset(char *cmd, int *cmdLen, int servo) {
    PROLOG

    // ID
    cmd[4] = 0x10;
    
    // data
    cmd[ITER++] = servo;

    EPILOG
}

void jointAbsolutePosSet(char *cmd, int *cmdLen, int servo, int* position) {
    PROLOG

    // ID
    cmd[4] = 0x11;
    
    // data
    // data
    cmd[ITER++] = servo;
    if (servo == 0) {
        cmd[ITER++] = 0x00;
        cmd[ITER++] = 0x00;
        for (int i=0; i<SERVONUMBER; i++) {
            transfer(&cmd[ITER], &cmd[ITER+1], position[i]);
            ITER+=2;
        }
    } else {
        transfer(&cmd[ITER], &cmd[ITER+1], position[0]);
        ITER+=2;
    }
    
    EPILOG
}

void jointRelativePosSet(char *cmd, int *cmdLen, int servo, int* position) {
    PROLOG
    
    // ID
    cmd[4] = 0x12;
    
    // data
    // data
    cmd[ITER++] = servo;
    if (servo == 0) {
        cmd[ITER++] = 0x00;
        cmd[ITER++] = 0x00;
        for (int i=0; i<SERVONUMBER; i++) {
            transfer(&cmd[ITER], &cmd[ITER+1], position[i]);
            ITER+=2;
        }
    } else {
        transfer(&cmd[ITER], &cmd[ITER+1], position[0]);
        ITER+=2;
    }
    
    EPILOG
}

void jointSpeedReSet(char *cmd, int *cmdLen, int servo) {
    PROLOG
    
    // ID
    cmd[4] = 0x20;
    
    // data
    cmd[ITER++] = servo;
    
    EPILOG
}

void jointAbsoluteSpeedSet(char *cmd, int *cmdLen, int servo, int* speed) {
    PROLOG
    
    // ID
    cmd[4] = 0x21;
    
    // data
    cmd[ITER++] = servo;
    if (servo == 0) {
        cmd[ITER++] = 0x00;
        cmd[ITER++] = 0x00;
        for (int i=0; i<SERVONUMBER; i++) {
            transfer(&cmd[ITER], &cmd[ITER+1], speed[i]);
            ITER+=2;
        }
    } else {
        transfer(&cmd[ITER], &cmd[ITER+1], speed[0]);
        ITER+=2;
    }
    
    EPILOG
}

void jointRelativeSpeedSet(char *cmd, int *cmdLen, int servo, int* speed) {
    PROLOG
    
    // ID
    cmd[4] = 0x22;
    
    // data
    cmd[ITER++] = servo;
    if (servo == 0) {
        cmd[ITER++] = 0x00;
        cmd[ITER++] = 0x00;
        for (int i=0; i<SERVONUMBER; i++) {
            transfer(&cmd[ITER], &cmd[ITER+1], speed[i]);
            ITER+=2;
        }
    } else {
        transfer(&cmd[ITER], &cmd[ITER+1], speed[0]);
        ITER+=2;
    }
    
    EPILOG
}

void jointPositionLock(char *cmd, int *cmdLen, int servo) {
    PROLOG
    
    // ID
    cmd[4] = 0x30;
    
    // data
    cmd[ITER++] = servo;
    
    EPILOG
}

void jointPositionUnlock(char *cmd, int *cmdLen, int servo) {
    PROLOG
    
    // ID
    cmd[4] = 0x31;
    
    // data
    cmd[ITER++] = servo;
    
    EPILOG
}

void crcError(char *cmd, int *cmdLen) {
    cmd[0] = cmdHeader;
    cmd[2] = cmdVersion;
    
    // type
    cmd[3] = cmdType[0];
    
    // ID
    cmd[4] = 0xff;
    
    // data
    cmd[5] = 0x00;
    
    // length
    cmd[1] = 0x07;
    
    cmd[cmd[1]-1] = calCRC8(cmd, cmd[1]-1);
    *cmdLen = cmd[1];
}

int mainBak(int argc, char *argv[], char **envp) {
    // argv[1] is serial port name
    // argv[2] is serial port rate
    // argv[3] is data to be sent

    int ret;

    HANDLE fd = serialPortOpen(argv[1], atoi(argv[2]));
    if (fd == NULL) {
        return -1;
    }

    char cmd[50];
    int len;

    heartBeat(cmd, &len);
    fprintf(stderr, "heartBeat len=%d\ndata=", len);
    for (int i=0; i<len; i++) {
        fprintf(stderr, "%02x ", unsigned char(cmd[i]));
    }
    fprintf(stderr, "\n");
              
    int speed[1], position[1];
    ret = 0;
    while(1) {
        char a = getchar();
        switch (a) {
            case '1':
            	speed[0] = 300;
            	jointAbsoluteSpeedSet(cmd, &len, 0x10, speed);
              fprintf(stderr, "jointAbsoluteSpeedSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }

            	position[0] = 450;
            	jointAbsolutePosSet(cmd, &len, 0x10, position);
              fprintf(stderr, "jointAbsolutePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
            	break;
            case '2':
            	position[0] = 450;
            	jointRelativePosSet(cmd, &len, 0x10, position);
            	fprintf(stderr, "jointRelativePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
            	break;
            case '3':
            	position[0] = -900;
            	jointRelativePosSet(cmd, &len, 0x10, position);
            	fprintf(stderr, "jointAbsolutePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
              break;
            case '4':
            	speed[0] = 150;
            	jointAbsoluteSpeedSet(cmd, &len, 0x10, speed);
              fprintf(stderr, "jointAbsoluteSpeedSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
              
            	position[0] = -900;
            	jointAbsolutePosSet(cmd, &len, 0x10, position);
            	fprintf(stderr, "jointAbsolutePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
              
              position[0] = 0;
            	jointAbsolutePosSet(cmd, &len, 0x10, position);
            	fprintf(stderr, "jointAbsolutePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
              break;
            case '5':
            	speed[0] = 300;
            	jointAbsoluteSpeedSet(cmd, &len, 0x11, speed);
              fprintf(stderr, "jointAbsoluteSpeedSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }

            	position[0] = 900;
            	jointAbsolutePosSet(cmd, &len, 0x11, position);
              fprintf(stderr, "jointAbsolutePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
              
              position[0] = -900;
            	jointRelativePosSet(cmd, &len, 0x11, position);
              fprintf(stderr, "jointRelativePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
              
              position[0] = -900;
            	jointAbsolutePosSet(cmd, &len, 0x11, position);
              fprintf(stderr, "jointAbsolutePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
            	break;
            case '6':
            	speed[0] = 300;
            	jointAbsoluteSpeedSet(cmd, &len, 0x12, speed);
              fprintf(stderr, "jointAbsoluteSpeedSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }

            	position[0] = 600;
            	jointAbsolutePosSet(cmd, &len, 0x12, position);
              fprintf(stderr, "jointAbsolutePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
              
              position[0] = -600;
            	jointRelativePosSet(cmd, &len, 0x12, position);
              fprintf(stderr, "jointRelativePosSet len=%d\ndata=", len);
              for (int i=0; i<len; i++) {
    	            fprintf(stderr, "%02x ", unsigned char(cmd[i]));
              }
              fprintf(stderr, "\n");
              ret = sendData(fd, cmd, len);
              if (ret != 0) {
                 return ret;
              }
            	break;
            case '\n':
            	break;
            default:
            	ret = 1;
            	break;            	
        }
        if (ret == 1) {
            break;
        }
    }

    ret = serialPortClose(fd);
    if (ret != 0) {
        return ret;
    }
    
    return 0;
}